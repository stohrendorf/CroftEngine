#pragma once

#include "core/angle.h"
#include "core/magic.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/lighting.h"
#include "qs/qs.h"
#include "sector.h"
#include "serialization/serialization_fwd.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <gl/buffer.h>
#include <glm/vec3.hpp>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <optional>
#include <vector>

namespace engine::world
{
class World;
}

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

namespace loader::file
{
struct Portal;
struct Room;
} // namespace loader::file

namespace engine
{
struct Location;
}

namespace engine::world
{
struct Room;
struct StaticMesh;

struct Portal
{
  gsl::not_null<Room*> adjoiningRoom;
  glm::vec3 normal;
  std::array<glm::vec3, 4> vertices;
  std::shared_ptr<render::scene::Mesh> mesh;

  void buildMesh(const loader::file::Portal& srcPortal,
                 const gsl::not_null<std::shared_ptr<render::scene::Material>>& material);
};

struct Light
{
  core::TRVec position;
  core::Intensity intensity;
  core::Length fadeDistance;
};

struct RoomStaticMesh
{
  core::TRVec position;
  core::Angle rotation;
  core::Shade shade;
  gsl::not_null<const StaticMesh*> staticMesh;
};

struct Room
{
  size_t physicalId;

  bool isWaterRoom;
  core::TRVec position{};
  int sectorCountZ{};
  int sectorCountX{};
  core::Shade ambientShade{};

  std::vector<Light> lights{};
  std::vector<Portal> portals{};
  std::vector<Sector> sectors{};
  std::vector<RoomStaticMesh> staticMeshes{};

  Room* alternateRoom{nullptr};

  std::shared_ptr<render::scene::Node> node = nullptr;
  std::vector<gsl::not_null<std::shared_ptr<render::scene::Node>>> sceneryNodes{};

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

  [[nodiscard]] const Sector* getSectorByIndex(int dx, int dz) const;

  [[nodiscard]] gsl::not_null<const Sector*> getInnerSectorByIndex(int dx, int dz) const
  {
    dx = std::clamp(dx, 1, sectorCountX - 2);
    dz = std::clamp(dz, 1, sectorCountZ - 2);
    return gsl::not_null{&sectors[sectorCountZ * dx + dz]};
  }

  [[nodiscard]] gsl::not_null<const Sector*> getBoundarySectorByIndex(int dx, int dz) const
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
    return gsl::not_null{getSectorByIndex(dx, dz)};
  }

  void resetScenery();

  void serialize(const serialization::Serializer<World>& ser);

  std::vector<engine::ShaderLight> bufferLights{};
  gsl::not_null<std::shared_ptr<gl::ShaderStorageBuffer<engine::ShaderLight>>> lightsBuffer{
    std::make_shared<gl::ShaderStorageBuffer<engine::ShaderLight>>("lights-buffer")};

  void collectShaderLights(size_t depth);

private:
  void createParticleMesh(const std::string& labe,
                          const glm::vec3& min,
                          const glm::vec3& max,
                          render::scene::MaterialManager& materialManager);
};

extern void patchHeightsForBlock(const engine::objects::Object& object, const core::Length& height);

[[nodiscard]] extern std::optional<core::Length> getWaterSurfaceHeight(const Location& location);
} // namespace engine::world
