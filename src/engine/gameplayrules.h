#pragma once

#include "serialization/serialization_fwd.h"

namespace engine::world
{
class World;
}

namespace engine
{
struct GameplayRules
{
  bool pistolsOnly = false;
  bool noLoads = false;
  bool noMeds = false;
  bool noCheats = false;
  bool noHits = false;

  void serialize(const serialization::Serializer<world::World>& ser) const;
  void deserialize(const serialization::Deserializer<world::World>& ser);
};
} // namespace engine
