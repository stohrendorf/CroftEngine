#include "mesh.h"

#include "io/sdlreader.h"
#include "io/util.h"
#include "loader/file/primitives.h"
#include "qs/quantity.h"

#include <cstddef>
#include <gsl/gsl-lite.hpp>

namespace loader::file
{
Mesh Mesh::readTr1(io::SDLReader& reader)
{
  Mesh mesh{};
  mesh.collision_center = readCoordinates16(reader);
  mesh.collision_radius = core::Length{gsl::narrow_cast<core::Length::type>(reader.readI16())};
  mesh.flags = reader.readU16();

  reader.readVector(mesh.vertices, reader.readU16(), &io::readCoordinates16);

  const auto num_normals = reader.readI16();
  if(num_normals >= 0)
  {
    gsl_Assert(static_cast<size_t>(num_normals) == mesh.vertices.size());
    reader.readVector(mesh.normals, num_normals, &io::readCoordinates16);
  }
  else
  {
    gsl_Assert(static_cast<size_t>(-num_normals) == mesh.vertices.size());
    reader.readVector(mesh.vertex_shades, -num_normals);
  }

  reader.readVector(mesh.textured_rectangles, reader.readU16(), &QuadFace::readTr1);
  reader.readVector(mesh.textured_triangles, reader.readU16(), &Triangle::readTr1);
  reader.readVector(mesh.colored_rectangles, reader.readU16(), &QuadFace::readTr1);
  reader.readVector(mesh.colored_triangles, reader.readU16(), &Triangle::readTr1);

  return mesh;
}

Mesh Mesh::readTr4(io::SDLReader& reader)
{
  Mesh mesh{};
  mesh.collision_center = readCoordinates16(reader);
  mesh.collision_radius = core::Length{gsl::narrow_cast<core::Length::type>(reader.readI32())};

  reader.readVector(mesh.vertices, reader.readU16(), &io::readCoordinates16);

  const auto num_normals = reader.readI16();
  if(num_normals >= 0)
  {
    reader.readVector(mesh.normals, num_normals, &io::readCoordinates16);
  }
  else
  {
    reader.readVector(mesh.vertex_shades, -num_normals);
  }

  reader.readVector(mesh.textured_rectangles, reader.readU16(), &QuadFace::readTr4);
  reader.readVector(mesh.textured_triangles, reader.readU16(), &Triangle::readTr4);

  return mesh;
}
} // namespace loader::file
