#pragma once

#include <functional>

namespace engine
{
class Engine;
}

namespace serialization
{
namespace detail
{
template<typename T>
T Result();
}

template<typename T>
struct TypeId;

class Serializer;
struct access;

using LazyCallback = std::function<void(const Serializer&)>;

#define S_NV(name, obj) name, obj
#define S_NVP(obj) S_NV(#obj, obj)
} // namespace serialization
