#pragma once

#include "access.h"            // IWYU pragma: export
#include "exception.h"         // IWYU pragma: export
#include "serialization_fwd.h" // IWYU pragma: export

#include <boost/algorithm/string/replace.hpp>
#include <boost/assert.hpp>
#include <boost/log/trivial.hpp>
#include <cstdint>
#include <exception>
#include <functional>
#include <gsl/gsl-lite.hpp>
#include <iterator>
#include <memory>
#include <optional>
#include <queue>
#include <ryml.hpp>     // IWYU pragma: export
#include <ryml_std.hpp> // IWYU pragma: export
#include <string>
#include <string_view>
#include <type_traits>
#include <typeinfo>
#include <utility>

// #define SERIALIZATION_TRACE

template<>
struct std::iterator_traits<c4::yml::NodeRef::iterator>
{
  using iterator_category = std::forward_iterator_tag;
};

template<>
struct std::iterator_traits<c4::yml::ConstNodeRef::iterator>
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

template<typename T>
struct TypeId
{
  using type = T;
};

template<bool>
class YAMLDocument;

template<typename T>
struct Default;
template<typename T>
struct OptionalValue;

template<bool Loading, typename TContext>
class BaseSerializer final
{
public:
  using Node = std::conditional_t<Loading, ryml::ConstNodeRef, ryml::NodeRef>;

private:
  template<bool>
  friend class YAMLDocument;

  using LazyWithContext = std::function<void()>;
  using LazyQueue = std::queue<LazyWithContext>;

