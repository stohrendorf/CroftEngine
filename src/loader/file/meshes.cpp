#include "meshes.h"

#include "io/sdlreader.h"
#include "io/util.h"
#include "util/helpers.h"

namespace loader::file
{
RoomStaticMesh RoomStaticMesh::readTr1(io::SDLReader& reader)
{
  RoomStaticMesh room_static_mesh;
  room_static_mesh.position = readCoordinates32(reader);
  room_static_mesh.rotation = core::auToAngle(reader.readI16());
  room_static_mesh.shade = core::Shade{reader.readI16()};
  room_static_mesh.meshId = core::StaticMeshId::type(reader.readU16());

  // only in TR2
  room_static_mesh.intensity2 = room_static_mesh.shade.get();

  room_static_mesh.tint.b = room_static_mesh.tint.g = room_static_mesh.tint.r
    = gsl::narrow_cast<float>(room_static_mesh.intensity2) / 16384.0f;
  room_static_mesh.tint.a = 1.0f;
  return room_static_mesh;
}

RoomStaticMesh RoomStaticMesh::readTr2(io::SDLReader& reader)
{
  RoomStaticMesh room_static_mesh;
  room_static_mesh.position = readCoordinates32(reader);
  room_static_mesh.rotation = core::auToAngle(reader.readI16());
  room_static_mesh.shade = core::Shade{reader.readI16()};
  room_static_mesh.intensity2 = reader.readI16();
  room_static_mesh.meshId = core::StaticMeshId::type(reader.readU16());
  // make consistent
  if(room_static_mesh.shade.get() >= 0)
    room_static_mesh.shade = core::Shade{gsl::narrow<core::Shade::type>((8191 - room_static_mesh.shade.get()) * 4)};
  if(room_static_mesh.intensity2 >= 0)
    room_static_mesh.intensity2 = (8191 - room_static_mesh.intensity2) * 4;

  room_static_mesh.tint.b = room_static_mesh.tint.g = room_static_mesh.tint.r
    = gsl::narrow_cast<float>(room_static_mesh.intensity2) / 16384.0f;
  room_static_mesh.tint.a = 1.0f;
  return room_static_mesh;
}

RoomStaticMesh RoomStaticMesh::readTr3(io::SDLReader& reader)
{
  RoomStaticMesh room_static_mesh;
  room_static_mesh.position = readCoordinates32(reader);
  room_static_mesh.rotation = core::auToAngle(reader.readI16());
  room_static_mesh.shade = core::Shade{reader.readI16()};
  room_static_mesh.intensity2 = reader.readI16();
  room_static_mesh.meshId = core::StaticMeshId::type(reader.readU16());
  room_static_mesh.tint.r
    = gsl::narrow_cast<float>(util::bits(static_cast<uint16_t>(room_static_mesh.shade.get()), 0, 5)) / 62.0f;
  room_static_mesh.tint.g
    = gsl::narrow_cast<float>(util::bits(static_cast<uint16_t>(room_static_mesh.shade.get()), 5, 5)) / 62.0f;
  room_static_mesh.tint.b
    = gsl::narrow_cast<float>(util::bits(static_cast<uint16_t>(room_static_mesh.shade.get()), 10, 5)) / 62.0f;
  room_static_mesh.tint.a = 1.0f;
  return room_static_mesh;
}

RoomStaticMesh RoomStaticMesh::readTr4(io::SDLReader& reader)
{
  RoomStaticMesh room_static_mesh;
  room_static_mesh.position = readCoordinates32(reader);
  room_static_mesh.rotation = core::auToAngle(reader.readI16());
  room_static_mesh.shade = core::Shade{reader.readI16()};
  room_static_mesh.intensity2 = reader.readI16();
  room_static_mesh.meshId = core::StaticMeshId::type(reader.readU16());
  room_static_mesh.tint.r
    = gsl::narrow_cast<float>(util::bits(static_cast<uint16_t>(room_static_mesh.shade.get()), 0, 5)) / 31.0f;
  room_static_mesh.tint.g
    = gsl::narrow_cast<float>(util::bits(static_cast<uint16_t>(room_static_mesh.shade.get()), 5, 5)) / 31.0f;
  room_static_mesh.tint.b
    = gsl::narrow_cast<float>(util::bits(static_cast<uint16_t>(room_static_mesh.shade.get()), 10, 5)) / 31.0f;
  room_static_mesh.tint.a = 1.0f;
  return room_static_mesh;
}
} // namespace loader::file
