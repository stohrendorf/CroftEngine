#pragma once

#include "core/containeroffset.h"
#include "core/id.h"
#include "io/sdlreader.h"

namespace loader::file
{
struct RoomVertex;

using VertexIndex = core::ContainerIndex<uint32_t, core::TRVec, int16_t, RoomVertex>;

struct Triangle
{
  //! Vertex buffer indices
  std::array<VertexIndex, 3> vertices;
  core::TextureTileId tileId{uint16_t(0)}; /**< \brief object-texture index or color index.
                             * If the triangle is textured, then this is an index into the object-texture list.
                             * If it's not textured, then the low 8 bit contain the index into the 256 color palette
                             * and from TR2 on the high 8 bit contain the index into the 16 bit palette.
                             */
  uint16_t lighting;                       /**< \brief transparency flag & strength of the highlight (TR4-TR5).
                             * bit0 if set, then alpha channel = intensity (see attribute in tr2_object_texture).<br>
                             * bit1-7 is the strength of the highlight.
                             */

  static Triangle readTr1(io::SDLReader& reader)
  {
    return read(reader, false);
  }

  static Triangle readTr4(io::SDLReader& reader)
  {
    return read(reader, true);
  }

private:
  static Triangle read(io::SDLReader& reader, const bool withLighting)
  {
    Triangle triangle;
    triangle.vertices[0] = reader.readU16();
    triangle.vertices[1] = reader.readU16();
    triangle.vertices[2] = reader.readU16();
    triangle.tileId = reader.readU16();
    if(withLighting)
      triangle.lighting = reader.readU16();
    else
      triangle.lighting = 0;
    return triangle;
  }
};

struct QuadFace
{
  //! Vertex buffer indices
  std::array<VertexIndex, 4> vertices;
  core::TextureTileId tileId{uint16_t(0)}; /**< \brief object-texture index or color index.
                             * If the rectangle is textured, then this is an index into the object-texture list.
                             * If it's not textured, then the low 8 bit contain the index into the 256 color palette
                             * and from TR2 on the high 8 bit contain the index into the 16 bit palette.
                             */
  uint16_t lighting;                       /**< \brief transparency flag & strength of the highlight (TR4-TR5).
                             *
                             * In TR4, objects can exhibit some kind of light reflection when seen from some particular angles.
                             * - bit0 if set, then alpha channel = intensity (see attribute in tr2_object_texture).
                             * - bit1-7 is the strength of the highlight.
                             */

  static QuadFace readTr1(io::SDLReader& reader)
  {
    return read(reader, false);
  }

  static QuadFace readTr4(io::SDLReader& reader)
  {
    return read(reader, true);
  }

private:
  static QuadFace read(io::SDLReader& reader, const bool withLighting)
  {
    QuadFace quadFace;
    quadFace.vertices[0] = reader.readU16();
    quadFace.vertices[1] = reader.readU16();
    quadFace.vertices[2] = reader.readU16();
    quadFace.vertices[3] = reader.readU16();
    quadFace.tileId = reader.readU16();
    if(withLighting)
      quadFace.lighting = reader.readU16();
    else
      quadFace.lighting = 0;
    return quadFace;
  }
};
} // namespace loader
