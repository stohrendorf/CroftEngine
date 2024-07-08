#include "box.h"

#include "serialization/serialization.h"

namespace engine::world
{
void Box::serialize(const serialization::Serializer<World>& ser) const
{
  ser(S_NV("blocked", blocked), S_NV("blockable", blockable));
}

void Box::deserialize(const serialization::Deserializer<World>& ser)
{
  ser(S_NV("blocked", blocked), S_NV("blockable", blockable));
}
} // namespace engine::world
