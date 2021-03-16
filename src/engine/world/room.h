#pragma once

#include "core/id.h"
#include "core/units.h"
#include "core/vec.h"
#include "loader/file/datatypes.h"
#include "loader/file/meshes.h"
#include "sector.h"

namespace render
{
class TextureAnimator;
}

namespace render::scene
{
class MaterialManager;
class Mesh;
class Material;
class Node;
} // namespace render::scene

namespace engine::objects
{
class Object;
}

namespace engine::world
{
struct Room;

struct Portal
{
  gsl::not_null<Room*> adjoiningRoom;
  glm::vec3 normal;
  std::array<glm::vec3, 4> vertices;
  std::shared_ptr<render::scene::Mesh> mesh;

  void buildMesh(const loader::file::Portal& srcPortal,
                 const gsl::not_null<std::shared_ptr<render::scene::Material>>& material);
};

struct Room
{
  bool isWaterRoom;
  core::TRVec position{};
  int sectorCountZ{};
  int sectorCountX{};
  core::Shade ambientShade{};
  std::vector<loader::file::Light> lights{};
  core::RoomIdI16 alternateRoom{int16_t(-1)};
  std::vector<loader::file::RoomStaticMesh> staticMeshes{};

  std::vector<Portal> portals{};
  std::vector<Sector> sectors{};

  std::shared_ptr<render::scene::Node> node = nullptr;
  std::vector<std::shared_ptr<render::scene::Node>> sceneryNodes{};

  void createSceneNode(const loader::file::Room& srcRoom,
                       size_t roomId,
                       World&,
                       render::TextureAnimator& animator,
                       render::scene::MaterialManager& materialManager);

  [[nodiscard]] const Sector* getSectorByAbsolutePosition(const core::TRVec& worldPos) const
  {
    return getSectorByRelativePosition(worldPos - position);
  }

  [[nodiscard]] const Sector* getSectorByRelativePosition(const core::TRVec& localPos) const
  {
    return getSectorByIndex(localPos.X / core::SectorSize, localPos.Z / core::SectorSize);
  }

  [[nodiscard]] gsl::not_null<const Sector*> getInnerSectorByAbsolutePosition(core::TRVec worldPos) const
  {
    worldPos -= position;
    return getInnerSectorByIndex(worldPos.X / core::SectorSize, worldPos.Z / core::SectorSize);
  }

  [[nodiscard]] bool isInnerPositionXZ(core::TRVec worldPos) const
  {
    worldPos -= position;
    const auto sx = worldPos.X / core::SectorSize;
    const auto sz = worldPos.Z / core::SectorSize;
    return sx > 0 && sx < sectorCountX - 1 && sz > 0 && sz < sectorCountZ - 1;
  }

  [[nodiscard]] const Sector* getSectorByIndex(const int dx, const int dz) const;

  [[nodiscard]] gsl::not_null<const Sector*> getInnerSectorByIndex(int dx, int dz) const
  {
    dx = std::clamp(dx, 1, sectorCountX - 2);
    dz = std::clamp(dz, 1, sectorCountZ - 2);
    return &sectors[sectorCountZ * dx + dz];
  }

  [[nodiscard]] gsl::not_null<const Sector*> findFloorSectorWithClampedIndex(int dx, int dz) const
  {
    if(dz <= 0)
    {
      dz = 0;
      dx = std::clamp(dx, 1, sectorCountX - 2);
    }
    else if(dz >= sectorCountZ - 1)
    {
      dz = sectorCountZ - 1;
      dx = std::clamp(dx, 1, sectorCountX - 2);
    }
    else
    {
      dx = std::clamp(dx, 0, sectorCountX - 1);
    }
    return getSectorByIndex(dx, dz);
  }

  void resetScenery();

  void serialize(const serialization::Serializer<World>& ser);
};

extern gsl::not_null<const Sector*> findRealFloorSector(const core::TRVec& position,
                                                        const gsl::not_null<gsl::not_null<const Room*>*>& room);

inline gsl::not_null<const Sector*> findRealFloorSector(const core::TRVec& position, gsl::not_null<const Room*> room)
{
  return findRealFloorSector(position, &room);
}

inline gsl::not_null<const Sector*> findRealFloorSector(core::RoomBoundPosition& rbs)
{
  return findRealFloorSector(rbs.position, &rbs.room);
}

extern void patchHeightsForBlock(const engine::objects::Object& object, const core::Length& height);

[[nodiscard]] extern std::optional<core::Length> getWaterSurfaceHeight(const core::RoomBoundPosition& pos);
} // namespace engine::world
