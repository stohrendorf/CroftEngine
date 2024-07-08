#pragma once

#include "animation.h"
#include "atlastile.h"
#include "core/id.h"
#include "engine/controllerbuttons.h"
#include "mesh.h"
#include "render/scene/mesh.h"
#include "render/textureanimator.h"
#include "sprite.h"
#include "staticmesh.h"
#include "transition.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

namespace loader::file
{
struct AnimFrame;
enum class AnimationId : uint16_t;
} // namespace loader::file

namespace loader::file::level
{
class Level;
};

namespace engine
{
class Engine;
}

namespace engine::world
{
struct SkeletalModelType;
class RenderMeshData;

class RoomGeometry final
{
public:
  explicit RoomGeometry(const gslu::nn_shared<render::scene::Mesh>& geometry,
                        const gslu::nn_shared<render::TextureAnimator>& textureAnimator,
                        const gslu::nn_shared<gl::VertexBuffer<render::AnimatedUV>>& uvBuffer)
      : m_geometry{geometry}
      , m_textureAnimator{textureAnimator}
      , m_uvBuffer{uvBuffer}
  {
  }

  [[nodiscard]] const auto& getGeometry() const noexcept
  {
    return m_geometry;
  }

  [[nodiscard]] std::shared_ptr<render::scene::Mesh> tryGetDustMesh(uint8_t i) const
  {
    if(const auto it = m_dustCache.find(i); it != m_dustCache.end())
      return it->second;

    return nullptr;
  }

  void setDustCache(uint8_t i, const gslu::nn_shared<render::scene::Mesh>& mesh)
  {
    gsl_Assert(m_dustCache.try_emplace(i, mesh).second);
  }

  void animateUv(const std::vector<AtlasTile>& atlasTiles)
  {
    m_textureAnimator->updateCoordinates(*m_uvBuffer, atlasTiles);
  }

private:
  gslu::nn_shared<render::scene::Mesh> m_geometry;
  gslu::nn_shared<render::TextureAnimator> m_textureAnimator;
  gslu::nn_shared<gl::VertexBuffer<render::AnimatedUV>> m_uvBuffer;
  std::map<uint8_t, gslu::nn_shared<render::scene::Mesh>> m_dustCache;
};

class WorldGeometry final
{
public:
  explicit WorldGeometry(Engine& engine, const loader::file::level::Level& level);
  ~WorldGeometry();

  [[nodiscard]] const std::unique_ptr<SpriteSequence>& findSpriteSequenceForType(const core::TypeId& type) const;
  [[nodiscard]] const StaticMesh* findStaticMeshById(const core::StaticMeshId& meshId) const;
  [[nodiscard]] const std::unique_ptr<SkeletalModelType>& findAnimatedModelForType(const core::TypeId& type) const;
  [[nodiscard]] gslu::nn_shared<RenderMeshData> getRenderMesh(size_t idx) const;
  core::TypeId find(const SkeletalModelType* model) const;
  core::TypeId find(const Sprite* sprite) const;
  bool isValid(const loader::file::AnimFrame* frame) const noexcept;
  [[nodiscard]] const Animation& getAnimation(loader::file::AnimationId id) const;

  [[nodiscard]] const auto& getAtlasTiles() const noexcept
  {
    return m_atlasTiles;
  }

  [[nodiscard]] const auto& getSprites() const noexcept
  {
    return m_sprites;
  }

  [[nodiscard]] const auto& getSpriteSequences() const noexcept
  {
    return m_spriteSequences;
  }

  [[nodiscard]] const auto& getMeshes() const noexcept
  {
    return m_meshes;
  }

  [[nodiscard]] const auto& getPoseFrames() const noexcept
  {
    return m_poseFrames;
  }

  [[nodiscard]] const auto& getAnimations() const noexcept
  {
    return m_animations;
  }

  [[nodiscard]] const auto& getAnimCommands() const noexcept
  {
    return m_animCommands;
  }

  [[nodiscard]] const auto& getControllerLayouts() const noexcept
  {
    return m_controllerLayouts;
  }

  [[nodiscard]] const auto& getPalette() const noexcept
  {
    return m_palette;
  }

  [[nodiscard]] std::shared_ptr<RoomGeometry> tryGetRoomGeometry(const size_t roomId) const
  {
    if(const auto it = m_roomGeometries.find(roomId); it != m_roomGeometries.end())
      return it->second;

    return nullptr;
  }

  void setRoomGeometry(const size_t roomId, const gslu::nn_shared<RoomGeometry>& roomGeometry)
  {
    gsl_Assert(m_roomGeometries.try_emplace(roomId, roomGeometry).second);
  }

private:
  void initAnimationData(const loader::file::level::Level& level);
  void initMeshes(const loader::file::level::Level& level);
  std::vector<gsl::not_null<const Mesh*>> initAnimatedModels(const loader::file::level::Level& level);
  void initStaticMeshes(const loader::file::level::Level& level,
                        const std::vector<gsl::not_null<const Mesh*>>& meshesDirect,
                        const Engine& engine);
  void initTextureDependentDataFromLevel(const loader::file::level::Level& level);
  void initTextures(Engine& engine, const loader::file::level::Level& level);
  void initSpriteMeshes(const Engine& engine);

  std::array<gl::SRGBA8, 256> m_palette;

  std::unordered_map<core::StaticMeshId, StaticMesh> m_staticMeshes;
  std::vector<Mesh> m_meshes;
  std::map<core::TypeId, std::unique_ptr<SkeletalModelType>> m_animatedModels;
  std::vector<Sprite> m_sprites;
  std::map<core::TypeId, std::unique_ptr<SpriteSequence>> m_spriteSequences;
  std::vector<AtlasTile> m_atlasTiles;

  std::vector<int16_t> m_poseFrames;
  std::vector<Animation> m_animations;
  std::vector<int32_t> m_boneTrees;
  std::vector<Transitions> m_transitions;
  std::vector<TransitionCase> m_transitionCases;
  std::vector<int16_t> m_animCommands;

  ControllerLayouts m_controllerLayouts;
  std::shared_ptr<gl::Texture2DArray<gl::PremultipliedSRGBA8>> m_allTextures;

  std::map<size_t, gslu::nn_shared<RoomGeometry>> m_roomGeometries;
};
} // namespace engine::world