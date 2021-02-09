#pragma once

#include <functional>

namespace serialization
{
template<typename T>
struct TypeId;

template<typename TContext>
class Serializer;

template<typename TContext>
using LazyCallback = std::function<void(const Serializer<TContext>&)>;

#define S_NV(name, obj) name, obj
#define S_NVP(obj) S_NV(#obj, obj)
} // namespace serialization
