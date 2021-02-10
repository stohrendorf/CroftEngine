#pragma once

#include <functional>
#include <type_traits>

namespace serialization
{
template<typename T>
struct TypeId;

template<typename TContext>
class Serializer;

template<typename TContext>
using LazyCallback = std::function<void(const Serializer<TContext>&)>;

namespace
{
template<typename T>
struct access;
}

#define S_NV(name, obj) name, obj
} // namespace serialization
