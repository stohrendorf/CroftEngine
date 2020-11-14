#pragma once

#include "color.h"
#include "core/angle.h"
#include "core/boundingbox.h"
#include "texture.h"

namespace render
{
class TextureAnimator;

namespace scene
{
class Mesh;
}
} // namespace render

namespace loader::file
{
namespace io
{
class SDLReader;
}

struct RoomStaticMesh
{
  core::TRVec position{}; // world coords
  core::Angle rotation{};
  core::Shade shade{};           // Constant lighting; -1 means use mesh lighting
  int16_t intensity2{};          // Like Intensity 1, and almost always the same value [absent from TR1 data files]
  core::StaticMeshId meshId{0u}; // which StaticMesh item to draw
  FloatColor tint{};             // extracted from intensity

  /** \brief reads a room static mesh definition.
    *
    * rotation gets converted to float and scaled appropriately.
    * darkness gets converted, so it matches the 0-32768 range introduced in TR3.
    * intensity2 is introduced in TR2 and is set to darkness for TR1.
    */
  static RoomStaticMesh readTr1(io::SDLReader& reader);

  static RoomStaticMesh readTr2(io::SDLReader& reader);

  static RoomStaticMesh readTr3(io::SDLReader& reader);

  static RoomStaticMesh readTr4(io::SDLReader& reader);
};

struct StaticMesh
{
  core::StaticMeshId id{0u}; // Object Identifier (matched in Items[])
  uint16_t mesh = 0;         // mesh (offset into MeshPointers[])
  core::BoundingBox visibility_box;
  core::BoundingBox collision_box;
  uint16_t flags = 0; // Meaning uncertain; it is usually 2, and is 3 for objects Lara can travel through,
  // like TR2's skeletons and underwater vegetation
  std::shared_ptr<render::scene::Mesh> renderMesh{nullptr};

  [[nodiscard]] bool doNotCollide() const
  {
    return (flags & 1u) != 0;
  }

  [[nodiscard]] bool isVisible() const
  {
    return (flags & 2u) != 0;
  }

  [[nodiscard]] core::BoundingBox getCollisionBox(const core::TRVec& pos, const core::Angle& angle) const;

  static std::unique_ptr<StaticMesh> read(io::SDLReader& reader);
};
} // namespace loader::file
