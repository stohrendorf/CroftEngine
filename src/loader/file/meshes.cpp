#include "meshes.h"

#include "io/sdlreader.h"
#include "io/util.h"

namespace loader::file
{
RoomStaticMesh RoomStaticMesh::readTr1(io::SDLReader& reader)
{
  RoomStaticMesh room_static_mesh;
  room_static_mesh.position = readCoordinates32(reader);
  room_static_mesh.rotation = core::auToAngle(reader.readI16());
  room_static_mesh.darkness = reader.readI16();
  room_static_mesh.meshId = core::StaticMeshId::type(reader.readU16());

  // only in TR2
  room_static_mesh.intensity2 = room_static_mesh.darkness;

  room_static_mesh.tint.b = room_static_mesh.tint.g = room_static_mesh.tint.r = room_static_mesh.intensity2 / 16384.0f;
  room_static_mesh.tint.a = 1.0f;
  return room_static_mesh;
}

RoomStaticMesh RoomStaticMesh::readTr2(io::SDLReader& reader)
{
  RoomStaticMesh room_static_mesh;
  room_static_mesh.position = readCoordinates32(reader);
  room_static_mesh.rotation = core::auToAngle(reader.readI16());
  room_static_mesh.darkness = reader.readI16();
  room_static_mesh.intensity2 = reader.readI16();
  room_static_mesh.meshId = core::StaticMeshId::type(reader.readU16());
  // make consistent
  if(room_static_mesh.darkness >= 0)
    room_static_mesh.darkness = (8191 - room_static_mesh.darkness) << 2;
  if(room_static_mesh.intensity2 >= 0)
    room_static_mesh.intensity2 = (8191 - room_static_mesh.intensity2) << 2;

  room_static_mesh.tint.b = room_static_mesh.tint.g = room_static_mesh.tint.r = room_static_mesh.intensity2 / 16384.0f;
  room_static_mesh.tint.a = 1.0f;
  return room_static_mesh;
}

RoomStaticMesh RoomStaticMesh::readTr3(io::SDLReader& reader)
{
  RoomStaticMesh room_static_mesh;
  room_static_mesh.position = readCoordinates32(reader);
  room_static_mesh.rotation = core::auToAngle(reader.readI16());
  room_static_mesh.darkness = reader.readI16();
  room_static_mesh.intensity2 = reader.readI16();
  room_static_mesh.meshId = core::StaticMeshId::type(reader.readU16());

  room_static_mesh.tint.r = (room_static_mesh.darkness & 0x001F) / 62.0f;

  room_static_mesh.tint.g = ((room_static_mesh.darkness & 0x03E0) >> 5) / 62.0f;

  room_static_mesh.tint.b = ((room_static_mesh.darkness & 0x7C00) >> 10) / 62.0f;
  room_static_mesh.tint.a = 1.0f;
  return room_static_mesh;
}

RoomStaticMesh RoomStaticMesh::readTr4(io::SDLReader& reader)
{
  RoomStaticMesh room_static_mesh;
  room_static_mesh.position = readCoordinates32(reader);
  room_static_mesh.rotation = core::auToAngle(reader.readI16());
  room_static_mesh.darkness = reader.readI16();
  room_static_mesh.intensity2 = reader.readI16();
  room_static_mesh.meshId = core::StaticMeshId::type(reader.readU16());

  room_static_mesh.tint.r = (room_static_mesh.darkness & 0x001F) / 31.0f;

  room_static_mesh.tint.g = ((room_static_mesh.darkness & 0x03E0) >> 5) / 31.0f;

  room_static_mesh.tint.b = ((room_static_mesh.darkness & 0x7C00) >> 10) / 31.0f;
  room_static_mesh.tint.a = 1.0f;
  return room_static_mesh;
}
} // namespace loader::file
