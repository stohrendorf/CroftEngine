#pragma once

#include "serialization_fwd.h"

#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <exception>
#include <gsl-lite.hpp>
#include <queue>
#include <ryml.hpp>
#include <ryml_std.hpp>
#include <string>
#include <type_traits>
#include <typeinfo>

// #define SERIALIZATION_TRACE

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

namespace access
{
template<typename T, typename TContext>
static void serializeTrivial(T& data, const Serializer<TContext>& ser);

template<typename T, typename TContext>
static std::enable_if_t<std::is_default_constructible_v<T>, T> createTrivial(const TypeId<T>&,
                                                                             const Serializer<TContext>& ser);

template<typename T, typename TContext>
static auto callSerializeOrLoad(T& data, const Serializer<TContext>& ser) -> decltype(data.serialize(ser), void())
{
  return data.serialize(ser);
}

template<typename T, typename TContext>
static auto callSerializeOrLoad(T& data, const Serializer<TContext>& ser) -> decltype(serialize(data, ser), void())
{
  return serialize(data, ser);
}

template<typename T, typename TContext>
static auto callSerializeOrLoad(T& data, const Serializer<TContext>& ser) -> decltype(data.load(ser), void())
{
  return data.load(ser);
}

template<typename T, typename TContext>
static auto callSerializeOrLoad(T& data, const Serializer<TContext>& ser) -> decltype(load(data, ser), void())
{
  return load(data, ser);
}

template<typename T, typename TContext>
static auto callSerializeOrSave(T& data, const Serializer<TContext>& ser) -> decltype(data.serialize(ser), void())
{
  return data.serialize(ser);
}

template<typename T, typename TContext>
static auto callSerializeOrSave(T& data, const Serializer<TContext>& ser) -> decltype(serialize(data, ser), void())
{
  return serialize(data, ser);
}

template<typename T, typename TContext>
static auto callSerializeOrSave(T& data, const Serializer<TContext>& ser) -> decltype(data.save(ser), void())
{
  return data.save(ser);
}

template<typename T, typename TContext>
static auto callSerializeOrSave(T& data, const Serializer<TContext>& ser) -> decltype(save(data, ser), void())
{
  return save(data, ser);
}

template<typename T, typename TContext>
static void callSerialize(T& data, const Serializer<TContext>& ser);

template<typename T, typename TContext>
static auto callCreate(const TypeId<T>&, const Serializer<TContext>& ser) -> decltype(T::create(ser));

template<typename T, typename TContext>
static auto callCreate(const TypeId<T>& tid, const Serializer<TContext>& ser) -> decltype(create(tid, ser));

// some specializations to avoid storing 8-bit numbers as characters
template<typename TContext>
inline void serializeTrivial(int8_t& data, const Serializer<TContext>& ser)
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

template<typename TContext>
inline void serializeTrivial(uint8_t& data, const Serializer<TContext>& ser)
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
}; // namespace access

template<bool>
class YAMLDocument;

template<typename TContext>
class Serializer final
{
  template<bool>
  friend class YAMLDocument;

  using LazyWithContext = std::function<void()>;
  using LazyQueue = std::queue<LazyWithContext>;

  explicit Serializer(const ryml::NodeRef& node,
                      TContext& context,
                      bool loading,
                      const std::shared_ptr<LazyQueue>& lazyQueue)
      : m_lazyQueue{lazyQueue == nullptr ? std::make_shared<LazyQueue>() : lazyQueue}
      , node{node}
      , context{context}
      , loading{loading}
  {
    ryml::set_callbacks(ryml::Callbacks{
      nullptr,
      [](size_t length, void* /*hint*/, void* /*user_data*/) -> gsl::owner<void*> { return new char[length]; },
      [](gsl::owner<void*> mem, size_t /*length*/, void* /*user_data*/) { delete[] static_cast<char*>(mem); },
      [](const char* msg, size_t msg_len, ryml::Location /*location*/, void* /*user_data*/) {
        const std::string msgStr{msg, msg_len};
        SERIALIZER_EXCEPTION(msgStr);
      }});
  }

