#include "vec.h"

#include "serialization/quantity.h"
#include "serialization/serialization.h"

#include <exception>
#include <ostream>

namespace core
{
void serialize(TRVec& v, const serialization::Serializer<engine::world::World>& ser)
{
  ser(S_NV("x", v.X), S_NV("y", v.Y), S_NV("z", v.Z));
}

std::ostream& operator<<(std::ostream& stream, const TRVec& rhs)
{
  return stream << "(" << rhs.X << ", " << rhs.Y << ", " << rhs.Z << ")";
}
} // namespace core
