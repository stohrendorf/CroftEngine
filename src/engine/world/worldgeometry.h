#pragma once

#include "atlastile.h"
#include "core/id.h"
#include "mesh.h"
#include "sprite.h"
#include "staticmesh.h"

#include <array>
#include <cstddef>
#include <gl/pixel.h>
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

namespace render::material
{
class MaterialManager;
}

namespace engine
{
class Engine;
}

namespace engine::world
{
struct SkeletalModelType;
class RenderMeshData;
struct Animation;
struct TransitionCase;
struct Transitions;

class WorldGeometry final
{
public:
  void init(loader::file::level::Level& level, const std::array<gl::SRGBA8, 256>& palette, const Engine& engine);
  void initTextureDependentDataFromLevel(const loader::file::level::Level& level);
  [[nodiscard]] const std::unique_ptr<SpriteSequence>& findSpriteSequenceForType(const core::TypeId& type) const;
  [[nodiscard]] const StaticMesh* findStaticMeshById(const core::StaticMeshId& meshId) const;
  [[nodiscard]] const std::unique_ptr<SkeletalModelType>& findAnimatedModelForType(const core::TypeId& type) const;
  [[nodiscard]] gslu::nn_shared<RenderMeshData> getRenderMesh(size_t idx) const;
  core::TypeId find(const SkeletalModelType* model) const;
  core::TypeId find(const Sprite* sprite) const;
  bool isValid(const loader::file::AnimFrame* frame) const noexcept;
  [[nodiscard]] const Animation& getAnimation(loader::file::AnimationId id) const;

  WorldGeometry();
  ~WorldGeometry();

  [[nodiscard]] auto& getAtlasTiles() noexcept
  {
    return m_atlasTiles;
  }

  [[nodiscard]] const auto& getAtlasTiles() const noexcept
  {
    return m_atlasTiles;
  }

  [[nodiscard]] auto& getSprites() noexcept
  {
    return m_sprites;
  }

  [[nodiscard]] const auto& getSprites() const noexcept
  {
    return m_sprites;
  }

  [[nodiscard]] const auto& getSpriteSequences() const noexcept
  {
    return m_spriteSequences;
  }

  [[nodiscard]] const auto& getStaticMeshes() const noexcept
  {
    return m_staticMeshes;
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

private:
  void initAnimationData(const loader::file::level::Level& level);
  void initMeshes(const loader::file::level::Level& level, const std::array<gl::SRGBA8, 256>& palette);
  std::vector<gsl::not_null<const Mesh*>> initAnimatedModels(const loader::file::level::Level& level);
  void initStaticMeshes(const loader::file::level::Level& level,
                        const std::vector<gsl::not_null<const Mesh*>>& meshesDirect,
                        const Engine& engine);

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
};
} // namespace engine::world