  std::shared_ptr<LazyQueue> m_lazyQueue;
  mutable std::string m_tag;

  void processQueues()
  {
    while(!m_lazyQueue->empty())
    {
#ifdef SERIALIZATION_TRACE
      BOOST_LOG_TRIVIAL(debug) << "Processing serialization queue...";
#endif
      auto current = std::exchange(m_lazyQueue, std::make_shared<LazyQueue>());
      while(!current->empty())
      {
        current->front()();
        current->pop();
      }
    }
  }

  void ensureIsMap() const
  {
    if(!loading)
      node |= ryml::MAP;

    Expects(node.is_map());
  }
#ifdef SERIALIZATION_TRACE
  std::string getQualifiedKey() const
  {
    if(!node.has_key())
    {
      return {};
    }

    std::string result = util::toString(node.key());
    auto cursor = node;
    while(!cursor.is_root() && cursor.has_parent())
    {
      cursor = cursor.parent();
      if(cursor.has_key())
        result = util::toString(cursor.key()) + "::" + result;
      else
        result = "??::" + result;
    }
    return result;
  }
#endif
  Serializer<TContext> createMapMemberSerializer(const gsl::not_null<gsl::czstring>& name) const
  {
    ensureIsMap();
    auto childNode = node.find_child(c4::to_csubstr(name.get()));
    const bool exists = !childNode.is_seed() && childNode.valid() && childNode.type() != ryml::NOTYPE;
    if(loading)
    {
      if(!exists)
        SERIALIZER_EXCEPTION(std::string{"Node "} + name.get() + " not defined");
    }
    else
    {
      if(exists)
        SERIALIZER_EXCEPTION(std::string{"Node "} + name.get() + " already defined");

      childNode = node.append_child();
      childNode.set_key(node.tree()->copy_to_arena(c4::to_csubstr(name.get())));
    }

    return withNode(childNode);
  }

public:
  mutable ryml::NodeRef node;
  TContext& context;
  const bool loading;

  ~Serializer()
  {
    if(!loading && node.has_val() && !m_tag.empty())
      node.set_val_tag(node.tree()->copy_to_arena(c4::to_csubstr(m_tag)));
  }

  void lazy(const LazyCallback<TContext>& lazyCallback) const
  {
    if(loading)
      m_lazyQueue->emplace([lazyCallback = lazyCallback, ser = *this]() { lazyCallback(ser); });
    else
      lazyCallback(*this);
  }

  template<typename T>
  void lazy(T* instance, void (T::*member)(const Serializer<TContext>&)) const
  {
    lazy(LazyCallback<TContext>{instance, member});
  }

  template<typename T>
  void lazy(const gsl::not_null<gsl::czstring>& name, T& data) const
  {
    lazy([pdata = &data, name = name](const Serializer<TContext>& ser) { ser(name, *pdata); });
  }

  template<typename T, typename... Ts>
  const Serializer<TContext>& operator()(const gsl::not_null<gsl::czstring>& headName, T&& headData, Ts&&... tail) const
  {
    (*this)(headName, std::forward<T>(headData));
    if constexpr(sizeof...(tail) > 0)
      (*this)(std::forward<Ts>(tail)...);
    return *this;
  }

