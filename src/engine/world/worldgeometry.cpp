#include "worldgeometry.h"

#include "animation.h"
#include "atlastile.h"
#include "engine/engine.h"
#include "engine/engineconfig.h"
#include "engine/presenter.h"
#include "loader/file/datatypes.h"
#include "loader/file/level/level.h"
#include "loader/file/mesh.h"
#include "loader/file/texture.h"
#include "mesh.h"
#include "render/rendersettings.h"
#include "render/scene/mesh.h"
#include "rendermeshdata.h"
#include "skeletalmodeltype.h"
#include "sprite.h"
#include "transition.h"

#include <algorithm>
#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <gl/pixel.h>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <iterator>
#include <stdexcept>
#include <utility>

namespace engine::world
{
const std::unique_ptr<SpriteSequence>& WorldGeometry::findSpriteSequenceForType(const core::TypeId& type) const
{
  const auto it = m_spriteSequences.find(type);
  if(it != m_spriteSequences.end())
    return it->second;

  static const std::unique_ptr<SpriteSequence> none;
  return none;
}

const StaticMesh* WorldGeometry::findStaticMeshById(const core::StaticMeshId& meshId) const
{
  auto it = m_staticMeshes.find(meshId);
  if(it != m_staticMeshes.end())
    return &it->second;

  return nullptr;
}

const std::unique_ptr<SkeletalModelType>& WorldGeometry::findAnimatedModelForType(const core::TypeId& type) const
{
  const auto it = m_animatedModels.find(type);
  if(it != m_animatedModels.end())
    return it->second;

  static const std::unique_ptr<SkeletalModelType> none;
  return none;
}

gslu::nn_shared<RenderMeshData> WorldGeometry::getRenderMesh(const size_t idx) const
{
  return m_meshes.at(idx).meshData;
}

core::TypeId WorldGeometry::find(const SkeletalModelType* model) const
{
  auto it = std::find_if(m_animatedModels.begin(),
                         m_animatedModels.end(),
                         [&model](const auto& item) noexcept
                         {
                           return item.second.get() == model;
                         });
  if(it != m_animatedModels.end())
    return it->first;

  BOOST_THROW_EXCEPTION(std::runtime_error("Cannot find model"));
}

core::TypeId WorldGeometry::find(const Sprite* sprite) const
{
  auto it = std::find_if(m_spriteSequences.begin(),
                         m_spriteSequences.end(),
                         [&sprite](const auto& sequence)
                         {
                           return !sequence.second->sprites.empty() && &sequence.second->sprites[0] == sprite;
                         });
  if(it != m_spriteSequences.end())
    return it->first;

  BOOST_THROW_EXCEPTION(std::runtime_error("Cannot find sprite"));
}

void WorldGeometry::initMeshes(const loader::file::level::Level& level, const std::array<gl::SRGBA8, 256>& palette)
{
  std::transform(level.m_meshes.begin(),
                 level.m_meshes.end(),
                 std::back_inserter(m_meshes),
                 [this, &palette](const loader::file::Mesh& mesh)
                 {
                   return Mesh{mesh.collision_center,
                               mesh.collision_radius,
                               gsl::make_shared<RenderMeshData>(mesh, m_atlasTiles, palette)};
                 });
}

void WorldGeometry::initTextureDependentDataFromLevel(const loader::file::level::Level& level)
{
  std::transform(level.m_textureTiles.begin(),
                 level.m_textureTiles.end(),
                 std::back_inserter(m_atlasTiles),
                 [](const loader::file::TextureTile& tile)
                 {
                   return AtlasTile{tile.textureKey,
                                    {tile.uvCoordinates[0].toGl(),
                                     tile.uvCoordinates[1].toGl(),
                                     tile.uvCoordinates[2].toGl(),
                                     tile.uvCoordinates[3].toGl()}};
                 });

  std::transform(level.m_sprites.begin(),
                 level.m_sprites.end(),
                 std::back_inserter(m_sprites),
                 [](const loader::file::Sprite& sprite)
                 {
                   return Sprite{sprite.atlas_id,
                                 sprite.uv0.toGl(),
                                 sprite.uv1.toGl(),
                                 sprite.render0,
                                 sprite.render1,
                                 nullptr,
                                 nullptr,
                                 {nullptr, nullptr}};
                 });

  for(const auto& [sequenceId, sequence] : level.m_spriteSequences)
  {
    gsl_Assert(sequence != nullptr);
    gsl_Assert(sequence->length <= 0);
    gsl_Assert(gsl::narrow<size_t>(sequence->offset - sequence->length) <= m_sprites.size());

    auto seq = std::make_unique<SpriteSequence>();
    *seq = SpriteSequence{sequence->type, gsl::make_span(&m_sprites.at(sequence->offset), -sequence->length)};
    const bool distinct = m_spriteSequences.emplace(sequenceId, std::move(seq)).second;
    gsl_Assert(distinct);
  }
}

void WorldGeometry::initStaticMeshes(const loader::file::level::Level& level,
                                     const std::vector<gsl::not_null<const Mesh*>>& meshesDirect,
                                     const Engine& engine)
{
  for(const auto& staticMesh : level.m_staticMeshes)
  {
    RenderMeshDataCompositor compositor;
    if(staticMesh.isVisible())
      compositor.append(*meshesDirect.at(staticMesh.mesh)->meshData, gl::SRGBA8{0, 0, 0, 0});
    auto mesh = compositor.toMesh(
      *engine.getPresenter().getMaterialManager(),
      false,
      false,
      []()
      {
        return false;
      },
      [&engine]()
      {
        const auto& settings = engine.getEngineConfig()->renderSettings;
        return !settings.lightingModeActive ? 0 : settings.lightingMode;
      },
      "static-mesh");
    mesh->getRenderState().setScissorTest(false);
    const bool distinct
      = m_staticMeshes.emplace(staticMesh.id, StaticMesh{staticMesh.collision_box, staticMesh.doNotCollide(), mesh})
          .second;

    gsl_Assert(distinct);
  }
}

std::vector<gsl::not_null<const Mesh*>> WorldGeometry::initAnimatedModels(const loader::file::level::Level& level)
{
  std::vector<gsl::not_null<const Mesh*>> meshesDirect;
  for(auto idx : level.m_meshIndices)
  {
    meshesDirect.emplace_back(&m_meshes.at(idx));
  }

  for(const auto& [modelId, model] : level.m_animatedModels)
  {
    if(model->pose_data_offset.index<decltype(m_poseFrames[0])>() >= m_poseFrames.size())
    {
      BOOST_LOG_TRIVIAL(warning) << "Pose frame data index "
                                 << model->pose_data_offset.index<decltype(m_poseFrames[0])>() << " out of range 0.."
                                 << m_poseFrames.size() - 1;
      continue;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    const auto frames = reinterpret_cast<const loader::file::AnimFrame*>(&model->pose_data_offset.from(m_poseFrames));
    if(model->nMeshes > 1)
    {
      model->boneTree = gsl::make_span(
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<const loader::file::BoneTreeEntry*>(&model->bone_index.from(m_boneTrees)),
        model->nMeshes - 1);
    }

    Animation* animations = nullptr;
    if(model->animation_index.index != 0xffff)
      animations = &model->animation_index.from(m_animations);

    std::vector<SkeletalModelType::Bone> bones;
    if(model->nMeshes > 0)
    {
      BOOST_ASSERT(model->boneTree.empty() || static_cast<size_t>(model->nMeshes) == model->boneTree.size() + 1);
      for(size_t i = 0; i < gsl::narrow_cast<size_t>(model->nMeshes); ++i)
      {
        const auto& mesh = (model->mesh_base_index + i).from(meshesDirect);
        bones.emplace_back(mesh->meshData,
                           mesh->collisionCenter,
                           mesh->collisionRadius,
                           i == 0 || model->boneTree.empty() ? std::nullopt
                                                             : std::make_optional(model->boneTree[i - 1]));
      }
    }

    m_animatedModels.emplace(modelId,
                             std::make_unique<SkeletalModelType>(SkeletalModelType{
                               model->type, model->mesh_base_index, std::move(bones), frames, animations}));
  }

  return meshesDirect;
}

bool WorldGeometry::isValid(const loader::file::AnimFrame* frame) const noexcept
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  return frame >= reinterpret_cast<const loader::file::AnimFrame*>(m_poseFrames.data())
         // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
         && frame + 1 <= reinterpret_cast<const loader::file::AnimFrame*>(m_poseFrames.data() + m_poseFrames.size());
}

const Animation& WorldGeometry::getAnimation(loader::file::AnimationId id) const
{
  return m_animations.at(static_cast<int>(id));
}

void WorldGeometry::initAnimationData(const loader::file::level::Level& level)
{
  m_animations.resize(level.m_animations.size());
  m_transitions.resize(level.m_transitions.size());
  for(size_t i = 0; i < m_animations.size(); ++i)
  {
    const auto& anim = level.m_animations[i];
    const loader::file::AnimFrame* frames = nullptr;

    if(anim.poseDataOffset.index<decltype(m_poseFrames[0])>() >= m_poseFrames.size())
    {
      BOOST_LOG_TRIVIAL(warning) << "Pose frame data index " << anim.poseDataOffset.index<decltype(m_poseFrames[0])>()
                                 << " out of range 0.." << m_poseFrames.size() - 1;
    }
    else
    {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      frames = reinterpret_cast<const loader::file::AnimFrame*>(&anim.poseDataOffset.from(m_poseFrames));
    }

    gsl_Assert(anim.nextAnimationIndex < m_animations.size());
    auto nextAnimation = &m_animations[anim.nextAnimationIndex];

    gsl_Assert(anim.animCommandCount == 0
               || (anim.animCommandIndex + anim.animCommandCount).exclusiveIn(m_animCommands));
    gsl_Assert(anim.transitionsCount == 0
               || (anim.transitionsIndex + anim.transitionsCount).exclusiveIn(m_transitions));
    gsl::span<const Transitions> transitions;
    if(anim.transitionsCount > 0)
      transitions = gsl::span{&anim.transitionsIndex.from(m_transitions), anim.transitionsCount};

    gsl_Assert(anim.segmentLength > 0_frame);
    gsl_Assert(anim.firstFrame <= anim.lastFrame);
    m_animations[i] = Animation{frames,
                                anim.segmentLength,
                                anim.state_id,
                                anim.speed,
                                anim.acceleration,
                                anim.firstFrame,
                                anim.lastFrame,
                                anim.nextFrame,
                                anim.animCommandCount,
                                anim.animCommandCount == 0 ? nullptr : &anim.animCommandIndex.from(m_animCommands),
                                nextAnimation,
                                transitions};
  }
  gsl_Ensures(m_animations.size() == level.m_animations.size());

  for(const auto& transitionCase : level.m_transitionCases)
  {
    const Animation* anim = nullptr;
    if(transitionCase.targetAnimationIndex.index < m_animations.size())
      anim = &transitionCase.targetAnimationIndex.from(m_animations);
    else
      BOOST_LOG_TRIVIAL(warning) << "Animation index " << transitionCase.targetAnimationIndex.index << " not less than "
                                 << m_animations.size();

    m_transitionCases.emplace_back(
      TransitionCase{transitionCase.firstFrame, transitionCase.lastFrame, transitionCase.targetFrame, anim});
  }
  gsl_Ensures(m_transitionCases.size() == level.m_transitionCases.size());

  gsl_Assert(m_transitions.size() == level.m_transitions.size());
  std::transform(
    level.m_transitions.begin(),
    level.m_transitions.end(),
    m_transitions.begin(),
    [this](const loader::file::Transitions& transitions)
    {
      gsl_Assert((transitions.firstTransitionCase + transitions.transitionCaseCount).exclusiveIn(m_transitionCases));
      if(transitions.transitionCaseCount > 0)
        return Transitions{
          transitions.stateId,
          gsl::span{&transitions.firstTransitionCase.from(m_transitionCases), transitions.transitionCaseCount}};
      return Transitions{};
    });
  gsl_Ensures(m_transitions.size() == level.m_transitions.size());
}

void WorldGeometry::init(loader::file::level::Level& level,
                         const std::array<gl::SRGBA8, 256>& palette,
                         const Engine& engine)
{
  m_poseFrames = std::move(level.m_poseFrames);
  m_animCommands = std::move(level.m_animCommands);
  m_boneTrees = std::move(level.m_boneTrees);
  initAnimationData(level);
  initMeshes(level, palette);
  const auto meshesDirect = initAnimatedModels(level);
  initStaticMeshes(level, meshesDirect, engine);
}

WorldGeometry::WorldGeometry() = default;
WorldGeometry::~WorldGeometry() = default;
} // namespace engine::world