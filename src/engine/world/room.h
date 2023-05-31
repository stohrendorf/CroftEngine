#pragma once

#include "core/magic.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/lighting.h"
#include "engine/particlecollection.h"
#include "sector.h"
#include "serialization/serialization_fwd.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <gl/buffer.h>
#include <gl/vertexbuffer.h>
#include <glm/fwd.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace engine::world
{
class World;
class WorldGeometry;
} // namespace engine::world

namespace render
{
class TextureAnimator;
struct AnimatedUV;
} // namespace render

namespace render::material
{
class MaterialManager;
class Material;
} // namespace render::material

namespace render::scene
{
class Mesh;
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
class Engine;
struct Location;
class Presenter;
} // namespace engine

namespace engine::world
{
struct Room;
struct StaticMesh;
struct RoomRenderVertex;
struct RoomRenderMesh;

struct Portal
{
  gsl::not_null<Room*> adjoiningRoom;
  glm::vec3 normal;
  std::array<glm::vec3, 4> vertices;
  std::shared_ptr<render::scene::Mesh> mesh;

  void buildMesh(const loader::file::Portal& srcPortal, const gslu::nn_shared<render::material::Material>& material);
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
  size_t physicalId{std::numeric_limits<size_t>::max()};

  bool isWaterRoom = false;
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
  std::vector<gslu::nn_shared<render::scene::Node>> sceneryNodes{};

  std::map<uint8_t, std::shared_ptr<render::scene::Node>> dustCache{};
  glm::vec3 verticesBBoxMin{std::numeric_limits<float>::max()};
  glm::vec3 verticesBBoxMax{std::numeric_limits<float>::lowest()};
  std::shared_ptr<render::scene::Node> dust = nullptr;
  mutable engine::InstancedParticleCollection particles{};
  std::unique_ptr<render::TextureAnimator> textureAnimator{};
  std::shared_ptr<gl::VertexBuffer<render::AnimatedUV>> uvCoordsBuffer{};

  void createSceneNode(const loader::file::Room& srcRoom,
                       size_t roomId,
                       World& world,
                       const std::vector<uint16_t>& textureAnimData,
                       render::material::MaterialManager& materialManager);

  [[nodiscard]] const Sector* getSectorByAbsolutePosition(const core::TRVec& worldPos) const
  {
    return getSectorByRelativePosition(worldPos - position);
  }

  [[nodiscard]] const Sector* getSectorByRelativePosition(const core::TRVec& localPos) const
  {
    return getSectorByIndex(sectorOf(localPos.X), sectorOf(localPos.Z));
  }

  [[nodiscard]] bool isInnerPositionXZ(core::TRVec worldPos) const
  {
    worldPos -= position;
    const auto sx = sectorOf(worldPos.X);
    const auto sz = sectorOf(worldPos.Z);
    return sx > 0 && sx < sectorCountX - 1 && sz > 0 && sz < sectorCountZ - 1;
  }

  [[nodiscard]] const Sector* getSectorByIndex(int dx, int dz) const;

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

  void serialize(const serialization::Serializer<World>& ser) const;
  void deserialize(const serialization::Deserializer<World>& ser);

  std::vector<engine::ShaderLight> bufferLights{};
  std::shared_ptr<gl::ShaderStorageBuffer<engine::ShaderLight>> lightsBuffer{};

  void collectShaderLights(size_t depth);
  void regenerateDust(const std::shared_ptr<engine::Presenter>& presenter,
                      const gslu::nn_shared<render::material::Material>& dustMaterial,
                      bool isDustEnabled,
                      uint8_t dustDensityDivisor);

private:
  std::shared_ptr<render::scene::Node>
    createParticleMesh(const std::string& label,
                       const gslu::nn_shared<render::material::Material>& dustMaterial,
                       uint8_t dustDensity) const;

  void buildMeshData(const WorldGeometry& worldGeometry,
                     const loader::file::Room& srcRoom,
                     std::vector<RoomRenderVertex>& vbufData,
                     std::vector<render::AnimatedUV>& uvCoordsData,
                     RoomRenderMesh& renderMesh) const;

  [[nodiscard]] gslu::nn_shared<render::scene::Mesh> buildMesh(const loader::file::Room& srcRoom,
                                                               const size_t roomId,
                                                               const Engine& engine,
                                                               const WorldGeometry& worldGeometry,
                                                               const std::vector<uint16_t>& textureAnimData);
};

extern void patchHeightsForBlock(const engine::objects::Object& object, const core::Length& height);

[[nodiscard]] extern std::optional<core::Length> getWaterSurfaceHeight(const Location& location);
} // namespace engine::world
