#include "gameplayrules.h"

#include "serialization/serialization.h"

namespace engine
{
void GameplayRules::serialize(const serialization::Serializer<world::World>& ser) const
{
  ser(S_NV("pistolsOnly", pistolsOnly), S_NV("noLoads", noLoads), S_NV("noMeds", noMeds), S_NV("noCheats", noCheats));
}

void GameplayRules::deserialize(const serialization::Deserializer<world::World>& ser)
{
  ser(S_NV("pistolsOnly", pistolsOnly), S_NV("noLoads", noLoads), S_NV("noMeds", noMeds), S_NV("noCheats", noCheats));
}
} // namespace engine
