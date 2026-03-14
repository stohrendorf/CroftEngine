#pragma once

#include "concepts.h"

#include <functional>

namespace serialization
{
template<bool Loading, typename TContext>
using LazyCallback = std::function<void(const BaseSerializer<Loading, TContext>&)>;

namespace access
{
template<typename T, typename TContext>
requires Serializable<T, TContext> void dispatchSerialize(const T& data, const Serializer<TContext>& ser);

template<typename T, typename TContext>
requires Deserializable<T, TContext> void dispatchDeserialize(T& data, const Deserializer<TContext>& ser);

template<typename T, typename TContext>
requires Creatable<T, TContext> T dispatchCreate(const Deserializer<TContext>& ser);
} // namespace access

#define S_NV(name, obj) name, obj
#define S_ANV(name1, name2, obj) std::pair<std::string_view, std::string_view>{name1, name2}, obj
} // namespace serialization