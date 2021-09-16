#pragma once

#include "serialization/serialization_fwd.h"

#include <cstddef>
#include <gsl/gsl-lite.hpp>
#include <memory>

// IWYU pragma: no_forward_declare serialization::Serializer
// IWYU pragma: no_forward_declare serialization::TypeId

namespace loader::file
{
struct Item;
}

namespace engine::world
{
class World;
}

namespace engine::objects
{
class Object;
extern std::shared_ptr<Object> createObject(world::World& world, loader::file::Item& item, size_t id);
extern gsl::not_null<std::shared_ptr<Object>>
  create(const serialization::TypeId<gsl::not_null<std::shared_ptr<Object>>>&,
         const serialization::Serializer<world::World>& ser);
} // namespace engine::objects

namespace serialization
{
using engine::objects::create;
extern void serialize(std::shared_ptr<engine::objects::Object>& ptr, const Serializer<engine::world::World>& ser);
} // namespace serialization
