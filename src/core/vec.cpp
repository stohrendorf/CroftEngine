#include "vec.h"

#include "serialization/not_null.h"
#include "serialization/room_ptr.h"

namespace core
{
void RoomBoundPosition::serialize(const serialization::Serializer& ser)
{
  ser(S_NVP(room), S_NVP(position));
}

RoomBoundPosition RoomBoundPosition::create(const serialization::Serializer& ser)
{
  const loader::file::Room* room = nullptr;
  TRVec position{};
  ser(S_NV("room", room), S_NV("position", position));
  return RoomBoundPosition{room, position};
}

void TRVec::serialize(const serialization::Serializer& ser)
{
  ser(S_NVP(X), S_NVP(Y), S_NVP(Z));
}
} // namespace core