  explicit BaseSerializer(const Node& node,
                          const gsl::not_null<TContext*>& context,
                          const std::shared_ptr<LazyQueue>& lazyQueue)
      : m_lazyQueue{lazyQueue == nullptr ? std::make_shared<LazyQueue>() : lazyQueue}
      , node{node}
      , context{context}
  {
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
    if constexpr(!Loading)
      node |= ryml::MAP;

    gsl_Ensures(node.is_map());
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
  std::optional<BaseSerializer<Loading, TContext>> createMapMemberSerializer(const std::string_view& name,
                                                                             bool required) const
  {
    ensureIsMap();
    auto childNode = node.find_child(c4::csubstr(name.data(), name.size()));

    bool isSeed = false;
    if constexpr(!Loading)
    {
      isSeed = childNode.is_seed();
    }

    const bool exists = !isSeed && childNode.readable() && childNode.type() != ryml::NOTYPE;
    if constexpr(Loading)
    {
      if(!exists)
      {
        if(required)
          SERIALIZER_EXCEPTION(std::string{"Node "} + std::string{name.begin(), name.end()} + " not defined");
        else
          return std::nullopt;
      }
    }
    else
    {
      if(exists)
        SERIALIZER_EXCEPTION(std::string{"Node "} + std::string{name.begin(), name.end()} + " already defined");

      childNode = node.append_child();
      childNode.set_key(node.tree()->copy_to_arena(c4::csubstr(name.data(), name.size())));
    }

    return withNode(childNode);
  }

  template<typename T>
  void serializeDeserialize(const std::string_view& name, T&& data, BaseSerializer<Loading, TContext>& ser) const
  {
    try
    {
      access<T, Loading>::dispatch(data, ser);
    }
    catch(Exception&)
    {
      BOOST_LOG_TRIVIAL(fatal) << "Error while serializing \"" << name << "\" of type \"" << typeid(data).name()
                               << "\"";
      throw;
    }
    catch(std::exception& ex)
    {
      BOOST_LOG_TRIVIAL(fatal) << "Error while serializing \"" << name << "\" of type \"" << typeid(data).name()
                               << "\"";
      SERIALIZER_EXCEPTION(ex.what());
    }
    catch(...)
    {
      BOOST_LOG_TRIVIAL(fatal) << "Error while serializing \"" << name << "\" of type \"" << typeid(data).name()
                               << "\"";
      SERIALIZER_EXCEPTION("Unexpected exception");
    }
  }

public:
  mutable Node node;
  gsl::not_null<TContext*> context;

  ~BaseSerializer()
  {
    if constexpr(!Loading)
    {
      if(node.has_val() && !m_tag.empty())
        node.set_val_tag(node.tree()->copy_to_arena(c4::to_csubstr(m_tag)));
    }
  }

  void lazy(const LazyCallback<Loading, TContext>& lazyCallback) const
  {
    if constexpr(Loading)
      m_lazyQueue->emplace(
        [lazyCallback = lazyCallback, ser = *this]()
        {
          lazyCallback(ser);
        });
    else
      lazyCallback(*this);
  }

  auto& operator<<(const LazyCallback<Loading, TContext>& lazyCallback) const
  {
    lazy(lazyCallback);
    return *this;
  }

  template<typename T>
  void lazy(T* instance, void (T::*member)(const BaseSerializer<Loading, TContext>&)) const
  {
    lazy(LazyCallback<Loading, TContext>{instance, member});
  }

  template<typename T>
  void lazy(const std::string_view& name, T& data) const
  {
    lazy(
      [pdata = &data, name = name](const BaseSerializer<Loading, TContext>& ser)
      {
        ser(name, *pdata);
      });
  }

  template<typename T, typename... Ts>
  const BaseSerializer<Loading, TContext>&
    operator()(const std::string_view& headName, T&& headData, Ts&&... tail) const
  {
    (*this)(headName, std::forward<T>(headData));
    if constexpr(sizeof...(tail) > 0)
      (*this)(std::forward<Ts>(tail)...);
    return *this;
  }

  template<typename T>
  const BaseSerializer<Loading, TContext>& operator()(const std::string_view& name, T&& data) const
  {
    ensureIsMap();
    BOOST_ASSERT(node.readable());
    if constexpr(!Loading)
    {
      BOOST_ASSERT(!node.is_seed());
    }

#ifdef SERIALIZATION_TRACE
    BOOST_LOG_TRIVIAL(trace) << "Serializing node " << getQualifiedKey() << "::" << name.get();
#endif

    auto ser = createMapMemberSerializer(name, true);
    gsl_Assert(ser.has_value());
    serializeDeserialize(name, data, *ser);
    return *this;
  }

  template<typename T>
  const BaseSerializer<Loading, TContext>& operator()(const std::string_view& name, Default<T>&& data) const
  {
    ensureIsMap();
    BOOST_ASSERT(node.readable());
    if constexpr(!Loading)
    {
      BOOST_ASSERT(!node.is_seed());
    }

#ifdef SERIALIZATION_TRACE
    BOOST_LOG_TRIVIAL(trace) << "Serializing node " << getQualifiedKey() << "::" << name.get();
#endif

    if constexpr(Loading)
    {
      auto ser = createMapMemberSerializer(name, false);
      if(!ser.has_value())
      {
        data.value.get() = data.defaultValue;
        return *this;
      }
      serializeDeserialize(name, data.value.get(), *ser);
    }
    else
    {
      auto ser = createMapMemberSerializer(name, true);
      serializeDeserialize(name, data.value.get(), *ser);
    }
    return *this;
  }

  template<typename T>
  const BaseSerializer<Loading, TContext>& operator()(const std::string_view& name, OptionalValue<T>&& data) const
  {
    ensureIsMap();
    BOOST_ASSERT(node.readable());

#ifdef SERIALIZATION_TRACE
    BOOST_LOG_TRIVIAL(trace) << "Serializing node " << getQualifiedKey() << "::" << name;
#endif

    auto ser = createMapMemberSerializer(name, false);
    if(!ser.has_value())
    {
      return *this;
    }
    serializeDeserialize(name, data.value.get(), *ser);
    return *this;
  }

  BaseSerializer<Loading, TContext> operator[](gsl::czstring name) const
  {
    return (*this)[std::string{name}];
  }

  BaseSerializer<Loading, TContext> operator[](const std::string& name) const
  {
    ensureIsMap();
    auto existing = node[c4::to_csubstr(name)];
    if constexpr(!Loading)
    {
      if(existing.is_seed() || !existing.readable() || existing.type() == ryml::NOTYPE)
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

  [[nodiscard]] auto withNode(const Node& otherNode) const
  {
    return BaseSerializer<Loading, TContext>{otherNode, context, m_lazyQueue};
  }

  template<bool LazyLoading = Loading>
  [[nodiscard]] auto newChild() const -> std::enable_if_t<!LazyLoading, Serializer<TContext>>
  {
    return withNode(node.append_child());
  }

  void tag(const std::string& tag) const
  {
    gsl_Expects(!tag.empty());

    auto normalizedTag
      = std::string{"<"}
        + boost::algorithm::replace_all_copy(boost::algorithm::replace_all_copy(tag, ">", "&gt;"), " ", "%20") + ">";
    if constexpr(Loading)
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

  template<bool LazyLoading = Loading>
  auto setNull() const -> std::enable_if_t<!LazyLoading, void>
  {
    node.set_val("~");
    m_tag = "!!null";
  }

  template<bool LazyLoading = Loading>
  auto isNull() const -> std::enable_if_t<LazyLoading, bool>
  {
    return node.has_val_tag() && util::toString(node.val_tag()) == "!!null";
  }
};

namespace detail
{
// some specializations to avoid storing 8-bit numbers as characters
template<typename TContext>
inline void serialize(const int8_t& data, const Serializer<TContext>& ser)
{
  ser.node << static_cast<int16_t>(data);
}

template<typename TContext>
inline void deserialize(int8_t& data, const Deserializer<TContext>& ser)
{
  int16_t tmp{};
  ser.node >> tmp;
  data = gsl::narrow<int8_t>(tmp);
}

template<typename TContext>
inline void serialize(const uint8_t& data, const Serializer<TContext>& ser)
{
  ser.node << static_cast<uint16_t>(data);
}

template<typename TContext>
inline void deserialize(uint8_t& data, const Deserializer<TContext>& ser)
{
  uint16_t tmp{};
  ser.node >> tmp;
  data = gsl::narrow<uint8_t>(tmp);
}

template<typename T, typename TContext>
inline void serialize(const T& data, const Serializer<TContext>& ser)
{
  ser.node << data;
}

template<typename T, typename TContext>
inline void deserialize(T& data, const Deserializer<TContext>& ser)
{
  ser.node >> data;
}

template<typename T, typename TContext>
inline std::enable_if_t<std::is_default_constructible_v<T>, T> createTrivial(const TypeId<T>&,
                                                                             const Deserializer<TContext>& ser)
{
  T tmp{};
  access<T, true>::dispatch(tmp, ser);
  return tmp;
}
} // namespace detail

template<typename TContext>
inline void serialize(const std::string& data, const Serializer<TContext>& ser)
{
  detail::serialize(data, ser);
}

template<typename TContext>
inline void deserialize(std::string& data, const Deserializer<TContext>& ser)
{
  detail::deserialize(data, ser);
}

template<typename T, typename TContext>
inline std::enable_if_t<std::is_arithmetic_v<T>, void> serialize(const T& data, const Serializer<TContext>& ser)
{
  detail::serialize(data, ser);
}

template<typename T, typename TContext>
inline std::enable_if_t<std::is_arithmetic_v<T>, void> deserialize(T& data, const Deserializer<TContext>& ser)
{
  detail::deserialize(data, ser);
}

template<typename T, typename TContext>
inline std::enable_if_t<std::is_enum_v<T>, void> serialize(const T& data, const Serializer<TContext>& ser)
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  detail::serialize(*reinterpret_cast<const std::underlying_type_t<T>*>(&data), ser);
}

template<typename T, typename TContext>
inline std::enable_if_t<std::is_enum_v<T>, void> deserialize(T& data, const Deserializer<TContext>& ser)
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  detail::deserialize(*reinterpret_cast<std::underlying_type_t<const T>*>(&data), ser);
}

template<typename TContext>
inline void serialize(const bool& data, const Serializer<TContext>& ser)
{
  detail::serialize(data, ser);
}

template<typename TContext>
inline void deserialize(bool& data, const Deserializer<TContext>& ser)
{
  detail::deserialize(data, ser);
}

template<typename T, typename TContext>
inline std::enable_if_t<std::is_arithmetic_v<T>, T> create(const TypeId<T>& tid, const Deserializer<TContext>& ser)
{
  return detail::createTrivial(tid, ser);
}

template<typename T, typename TContext>
inline std::enable_if_t<std::is_enum_v<T>, T> create(const TypeId<T>&, const Deserializer<TContext>& ser)
{
  return static_cast<T>(detail::createTrivial(TypeId<std::underlying_type_t<T>>{}, ser));
}

template<typename TContext>
inline bool create(const TypeId<bool>& tid, const Deserializer<TContext>& ser)
{
  return detail::createTrivial(tid, ser);
}

template<typename TContext>
inline std::string create(const TypeId<std::string>& tid, const Deserializer<TContext>& ser)
{
  return detail::createTrivial(tid, ser);
}
} // namespace serialization
