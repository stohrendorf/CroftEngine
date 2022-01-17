#include "camerasink.h"

#include "serialization/serialization.h"

#include <exception>

namespace engine::world
{
class World;

void CameraSink::serialize(const serialization::Serializer<World>& ser)
{
  ser(S_NV("flags", flags));
}
} // namespace engine::world
