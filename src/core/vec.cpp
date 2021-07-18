#include "vec.h"

#include "engine/world/world.h"
#include "loader/file/datatypes.h"
#include "loader/file/level/level.h"
#include "serialization/not_null.h"
#include "serialization/quantity.h"
#include "serialization/serialization.h"
#include "serialization/vector_element.h"

namespace core
{
void TRVec::serialize(const serialization::Serializer<engine::world::World>& ser)
{
  ser(S_NV("x", X), S_NV("y", Y), S_NV("z", Z));
}

std::ostream& operator<<(std::ostream& stream, const TRVec& rhs)
{
  return stream << "(" << rhs.X << ", " << rhs.Y << ", " << rhs.Z << ")";
}
} // namespace core
