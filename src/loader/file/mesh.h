#pragma once

#include "color.h"
#include "core/vec.h"
#include "primitives.h"

#include <vector>

namespace loader::file
{
namespace io
{
class SDLReader;
}

struct Mesh
{
  static constexpr uint16_t FlatShaded = 0x0001;
  static constexpr uint16_t DrawAlways = 0x0002;

  uint16_t flags;
  core::TRVec collision_center;
  core::Length collision_radius;
  std::vector<core::TRVec> vertices;      //[NumVertices]; // list of vertices (relative coordinates)
  std::vector<core::TRVec> normals;       //[NumNormals]; // list of normals (if NumNormals is positive)
  std::vector<core::Shade> vertex_shades; //[-NumNormals]; // list of light values (if NumNormals is negative), 0..8191
  std::vector<QuadFace> textured_rectangles; //[NumTexturedRectangles]; // list of textured rectangles
  std::vector<Triangle> textured_triangles;  //[NumTexturedTriangles]; // list of textured triangles
  // the rest is not present in TR4
  std::vector<QuadFace> colored_rectangles; //[NumColoredRectangles]; // list of colored rectangles
  std::vector<Triangle> colored_triangles;  //[NumColoredTriangles]; // list of colored triangles

  bool isFlatShaded() const noexcept
  {
    return (flags & FlatShaded) != 0;
  }

  /** \brief reads mesh definition.
    *
    * The read num_normals value is positive when normals are available and negative when light
    * values are available. The values get set appropriately.
    */
  static std::unique_ptr<Mesh> readTr1(io::SDLReader& reader);
  static std::unique_ptr<Mesh> readTr4(io::SDLReader& reader);
};
} // namespace loader::file
