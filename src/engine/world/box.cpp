#include "box.h"

#include "serialization/serialization.h"

#include <exception>

namespace engine::world
{
void Box::serialize(const serialization::Serializer<World>& ser)
{
  ser(S_NV("blocked", blocked), S_NV("blockable", blockable));
}
} // namespace engine::world
