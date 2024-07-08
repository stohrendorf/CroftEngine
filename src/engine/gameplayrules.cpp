#include "gameplayrules.h"

#include "serialization/optional_value.h"
#include "serialization/serialization.h"

#include <functional>

namespace engine
{
void GameplayRules::serialize(const serialization::Serializer<world::World>& ser) const
{
  ser(S_NV("pistolsOnly", pistolsOnly),
      S_NV("noLoads", noLoads),
      S_NV("noMeds", noMeds),
      S_NV("noCheats", noCheats),
      S_NV("noHits", noHits));
}

void GameplayRules::deserialize(const serialization::Deserializer<world::World>& ser)
{
  ser(S_NV("pistolsOnly", pistolsOnly),
      S_NV("noLoads", noLoads),
      S_NV("noMeds", noMeds),
      S_NV("noCheats", noCheats),
      S_NVO("noHits", std::ref(noHits)));
}
} // namespace engine
