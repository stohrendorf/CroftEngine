#pragma once

#include "core/angle.h"
#include "core/boundingbox.h"
#include "core/id.h"
#include "core/units.h"
#include "engine/lighting.h"
#include "object.h"
#include "objectstate.h"

#include <cstddef>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <string>

namespace core
{
struct TRVec;
}

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
protected:
  std::shared_ptr<SkeletalModelNode> m_skeleton;
  Lighting m_lighting;

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
              const gsl::not_null<const world::SkeletalModelType*>& model);

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

  bool testBoneCollision(const ModelObject& other);

  void enemyPush(CollisionInfo& collisionInfo, bool enableSpaz, bool withXZCollRadius);

  gsl::not_null<std::shared_ptr<Particle>>
    emitParticle(const core::TRVec& localPosition,
                 size_t boneIndex,
                 gsl::not_null<std::shared_ptr<Particle>> (*generate)(
                   world::World& world, const Location& location, const core::Speed& speed, const core::Angle& angle));

  void updateLighting() override;

  void collideWithLara(CollisionInfo& collisionInfo, bool push = true);

  void trapCollideWithLara(CollisionInfo& collisionInfo);

  void serialize(const serialization::Serializer<world::World>& ser) override;

  static std::shared_ptr<ModelObject> create(serialization::Serializer<world::World>& ser);
};

#define MODELOBJECT_DEFAULT_CONSTRUCTORS(CLASS, HAS_UPDATE_FUNCTION)             \
  CLASS(const gsl::not_null<world::World*>& world, const Location& location)     \
      : ModelObject{world, location}                                             \
  {                                                                              \
  }                                                                              \
                                                                                 \
  CLASS(const std::string& name,                                                 \
        const gsl::not_null<world::World*>& world,                               \
        const gsl::not_null<const world::Room*>& room,                           \
        const loader::file::Item& item,                                          \
        const gsl::not_null<const world::SkeletalModelType*>& animatedModel)     \
      : ModelObject{name, world, room, item, HAS_UPDATE_FUNCTION, animatedModel} \
  {                                                                              \
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

  void serialize(const serialization::Serializer<world::World>& ser) override;
};

std::shared_ptr<ModelObject> create(const serialization::TypeId<std::shared_ptr<ModelObject>>&,
                                    serialization::Serializer<world::World>& ser);
} // namespace engine::objects
