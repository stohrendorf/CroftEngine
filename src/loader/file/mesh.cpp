#include "mesh.h"

#include "color.h"
#include "datatypes.h"
#include "io/sdlreader.h"
#include "io/util.h"
#include "render/scene/mesh.h"
#include "rendermeshdata.h"
#include "util.h"

#include <gl/vertexarray.h>
#include <utility>

namespace loader::file
{
std::unique_ptr<Mesh> Mesh::readTr1(io::SDLReader& reader)
{
  std::unique_ptr<Mesh> mesh{std::make_unique<Mesh>()};
  mesh->center = readCoordinates16(reader);
  mesh->collision_size = core::Length{core::Length::type{reader.readI16()}};
  reader.skip(2); // some unknown flags

  reader.readVector(mesh->vertices, reader.readU16(), &io::readCoordinates16);

  const auto num_normals = reader.readI16();
  if(num_normals >= 0)
  {
    Expects(static_cast<size_t>(num_normals) == mesh->vertices.size());
    reader.readVector(mesh->normals, num_normals, &io::readCoordinates16);
  }
  else
  {
    Expects(static_cast<size_t>(-num_normals) == mesh->vertices.size());
    reader.readVector(mesh->vertexShades, -num_normals);
  }

  reader.readVector(mesh->textured_rectangles, reader.readU16(), &QuadFace::readTr1);
  reader.readVector(mesh->textured_triangles, reader.readU16(), &Triangle::readTr1);
  reader.readVector(mesh->colored_rectangles, reader.readU16(), &QuadFace::readTr1);
  reader.readVector(mesh->colored_triangles, reader.readU16(), &Triangle::readTr1);

  return mesh;
}

std::unique_ptr<Mesh> Mesh::readTr4(io::SDLReader& reader)
{
  std::unique_ptr<Mesh> mesh{std::make_unique<Mesh>()};
  mesh->center = readCoordinates16(reader);
  mesh->collision_size = core::Length{reader.readI32()};

  reader.readVector(mesh->vertices, reader.readU16(), &io::readCoordinates16);

  const auto num_normals = reader.readI16();
  if(num_normals >= 0)
  {
    reader.readVector(mesh->normals, num_normals, &io::readCoordinates16);
  }
  else
  {
    reader.readVector(mesh->vertexShades, -num_normals);
  }

  reader.readVector(mesh->textured_rectangles, reader.readU16(), &QuadFace::readTr4);
  reader.readVector(mesh->textured_triangles, reader.readU16(), &Triangle::readTr4);

  return mesh;
}
} // namespace loader::file
