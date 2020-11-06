#pragma once

#include "serialization_fwd.h"

#include <ryml/ryml_std.hpp>
#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <exception>
#include <fstream>
#include <gsl-lite.hpp>
#include <queue>
#include <ryml/ryml.hpp>
#include <string>
#include <type_traits>
#include <typeinfo>

namespace engine
{
class Engine;
}

template<>
struct std::iterator_traits<c4::yml::NodeRef::iterator>
{
  using iterator_category = std::forward_iterator_tag;
};

namespace serialization
{
namespace util
{
inline std::string toString(const c4::csubstr& s)
{
  if(!s.has_str())
    return {};

  return {s.data(), s.size()};
}
} // namespace util

class Exception : public std::runtime_error
{
public:
  explicit Exception(const std::string& msg)
      : Exception{msg.c_str()}
  {
  }

  explicit Exception(gsl::czstring msg);
};

#define SERIALIZER_EXCEPTION(msg) BOOST_THROW_EXCEPTION(::serialization::Exception{msg})

template<typename T>
struct TypeId
{
  using type = T;
};

struct access
{
  template<typename T>
  static void serializeTrivial(T& data, const Serializer& ser);

  template<typename T>
  static std::enable_if_t<std::is_default_constructible_v<T>, T> createTrivial(const TypeId<T>&, const Serializer& ser);

  template<typename T>
  static auto callSerializeOrLoad(T& data, const Serializer& ser) -> decltype(data.serialize(ser), void())
  {
    return data.serialize(ser);
  }

  template<typename T>
  static auto callSerializeOrLoad(T& data, const Serializer& ser) -> decltype(serialize(data, ser), void())
  {
    return serialize(data, ser);
  }

  template<typename T>
  static auto callSerializeOrLoad(T& data, const Serializer& ser) -> decltype(data.load(ser), void())
  {
    return data.load(ser);
  }

  template<typename T>
  static auto callSerializeOrLoad(T& data, const Serializer& ser) -> decltype(load(data, ser), void())
  {
    return load(data, ser);
  }

  template<typename T>
  static auto callSerializeOrSave(T& data, const Serializer& ser) -> decltype(data.serialize(ser), void())
  {
    return data.serialize(ser);
  }

  template<typename T>
  static auto callSerializeOrSave(T& data, const Serializer& ser) -> decltype(serialize(data, ser), void())
  {
    return serialize(data, ser);
  }

  template<typename T>
  static auto callSerializeOrSave(T& data, const Serializer& ser) -> decltype(data.save(ser), void())
  {
    return data.save(ser);
  }

  template<typename T>
  static auto callSerializeOrSave(T& data, const Serializer& ser) -> decltype(save(data, ser), void())
  {
    return save(data, ser);
  }

  template<typename T>
  static void callSerialize(T& data, const Serializer& ser);

  template<typename T>
  static auto callCreate(const TypeId<T>&, const Serializer& ser) -> decltype(T::create(ser));

  template<typename T>
  static auto callCreate(const TypeId<T>& tid, const Serializer& ser) -> decltype(create(tid, ser));
};

class Serializer final
{
  using LazyWithContext = std::function<void()>;
  using LazyQueue = std::queue<LazyWithContext>;

  explicit Serializer(const ryml::NodeRef& node,
                      engine::Engine& engine,
                      bool loading,
                      const std::shared_ptr<LazyQueue>& lazyQueue);

  std::shared_ptr<LazyQueue> m_lazyQueue;
  mutable std::string m_tag;

  void processQueues();

  void ensureIsMap() const;
  std::string getQualifiedKey() const;
  Serializer createMapMemberSerializer(const gsl::not_null<gsl::czstring>& name) const;

public:
  mutable ryml::NodeRef node;
  engine::Engine& engine;
  const bool loading;

  ~Serializer();

  void lazy(const LazyCallback& lazyCallback) const;

  template<typename T>
  void lazy(T* instance, void (T::*member)(const Serializer&)) const
  {
    lazy(LazyCallback{instance, member});
  }

  template<typename T>
  void lazy(const gsl::not_null<gsl::czstring>& name, T& data) const
  {
    lazy([pdata = &data, name = name](const Serializer& ser) { ser(name, *pdata); });
  }

  static void save(const std::string& filename, engine::Engine& engine);
  static void load(const std::string& filename, engine::Engine& engine);

  template<typename T, typename... Ts>
  const Serializer& operator()(const gsl::not_null<gsl::czstring>& headName, T&& headData, Ts&&... tail) const
  {
    (*this)(headName, std::forward<T>(headData));
    if constexpr(sizeof...(tail) > 0)
      (*this)(std::forward<Ts>(tail)...);
    return *this;
  }

