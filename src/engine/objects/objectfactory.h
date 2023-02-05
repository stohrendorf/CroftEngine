#pragma once

#include "serialization/serialization_fwd.h"

#include <cstddef>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <memory>

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
extern gslu::nn_shared<Object> create(const serialization::TypeId<gslu::nn_shared<Object>>&,
                                      const serialization::Deserializer<world::World>& ser);
} // namespace engine::objects

namespace serialization
{
using engine::objects::create;
extern void serialize(const std::shared_ptr<engine::objects::Object>& ptr, const Serializer<engine::world::World>& ser);
extern void deserialize(std::shared_ptr<engine::objects::Object>& ptr, const Deserializer<engine::world::World>& ser);
} // namespace serialization
