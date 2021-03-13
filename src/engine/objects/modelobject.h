#pragma once

#include "object.h"

namespace engine::objects
{
class ModelObject : public Object
{
protected:
  std::shared_ptr<SkeletalModelNode> m_skeleton;

public:
  ModelObject(const gsl::not_null<world::World*>& world, const core::RoomBoundPosition& position)
      : Object{world, position}
  {
  }

  ModelObject(const gsl::not_null<world::World*>& world,
              const gsl::not_null<const loader::file::Room*>& room,
              const loader::file::Item& item,
              bool hasUpdateFunction,
              const gsl::not_null<const loader::file::SkeletalModelType*>& model);

  ModelObject(const ModelObject&) = delete;

  ModelObject& operator=(const ModelObject&) = delete;

  ModelObject& operator=(ModelObject&&) = delete;

  ~ModelObject() override
  {
    if(m_skeleton != nullptr)
    {
      setParent(m_skeleton, nullptr);
    }
  }

  std::shared_ptr<render::scene::Node> getNode() const override
  {
    return m_skeleton;
  }

  const std::shared_ptr<SkeletalModelNode>& getSkeleton() const
  {
    return m_skeleton;
  }

  bool triggerSwitch(const core::Frame timeout) override
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

  loader::file::BoundingBox getBoundingBox() const override;

  bool isNear(const ModelObject& other, const core::Length& radius) const;

  bool isNear(const Particle& other, const core::Length& radius) const;

  bool testBoneCollision(const ModelObject& other);

  void enemyPush(CollisionInfo& collisionInfo, bool enableSpaz, bool withXZCollRadius);

  gsl::not_null<std::shared_ptr<Particle>>
    emitParticle(const core::TRVec& localPosition,
                 size_t boneIndex,
                 gsl::not_null<std::shared_ptr<Particle>> (*generate)(world::World& world,
                                                                      const core::RoomBoundPosition& pos,
                                                                      const core::Speed& speed,
                                                                      const core::Angle& angle));

  void serialize(const serialization::Serializer<world::World>& ser) override;

  static std::shared_ptr<ModelObject> create(serialization::Serializer<world::World>& ser);
};

class NullRenderModelObject : public ModelObject
{
public:
  NullRenderModelObject(const gsl::not_null<world::World*>& world, const core::RoomBoundPosition& position)
      : ModelObject{world, position}
  {
  }

  NullRenderModelObject(const gsl::not_null<world::World*>& world,
                        const gsl::not_null<const loader::file::Room*>& room,
                        const loader::file::Item& item,
                        bool hasUpdateFunction,
                        const gsl::not_null<const loader::file::SkeletalModelType*>& model)
      : ModelObject{world, room, item, hasUpdateFunction, model}
  {
    getSkeleton()->setRenderable(nullptr);
    getSkeleton()->removeAllChildren();
    getSkeleton()->clearParts();
  }

  void serialize(const serialization::Serializer<world::World>& ser) override;
};

std::shared_ptr<ModelObject> create(const serialization::TypeId<std::shared_ptr<ModelObject>>&,
                                    serialization::Serializer<world::World>& ser);
} // namespace engine::objects