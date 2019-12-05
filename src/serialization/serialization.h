#pragma once

#include "gsl-lite.hpp"
#include "serialization_fwd.h"

#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <exception>
#include <fstream>
#include <queue>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <yaml-cpp/yaml.h>

namespace engine
{
class Engine;
}

namespace serialization
{
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
  using NextWithContext = std::function<void()>;
  using NextQueue = std::queue<NextWithContext>;

  explicit Serializer(const YAML::Node& node,
                      engine::Engine& engine,
                      bool loading,
                      const std::shared_ptr<NextQueue>& nextQueue)
      : m_nextQueue{nextQueue == nullptr ? std::make_shared<NextQueue>() : nextQueue}
      , node{node}
      , engine{engine}
      , loading{loading}
  {
  }

  std::shared_ptr<NextQueue> m_nextQueue;

  void processQueues();

  mutable std::string m_tag{};

  void applyTag();

public:
  mutable YAML::Node node;
  engine::Engine& engine;
  const bool loading;

  ~Serializer()
  {
    applyTag();
  }

  void lazy(const Next& next) const
  {
    if(loading)
      m_nextQueue->emplace([next = next, ser = *this]() { next(ser); });
    else
      next(*this);
  }

  template<typename T>
  void lazy(T* instance, void (T::*member)(const Serializer&)) const
  {
    lazy(Next{instance, member});
  }

  template<typename T>
  void lazy(const gsl::not_null<gsl::czstring>& name, T& data) const
  {
    lazy([pdata = &data, name = name](const Serializer& ser) { ser(name, *pdata); });
  }

  static void save(const std::string& filename, engine::Engine& engine);
  static void load(const std::string& filename, engine::Engine& engine);

  const Serializer& operator()() const noexcept
  {
    return *this;
  }

  template<typename T, typename... Ts>
  const Serializer& operator()(const gsl::not_null<gsl::czstring>& headName,
                               T&& headData,
                               const gsl::not_null<gsl::czstring>& tailName,
                               Ts&&... tail) const
  {
    (*this)(headName, std::forward<T>(headData));
    (*this)(tailName, std::forward<Ts>(tail)...);
    return *this;
  }

  template<typename T>
  const Serializer& operator()(const gsl::not_null<gsl::czstring>& name, T&& data) const
  {
    BOOST_LOG_TRIVIAL(trace) << "Serializing node " << name.get();

    if(loading)
    {
      if(!node[name.get()].IsDefined())
        SERIALIZER_EXCEPTION(std::string{"Node "} + name.get() + " not defined");
    }
    else
    {
      if(node[name.get()].IsDefined())
        SERIALIZER_EXCEPTION(std::string{"Node "} + name.get() + " already defined");
    }

    try
    {
      auto ser = (*this)[name.get()];
      if(loading)
        access::callSerializeOrLoad(data, ser);
      else
        access::callSerializeOrSave(data, ser);
      ser.applyTag();
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

  Serializer operator[](const gsl::czstring name) const
  {
    return withNode(node[name]);
  }

  Serializer operator[](const std::string& name) const
  {
    return withNode(node[name]);
  }

  Serializer withNode(const YAML::Node& otherNode) const
  {
    return Serializer{otherNode, engine, loading, m_nextQueue};
  }

  void tag(std::string tag) const
  {
    m_tag = std::move(tag);
  }
};

template<typename T>
inline void access::serializeTrivial(T& data, const Serializer& ser)
{
  if(ser.loading)
  {
    data = std::move(ser.node.as<T>());
  }
  else
  {
    ser.node = data;
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