  template<typename T>
  const Serializer& operator()(const gsl::not_null<gsl::czstring>& name, T&& data) const
  {
    ensureIsMap();
    BOOST_ASSERT(node.valid());
    BOOST_ASSERT(!node.is_seed());
    BOOST_LOG_TRIVIAL(trace) << "Serializing node " << getQualifiedKey() << "::" << name.get();

    Serializer ser = createMapMemberSerializer(name);

    try
    {
      if(loading)
        access::callSerializeOrLoad(data, ser);
      else
        access::callSerializeOrSave(data, ser);
    }
    catch(Exception&)
    {
      BOOST_LOG_TRIVIAL(fatal) << "Error while serializing \"" << name.get() << "\" of type \"" << typeid(data).name()
                               << "\"";
      throw;
    }
    catch(std::exception& ex)
    {
      BOOST_LOG_TRIVIAL(fatal) << "Error while serializing \"" << name.get() << "\" of type \"" << typeid(data).name()
                               << "\"";
      SERIALIZER_EXCEPTION(ex.what());
    }
    catch(...)
    {
      BOOST_LOG_TRIVIAL(fatal) << "Error while serializing \"" << name.get() << "\" of type \"" << typeid(data).name()
                               << "\"";
      SERIALIZER_EXCEPTION("Unexpected exception");
    }
    return *this;
  }

  Serializer operator[](gsl::czstring name) const;
  Serializer operator[](const std::string& name) const;
  Serializer withNode(const ryml::NodeRef& otherNode) const;
  Serializer newChild() const;
  void tag(const std::string& tag) const;

  void setNull() const
  {
    if(loading)
      SERIALIZER_EXCEPTION("Cannot set a node to null when loading");

    node.set_val("~");
    m_tag = "!!null";
  }

  bool isNull() const
  {
    if(!loading)
      SERIALIZER_EXCEPTION("Cannot check a node for null when not loading");

    return node.has_val_tag() && util::toString(node.val_tag()) == "!!null";
  }
};

template<typename T>
inline void access::serializeTrivial(T& data, const Serializer& ser)
{
  if(ser.loading)
  {
    ser.node >> data;
  }
  else
  {
    ser.node << data;
  }
}

// some specializations to avoid storing 8-bit numbers as characters
template<>
inline void access::serializeTrivial<int8_t>(int8_t& data, const Serializer& ser)
{
  if(ser.loading)
  {
    int16_t tmp{};
    ser.node >> tmp;
    data = gsl::narrow<int8_t>(tmp);
  }
  else
  {
    ser.node << static_cast<int16_t>(data);
  }
}

template<>
inline void access::serializeTrivial<uint8_t>(uint8_t& data, const Serializer& ser)
{
  if(ser.loading)
  {
    uint16_t tmp{};
    ser.node >> tmp;
    data = gsl::narrow<uint8_t>(tmp);
  }
  else
  {
    ser.node << static_cast<uint16_t>(data);
  }
}

template<typename T>
inline std::enable_if_t<std::is_default_constructible_v<T>, T> access::createTrivial(const TypeId<T>&,
                                                                                     const Serializer& ser)
{
  Expects(ser.loading);

  T tmp{};
  callSerializeOrLoad(tmp, ser);
  return tmp;
}

template<typename T>
inline void access::callSerialize(T& data, const Serializer& ser)
{
  if(ser.loading)
    callSerializeOrLoad(data, ser);
  else
    callSerializeOrSave(data, ser);
}

template<typename T>
inline auto access::callCreate(const TypeId<T>&, const Serializer& ser) -> decltype(T::create(ser))
{
  Expects(ser.loading);
  return T::create(ser);
}

template<typename T>
inline auto access::callCreate(const TypeId<T>& tid, const Serializer& ser) -> decltype(create(tid, ser))
{
  Expects(ser.loading);
  return create(tid, ser);
}

inline void serialize(std::string& data, const Serializer& ser)
{
  access::serializeTrivial(data, ser);
}

template<typename T>
std::enable_if_t<std::is_arithmetic_v<T>, void> serialize(T& data, const Serializer& ser)
{
  access::serializeTrivial(data, ser);
}

template<typename T>
std::enable_if_t<std::is_enum_v<T>, void> serialize(T& data, const Serializer& ser)
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  access::serializeTrivial(*reinterpret_cast<std::underlying_type_t<T>*>(&data), ser);
}

inline void serialize(bool& data, const Serializer& ser)
{
  access::serializeTrivial(data, ser);
}

template<typename T>
std::enable_if_t<std::is_arithmetic_v<T>, T> create(const TypeId<T>& tid, const Serializer& ser)
{
  return access::createTrivial(tid, ser);
}

template<typename T>
std::enable_if_t<std::is_enum_v<T>, T> create(const TypeId<T>&, const Serializer& ser)
{
  return static_cast<T>(access::createTrivial(TypeId<std::underlying_type_t<T>>{}, ser));
}

inline bool create(const TypeId<bool>& tid, const Serializer& ser)
{
  return access::createTrivial(tid, ser);
}

inline std::string create(const TypeId<std::string>& tid, const Serializer& ser)
{
  return access::createTrivial(tid, ser);
}
} // namespace serialization
