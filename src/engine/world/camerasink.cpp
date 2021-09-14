#include "camerasink.h"

#include "serialization/serialization.h"

#include <exception>
#include <gsl/gsl-lite.hpp>

namespace engine::world
{
class World;

void CameraSink::serialize(const serialization::Serializer<World>& ser)
{
  ser(S_NV("flags", flags));
}
} // namespace engine::world
