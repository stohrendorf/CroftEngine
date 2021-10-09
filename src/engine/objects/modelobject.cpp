#include "modelobject.h"

#include "core/interval.h"
#include "core/magic.h"
#include "core/vec.h"
#include "engine/collisioninfo.h"
#include "engine/location.h"
#include "engine/objectmanager.h"
#include "engine/particle.h"
#include "engine/skeletalmodelnode.h"
#include "engine/soundeffects_tr1.h"
#include "engine/world/animation.h"
#include "engine/world/world.h"
#include "laraobject.h"
#include "loader/file/animation.h"
#include "object.h"
#include "objectstate.h"
#include "qs/qs.h"
#include "render/scene/node.h"
#include "serialization/serialization.h"
#include "util/helpers.h"

#include <algorithm>
#include <bitset>
#include <boost/assert.hpp>
#include <cstdint>
#include <exception>
#include <glm/geometric.hpp>
#include <optional>
#include <vector>

namespace engine::objects
{
ModelObject::ModelObject(const std::string& name,
                         const gsl::not_null<world::World*>& world,
                         const gsl::not_null<const world::Room*>& room,
                         const loader::file::Item& item,
                         const bool hasUpdateFunction,
                         const gsl::not_null<const world::SkeletalModelType*>& model)
    : Object{world, room, item, hasUpdateFunction}
    , m_skeleton{std::make_shared<SkeletalModelNode>(name, world, model)}
{
  SkeletalModelNode::buildMesh(m_skeleton, m_state.current_anim_state);
  m_lighting.bind(*m_skeleton);
}

void ModelObject::update()
{
  const auto endOfAnim = m_skeleton->advanceFrame(m_state);

  m_state.is_hit = false;
  m_state.touch_bits = 0;

  const auto& anim = getSkeleton()->getAnim();
  if(endOfAnim)
  {
    const auto* cmd = anim->animCommandCount == 0 ? nullptr : anim->animCommands;
    for(uint16_t i = 0; i < anim->animCommandCount; ++i)
    {
      BOOST_ASSERT(cmd <= &getWorld().getAnimCommands().back());
      const auto opcode = static_cast<AnimCommandOpcode>(*cmd);
      ++cmd;
      switch(opcode)
      {
      case AnimCommandOpcode::SetPosition:
        moveLocal(core::TRVec{core::Length{static_cast<core::Length::type>(cmd[0])},
                              core::Length{static_cast<core::Length::type>(cmd[1])},
                              core::Length{static_cast<core::Length::type>(cmd[2])}});
        cmd += 3;
        break;
      case AnimCommandOpcode::StartFalling:
        m_state.fallspeed = core::Speed{static_cast<core::Speed::type>(cmd[0])};
        m_state.speed = core::Speed{static_cast<core::Speed::type>(cmd[1])};
        m_state.falling = true;
        cmd += 2;
        break;
        // NOLINTNEXTLINE(bugprone-branch-clone)
      case AnimCommandOpcode::PlaySound:
        cmd += 2;
        break;
      case AnimCommandOpcode::PlayEffect:
        cmd += 2;
        break;
      case AnimCommandOpcode::Kill:
        m_state.triggerState = TriggerState::Deactivated;
        break;
      default:
        break;
      }
    }

    m_skeleton->setAnimation(m_state.current_anim_state, gsl::not_null{anim->nextAnimation}, anim->nextFrame);
    m_state.goal_anim_state = m_state.current_anim_state;
    if(m_state.current_anim_state == m_state.required_anim_state)
      m_state.required_anim_state = 0_as;
  }

  const auto* cmd = anim->animCommandCount == 0 ? nullptr : anim->animCommands;
  for(uint16_t i = 0; i < anim->animCommandCount; ++i)
  {
    BOOST_ASSERT(cmd != nullptr && cmd <= &getWorld().getAnimCommands().back());
    const auto opcode = static_cast<AnimCommandOpcode>(*cmd);
    ++cmd;
    switch(opcode)
    {
    case AnimCommandOpcode::SetPosition:
      cmd += 3;
      break;
    case AnimCommandOpcode::StartFalling:
      cmd += 2;
      break;
    case AnimCommandOpcode::PlaySound:
      if(getSkeleton()->getFrame().get() == cmd[0])
      {
        playSoundEffect(static_cast<TR1SoundEffect>(cmd[1]));
      }
      cmd += 2;
      break;
    case AnimCommandOpcode::PlayEffect:
      if(getSkeleton()->getFrame().get() == cmd[0])
      {
        getWorld().runEffect(cmd[1], this);
      }
      cmd += 2;
      break;
    default:
      break;
    }
  }

  applyMovement(false);
}

void ModelObject::applyMovement(const bool forLara)
{
  if(!m_state.falling)
  {
    m_state.speed = m_skeleton->calculateFloorSpeed();
  }
  else
  {
    if(m_state.fallspeed >= core::TerminalSpeed)
    {
      m_state.fallspeed += core::TerminalGravity * 1_frame;
    }
    else
    {
      m_state.fallspeed += core::Gravity * 1_frame;
    }

    if(forLara)
    {
      // we only add acceleration here
      m_state.speed
        = m_state.speed + m_skeleton->calculateFloorSpeed(0_frame) - m_skeleton->calculateFloorSpeed(-1_frame);
    }
  }

  m_state.location.move(util::pitch(m_state.speed * 1_frame, getMovementAngle())
                        + core::TRVec{0_len, (m_state.falling ? m_state.fallspeed : 0_spd) * 1_frame, 0_len});
  if(!forLara)
  {
    m_state.location.updateRoom();
    setCurrentRoom(m_state.location.room);
  }

  applyTransform();

  m_skeleton->updatePose();
}

core::BoundingBox ModelObject::getBoundingBox() const
{
  return m_skeleton->getBoundingBox();
}

bool ModelObject::isNear(const ModelObject& other, const core::Length& radius) const
{
  const auto bbox = getSkeleton()->getInterpolationInfo().getNearestFrame()->bbox.toBBox();
  const auto otherBBox = other.getSkeleton()->getInterpolationInfo().getNearestFrame()->bbox.toBBox();
  const auto selfY = m_state.location.position.Y + bbox.y;
  const auto otherY = other.m_state.location.position.Y + otherBBox.y;
  if(!selfY.intersectsExclusive(otherY))
  {
    return false;
  }

  const auto xz = util::pitch(other.m_state.location.position - m_state.location.position, -m_state.rotation.Y);
  return bbox.x.broadened(radius).contains(xz.X) && bbox.z.broadened(radius).contains(xz.Z);
}

bool ModelObject::isNear(const Particle& other, const core::Length& radius) const
{
  const auto frame = getSkeleton()->getInterpolationInfo().getNearestFrame();
  const auto bbox = frame->bbox.toBBox();
  const auto selfY = m_state.location.position.Y + bbox.y;
  if(!selfY.containsExclusive(other.location.position.Y))
  {
    return false;
  }

  const auto xz = util::pitch(other.location.position - m_state.location.position, -m_state.rotation.Y);
  return bbox.x.broadened(radius).contains(xz.X) && bbox.z.broadened(radius).contains(xz.Z);
}

void ModelObject::enemyPush(CollisionInfo& collisionInfo, const bool enableSpaz, const bool withXZCollRadius)
{
  auto& lara = getWorld().getObjectManager().getLara();

  const auto enemyToLara = lara.m_state.location.position - m_state.location.position;
  auto bbox = m_skeleton->getInterpolationInfo().getNearestFrame()->bbox.toBBox();
  if(withXZCollRadius)
  {
    const auto r = collisionInfo.collisionRadius;
    bbox.x = bbox.x.broadened(r);
    bbox.z = bbox.z.broadened(r);
  }

  // determine which edge lara is closest to
  auto laraInBBox = util::pitch(enemyToLara, -m_state.rotation.Y);
  const auto distMinX = laraInBBox.X - bbox.x.min;
  const auto distMaxX = bbox.x.max - laraInBBox.X;
  const auto distMinZ = laraInBBox.Z - bbox.z.min;
  const auto distMaxZ = bbox.z.max - laraInBBox.Z;
  const auto closestEdge = std::min(std::min(distMinX, distMaxX), std::min(distMinZ, distMaxZ));
  if(closestEdge <= 0_len)
    return;

  // push lara out to the closest edge
  if(distMinX == closestEdge)
  {
    laraInBBox.X = bbox.x.min;
  }
  else if(distMaxX == closestEdge)
  {
    laraInBBox.X = bbox.x.max;
  }
  else if(distMinZ == closestEdge)
  {
    laraInBBox.Z = bbox.z.min;
  }
  else
  {
    BOOST_ASSERT(distMaxZ == closestEdge);
    laraInBBox.Z = bbox.z.max;
  }
  // update lara's position to where she was pushed
  lara.m_state.location.position = m_state.location.position + util::pitch(laraInBBox, m_state.rotation.Y);
  if(enableSpaz)
  {
    const auto tmp = enemyToLara - util::pitch(core::TRVec{bbox.x.mid(), 0_len, bbox.z.mid()}, m_state.rotation.Y);
    const auto a = angleFromAtan(tmp.X, tmp.Z) - 180_deg;
    lara.hit_direction = axisFromAngle(lara.m_state.rotation.Y - a);
    if(lara.hit_frame == 0_frame)
    {
      lara.playSoundEffect(TR1SoundEffect::LaraOof);
    }
    lara.hit_frame += 1_frame;
    if(lara.hit_frame > 34_frame)
    {
      lara.hit_frame = 34_frame;
    }
  }
  collisionInfo.floorCollisionRangeMin = core::HeightLimit;
  collisionInfo.floorCollisionRangeMax = -384_len;
  collisionInfo.ceilingCollisionRangeMin = 0_len;
  const auto facingAngle = collisionInfo.facingAngle;
  collisionInfo.facingAngle = angleFromAtan(lara.m_state.location.position.X - collisionInfo.initialPosition.X,
                                            lara.m_state.location.position.Z - collisionInfo.initialPosition.Z);
  collisionInfo.initHeightInfo(lara.m_state.location.position, getWorld(), core::LaraWalkHeight);
  collisionInfo.facingAngle = facingAngle;
  if(collisionInfo.collisionType != CollisionInfo::AxisColl::None)
  {
    lara.m_state.location.position.X = collisionInfo.initialPosition.X;
    lara.m_state.location.position.Z = collisionInfo.initialPosition.Z;
  }
  else
  {
    collisionInfo.initialPosition = lara.m_state.location.position;
    lara.updateFloorHeight(-10_len);
  }
}

bool ModelObject::testBoneCollision(const ModelObject& other)
{
  m_state.touch_bits.reset();
  const auto boneSpheres = m_skeleton->getBoneCollisionSpheres();
  const auto otherSpheres = other.m_skeleton->getBoneCollisionSpheres();
  for(size_t boneSphereIdx = 0; boneSphereIdx < boneSpheres.size(); ++boneSphereIdx)
  {
    const auto& boneSphere = boneSpheres[boneSphereIdx];
    if(boneSphere.radius <= 0_len)
      continue;

    for(const auto& otherSphere : otherSpheres)
    {
      if(otherSphere.radius <= 0_len)
        continue;

      const auto distance = glm::distance(otherSphere.getCollisionPosition(), boneSphere.getCollisionPosition());
      const auto radii = (boneSphere.radius + otherSphere.radius).get<float>();
      if(distance >= radii)
        continue;

      m_state.touch_bits.set(boneSphereIdx);
      break;
    }
  }

  return m_state.touch_bits.any();
}

gsl::not_null<std::shared_ptr<Particle>>
  ModelObject::emitParticle(const core::TRVec& localPosition,
                            const size_t boneIndex,
                            gsl::not_null<std::shared_ptr<Particle>> (*generate)(
                              world::World& world, const Location&, const core::Speed&, const core::Angle&))
{
  BOOST_ASSERT(generate != nullptr);
  BOOST_ASSERT(boneIndex < m_skeleton->getBoneCount());

  const auto boneSpheres = m_skeleton->getBoneCollisionSpheres();
  BOOST_ASSERT(boneIndex < boneSpheres.size());

  auto location = m_state.location;
  location.position = core::TRVec{boneSpheres.at(boneIndex).relative(localPosition.toRenderSystem())};
  auto particle = generate(getWorld(), location, m_state.speed, m_state.rotation.Y);
  getWorld().getObjectManager().registerParticle(particle);

  return particle;
}

void ModelObject::updateLighting()
{
  m_lighting.update(core::Shade{core::Shade::type{-1}}, *m_state.location.room);
}

void ModelObject::serialize(const serialization::Serializer<world::World>& ser)
{
  Object::serialize(ser);
  ser(S_NV("skeleton", m_skeleton));
  if(ser.loading)
  {
    SkeletalModelNode::buildMesh(m_skeleton, m_state.current_anim_state);
    m_lighting.bind(*m_skeleton);
  }
}

std::shared_ptr<ModelObject> ModelObject::create(serialization::Serializer<world::World>& ser)
{
  auto result = std::make_shared<ModelObject>(gsl::not_null{&ser.context}, Location::create(ser["@location"]));
  result->serialize(ser);
  return result;
}

void ModelObject::collideWithLara(CollisionInfo& collisionInfo, bool push)
{
  const auto& lara = getWorld().getObjectManager().getLara();

  if(!isNear(lara, collisionInfo.collisionRadius))
    return;

  if(!testBoneCollision(lara))
    return;

  if(!push || !collisionInfo.policies.is_set(CollisionInfo::PolicyFlags::EnableBaddiePush))
    return;

  enemyPush(collisionInfo, false, true);
}

void ModelObject::trapCollideWithLara(CollisionInfo& collisionInfo)
{
  if(m_state.triggerState == TriggerState::Active)
  {
    collideWithLara(collisionInfo, false);
  }
  else if(m_state.triggerState != TriggerState::Invisible)
  {
    collideWithLara(collisionInfo);
  }
}

ModelObject::~ModelObject()
{
  if(m_skeleton != nullptr)
  {
    setParent(gsl::not_null{m_skeleton}, nullptr);
  }
}

std::shared_ptr<render::scene::Node> ModelObject::getNode() const
{
  return m_skeleton;
}

std::shared_ptr<ModelObject> create(const serialization::TypeId<std::shared_ptr<ModelObject>>&,
                                    serialization::Serializer<world::World>& ser)
{
  return ModelObject::create(ser);
}

void NullRenderModelObject::serialize(const serialization::Serializer<world::World>& ser)
{
  ModelObject::serialize(ser);
  if(ser.loading)
  {
    getSkeleton()->setRenderable(nullptr);
    getSkeleton()->removeAllChildren();
    getSkeleton()->clearParts();
  }
}

NullRenderModelObject::NullRenderModelObject(const std::string& name,
                                             const gsl::not_null<world::World*>& world,
                                             const gsl::not_null<const world::Room*>& room,
                                             const loader::file::Item& item,
                                             bool hasUpdateFunction,
                                             const gsl::not_null<const world::SkeletalModelType*>& model)
    : ModelObject{name, world, room, item, hasUpdateFunction, model}
{
  getSkeleton()->setRenderable(nullptr);
  getSkeleton()->removeAllChildren();
  getSkeleton()->clearParts();
}
} // namespace engine::objects
