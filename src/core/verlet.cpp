#include "verlet.h"

#include "serialization/quantity.h"
#include "serialization/serialization.h"

namespace core
{
void Verlet::serialize(const serialization::Serializer<engine::world::World>& ser)
{
  ser(S_NV("velocity", velocity), S_NV("acceleration", acceleration));
}
} // namespace core
