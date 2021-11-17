#pragma once

#include "core/containeroffset.h"
#include "core/id.h"
#include "core/units.h"
#include "core/vec.h"

#include <array>
#include <cstdint>

namespace loader::file
{
namespace io
{
class SDLReader;
}

struct RoomVertex;

using VertexIndex = core::ContainerIndex<uint32_t, core::TRVec, core::Shade, RoomVertex>;

struct Triangle
{
  //! Vertex buffer indices
  std::array<VertexIndex, 3> vertices{};
  core::TextureTileId tileId{uint16_t(0)}; /**< \brief object-texture index or color index.
                             * If the triangle is textured, then this is an index into the object-texture list.
                             * If it's not textured, then the low 8 bit contain the index into the 256 color palette
                             * and from TR2 on the high 8 bit contain the index into the 16 bit palette.
                             */
  uint16_t lighting = 0;                   /**< \brief transparency flag & strength of the highlight (TR4-TR5).
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
  static Triangle read(io::SDLReader& reader, bool withLighting);
};

struct QuadFace
{
  //! Vertex buffer indices
  std::array<VertexIndex, 4> vertices{};
  core::TextureTileId tileId{uint16_t(0)}; /**< \brief object-texture index or color index.
                             * If the rectangle is textured, then this is an index into the object-texture list.
                             * If it's not textured, then the low 8 bit contain the index into the 256 color palette
                             * and from TR2 on the high 8 bit contain the index into the 16 bit palette.
                             */
  uint16_t lighting = 0;                   /**< \brief transparency flag & strength of the highlight (TR4-TR5).
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
  static QuadFace read(io::SDLReader& reader, bool withLighting);
};
} // namespace loader::file