  template<typename T>
  const Serializer<TContext>& operator()(const gsl::not_null<gsl::czstring>& name, T&& data) const
  {
    ensureIsMap();
    BOOST_ASSERT(node.valid());
    BOOST_ASSERT(!node.is_seed());
#ifdef SERIALIZATION_TRACE
    BOOST_LOG_TRIVIAL(trace) << "Serializing node " << getQualifiedKey() << "::" << name.get();
#endif

    auto ser = createMapMemberSerializer(name);

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

  Serializer<TContext> operator[](gsl::czstring name) const
  {
    return (*this)[std::string{name}];
  }

  Serializer<TContext> operator[](const std::string& name) const
  {
    ensureIsMap();
    auto existing = node[c4::to_csubstr(name)];
    if(!loading)
    {
      if(existing.is_seed() || !existing.valid() || existing.type() == ryml::NOTYPE)
      {
        auto ser = newChild();
        ser.node.set_key(node.tree()->copy_to_arena(c4::to_csubstr(name)));
        return ser;
      }
      else
      {
        return withNode(existing);
      }
    }
    else
    {
      return withNode(existing);
    }
  }

  Serializer<TContext> withNode(const ryml::NodeRef& otherNode) const
  {
    return Serializer{otherNode, context, loading, m_lazyQueue};
  }

  Serializer<TContext> newChild() const
  {
    return withNode(node.append_child());
  }

  void tag(const std::string& tag) const
  {
    Expects(!tag.empty());

    auto normalizedTag
      = std::string{"!<"}
        + boost::algorithm::replace_all_copy(boost::algorithm::replace_all_copy(tag, ">", "&gt;"), " ", "%20") + ">";
    if(loading)
    {
      if(!node.has_val())
        return;

      if(!node.has_val_tag())
        SERIALIZER_EXCEPTION(("Expected tag \"" + normalizedTag + "\", but got no tag"));

      std::string existingTag = util::toString(node.val_tag());
      if(existingTag != normalizedTag)
        SERIALIZER_EXCEPTION("Expected tag \"" + normalizedTag + "\", but got \"" + existingTag + "\"");
    }
    else
      m_tag = normalizedTag;
  }

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

template<typename T, typename TContext>
inline void access::serializeTrivial(T& data, const Serializer<TContext>& ser)
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

template<typename T, typename TContext>
inline std::enable_if_t<std::is_default_constructible_v<T>, T> access::createTrivial(const TypeId<T>&,
                                                                                     const Serializer<TContext>& ser)
{
  Expects(ser.loading);

  T tmp{};
  callSerializeOrLoad(tmp, ser);
  return tmp;
}

template<typename T, typename TContext>
inline void access::callSerialize(T& data, const Serializer<TContext>& ser)
{
  if(ser.loading)
    callSerializeOrLoad(data, ser);
  else
    callSerializeOrSave(data, ser);
}

template<typename T, typename TContext>
inline auto access::callCreate(const TypeId<T>&, const Serializer<TContext>& ser) -> decltype(T::create(ser))
{
  Expects(ser.loading);
  return T::create(ser);
}

template<typename T, typename TContext>
inline auto access::callCreate(const TypeId<T>& tid, const Serializer<TContext>& ser) -> decltype(create(tid, ser))
{
  Expects(ser.loading);
  return create(tid, ser);
}

template<typename TContext>
inline void serialize(std::string& data, const Serializer<TContext>& ser)
{
  access::serializeTrivial(data, ser);
}

template<typename T, typename TContext>
std::enable_if_t<std::is_arithmetic_v<T>, void> serialize(T& data, const Serializer<TContext>& ser)
{
  access::serializeTrivial(data, ser);
}

template<typename T, typename TContext>
std::enable_if_t<std::is_enum_v<T>, void> serialize(T& data, const Serializer<TContext>& ser)
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  access::serializeTrivial(*reinterpret_cast<std::underlying_type_t<T>*>(&data), ser);
}

template<typename TContext>
inline void serialize(bool& data, const Serializer<TContext>& ser)
{
  access::serializeTrivial(data, ser);
}

template<typename T, typename TContext>
std::enable_if_t<std::is_arithmetic_v<T>, T> create(const TypeId<T>& tid, const Serializer<TContext>& ser)
{
  return access::createTrivial(tid, ser);
}

template<typename T, typename TContext>
std::enable_if_t<std::is_enum_v<T>, T> create(const TypeId<T>&, const Serializer<TContext>& ser)
{
  return static_cast<T>(access::createTrivial(TypeId<std::underlying_type_t<T>>{}, ser));
}

template<typename TContext>
inline bool create(const TypeId<bool>& tid, const Serializer<TContext>& ser)
{
  return access::createTrivial(tid, ser);
}

template<typename TContext>
inline std::string create(const TypeId<std::string>& tid, const Serializer<TContext>& ser)
{
  return access::createTrivial(tid, ser);
}
} // namespace serialization
