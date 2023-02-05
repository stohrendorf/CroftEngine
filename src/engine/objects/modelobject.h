#pragma once

#include "core/boundingbox.h"
#include "core/id.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/lighting.h"
#include "object.h"
#include "objectstate.h"
#include "serialization/serialization_fwd.h"

#include <cstddef>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <memory>
#include <string>

namespace engine
{
class Particle;
struct CollisionInfo;
struct Location;
class SkeletalModelNode;
} // namespace engine

namespace engine::world
{
class World;
struct Room;
struct SkeletalModelType;
} // namespace engine::world

namespace loader::file
{
struct Item;
}

namespace render::scene
{
class Node;
}

namespace engine::objects
{
class ModelObject : public Object
{
public:
  ModelObject(const gsl::not_null<world::World*>& world, const Location& location)
      : Object{world, location}
  {
  }

  ModelObject(const std::string& name,
              const gsl::not_null<world::World*>& world,
              const gsl::not_null<const world::Room*>& room,
              const loader::file::Item& item,
              bool hasUpdateFunction,
              const gsl::not_null<const world::SkeletalModelType*>& model,
              bool shadowCaster);

  ModelObject(const ModelObject&) = delete;
  ModelObject& operator=(const ModelObject&) = delete;
  ModelObject& operator=(ModelObject&&) = delete;

  ~ModelObject() override;

  std::shared_ptr<render::scene::Node> getNode() const override;

  const std::shared_ptr<SkeletalModelNode>& getSkeleton() const
  {
    return m_skeleton;
  }

  bool triggerSwitch(const core::Frame& timeout) override
  {
    if(m_state.triggerState != TriggerState::Deactivated)
    {
      return false;
    }

    if(m_state.current_anim_state == 0_as && timeout > 0_frame)
    {
      // switch has a timer
      m_state.timer = timeout;
      m_state.triggerState = TriggerState::Active;
    }
    else
    {
      deactivate();
      m_state.triggerState = TriggerState::Inactive;
    }

    return true;
  }

  void update() override;

  void applyMovement(bool forLara);

  core::BoundingBox getBoundingBox() const override;

  bool isNear(const ModelObject& other, const core::Length& radius) const;
  bool isNear(const Particle& other, const core::Length& radius) const;

  bool isNearInexact(const core::TRVec& other, const core::Length& radius) const;

  bool testBoneCollision(const ModelObject& other);

  void enemyPush(CollisionInfo& collisionInfo, bool enableSpaz, bool withXZCollRadius);

  gslu::nn_shared<Particle> emitParticle(const core::TRVec& localPosition,
                                         size_t boneIndex,
                                         gslu::nn_shared<Particle> (*generate)(world::World& world,
                                                                               const Location& location,
                                                                               const core::Speed& speed,
                                                                               const core::Angle& angle));

  void updateLighting() override;

  void collideWithLara(CollisionInfo& collisionInfo, bool push = true);

  void trapCollideWithLara(CollisionInfo& collisionInfo);

  void serialize(const serialization::Serializer<world::World>& ser) const override;
  void deserialize(const serialization::Deserializer<world::World>& ser) override;

  static std::shared_ptr<ModelObject> create(serialization::Deserializer<world::World>& ser);

protected:
  std::shared_ptr<SkeletalModelNode> m_skeleton;
  Lighting m_lighting;
};

#define MODELOBJECT_DEFAULT_CONSTRUCTORS(CLASS, HAS_UPDATE_FUNCTION, SHADOW_CASTER)             \
  CLASS(const gsl::not_null<world::World*>& world, const Location& location)                    \
      : ModelObject{world, location}                                                            \
  {                                                                                             \
  }                                                                                             \
                                                                                                \
  CLASS(const std::string& name,                                                                \
        const gsl::not_null<world::World*>& world,                                              \
        const gsl::not_null<const world::Room*>& room,                                          \
        const loader::file::Item& item,                                                         \
        const gsl::not_null<const world::SkeletalModelType*>& animatedModel)                    \
      : ModelObject{name, world, room, item, HAS_UPDATE_FUNCTION, animatedModel, SHADOW_CASTER} \
  {                                                                                             \
  }

class NullRenderModelObject : public ModelObject
{
public:
  NullRenderModelObject(const gsl::not_null<world::World*>& world, const Location& location)
      : ModelObject{world, location}
  {
  }

  NullRenderModelObject(const std::string& name,
                        const gsl::not_null<world::World*>& world,
                        const gsl::not_null<const world::Room*>& room,
                        const loader::file::Item& item,
                        bool hasUpdateFunction,
                        const gsl::not_null<const world::SkeletalModelType*>& model);

  void serialize(const serialization::Serializer<world::World>& ser) const override;
  void deserialize(const serialization::Deserializer<world::World>& ser) override;
};

std::shared_ptr<ModelObject> create(const serialization::TypeId<std::shared_ptr<ModelObject>>&,
                                    serialization::Deserializer<world::World>& ser);
} // namespace engine::objects
