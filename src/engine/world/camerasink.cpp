#include "camerasink.h"

#include "serialization/serialization.h"

namespace engine::world
{
void CameraSink::serialize(const serialization::Serializer<World>& ser)
{
  ser(S_NV("flags", flags));
}
} // namespace engine::world
