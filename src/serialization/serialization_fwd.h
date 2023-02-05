#pragma once

#include <functional>

namespace serialization
{
template<typename T>
struct TypeId;

template<bool Loading, typename TContext>
class BaseSerializer;

template<typename TContext>
using Serializer = BaseSerializer<false, TContext>;
template<typename TContext>
using Deserializer = BaseSerializer<true, TContext>;

template<bool Loading, typename TContext>
using LazyCallback = std::function<void(const BaseSerializer<Loading, TContext>&)>;

template<typename T, bool Loading>
struct access;

#define S_NV(name, obj) name, obj
} // namespace serialization
