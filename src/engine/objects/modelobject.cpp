#include "modelobject.h"

#include "engine/particle.h"
#include "engine/world/animation.h"
#include "engine/world/world.h"
#include "laraobject.h"
#include "loader/file/item.h"
#include "serialization/serialization.h"

#include <boost/range/adaptor/indexed.hpp>

namespace engine::objects
{
ModelObject::ModelObject(const gsl::not_null<world::World*>& world,
                         const gsl::not_null<const world::Room*>& room,
                         const loader::file::Item& item,
                         const bool hasUpdateFunction,
                         const gsl::not_null<const world::SkeletalModelType*>& model)
    : Object{world, room, item, hasUpdateFunction}
    , m_skeleton{std::make_shared<SkeletalModelNode>(
        std::string("skeleton(type:") + toString(item.type.get_as<TR1ItemId>()) + ")", world, model)}
{
  SkeletalModelNode::buildMesh(m_skeleton, m_state.current_anim_state);
  m_lighting.bind(*m_skeleton);
}

void ModelObject::update()
{
  const auto endOfAnim = m_skeleton->advanceFrame(m_state);

  m_state.is_hit = false;
  m_state.touch_bits = 0;

  if(endOfAnim)
  {
    const auto* cmd
      = getSkeleton()->getAnim()->animCommandCount == 0 ? nullptr : getSkeleton()->getAnim()->animCommands;
    for(uint16_t i = 0; i < getSkeleton()->getAnim()->animCommandCount; ++i)
    {
      BOOST_ASSERT(cmd < &getWorld().getAnimCommands().back());
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
      case AnimCommandOpcode::PlaySound: cmd += 2; break;
      case AnimCommandOpcode::PlayEffect: cmd += 2; break;
      case AnimCommandOpcode::Kill: m_state.triggerState = TriggerState::Deactivated; break;
      default: break;
      }
    }

    m_skeleton->setAnimation(
      m_state.current_anim_state, getSkeleton()->getAnim()->nextAnimation, getSkeleton()->getAnim()->nextFrame);
    m_state.goal_anim_state = m_state.current_anim_state;
    if(m_state.current_anim_state == m_state.required_anim_state)
      m_state.required_anim_state = 0_as;
  }

  const auto* cmd = getSkeleton()->getAnim()->animCommandCount == 0 ? nullptr : getSkeleton()->getAnim()->animCommands;
  for(uint16_t i = 0; i < getSkeleton()->getAnim()->animCommandCount; ++i)
  {
    BOOST_ASSERT(cmd != nullptr && cmd < &getWorld().getAnimCommands().back());
    const auto opcode = static_cast<AnimCommandOpcode>(*cmd);
    ++cmd;
    switch(opcode)
    {
    case AnimCommandOpcode::SetPosition: cmd += 3; break;
    case AnimCommandOpcode::StartFalling: cmd += 2; break;
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
        BOOST_LOG_TRIVIAL(debug) << "Anim effect: " << int(cmd[1]);
        getWorld().runEffect(cmd[1], this);
      }
      cmd += 2;
      break;
    default: break;
    }
  }

  applyMovement(false);
}

void ModelObject::applyMovement(const bool forLara)
{
  if(m_state.falling)
  {
    if(m_state.fallspeed >= core::AirResistanceSpeed * 1_frame)
    {
      m_state.fallspeed += core::HighSpeedGravity * 1_frame;
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
  else
  {
    m_state.speed = m_skeleton->calculateFloorSpeed();
  }

  move(util::pitch(m_state.speed * 1_frame, getMovementAngle())
       + core::TRVec{0_len, (m_state.falling ? m_state.fallspeed : 0_spd) * 1_frame, 0_len});

  applyTransform();

  m_skeleton->updatePose();
}

loader::file::BoundingBox ModelObject::getBoundingBox() const
{
  return m_skeleton->getBoundingBox();
}

bool ModelObject::isNear(const ModelObject& other, const core::Length& radius) const
{
  const auto bbox = getSkeleton()->getInterpolationInfo().getNearestFrame()->bbox.toBBox();
  const auto otherBBox = other.getSkeleton()->getInterpolationInfo().getNearestFrame()->bbox.toBBox();
  if(other.m_state.position.position.Y + otherBBox.minY >= m_state.position.position.Y + bbox.maxY
     || m_state.position.position.Y + bbox.minY >= other.m_state.position.position.Y + otherBBox.maxY)
  {
    return false;
  }

  const auto xz = util::pitch(other.m_state.position.position - m_state.position.position, -m_state.rotation.Y);
  return xz.X >= bbox.minX - radius && xz.X <= bbox.maxX + radius && xz.Z >= bbox.minZ - radius
         && xz.Z <= bbox.maxZ + radius;
}

bool ModelObject::isNear(const Particle& other, const core::Length& radius) const
{
  const auto frame = getSkeleton()->getInterpolationInfo().getNearestFrame();
  const auto bbox = frame->bbox.toBBox();
  if(other.pos.position.Y >= m_state.position.position.Y + bbox.maxY
     || m_state.position.position.Y + bbox.minY >= other.pos.position.Y)
  {
    return false;
  }

  const auto xz = util::pitch(other.pos.position - m_state.position.position, -m_state.rotation.Y);
  return xz.X >= bbox.minX - radius && xz.X <= bbox.maxX + radius && xz.Z >= bbox.minZ - radius
         && xz.Z <= bbox.maxZ + radius;
}

void ModelObject::enemyPush(CollisionInfo& collisionInfo, const bool enableSpaz, const bool withXZCollRadius)
{
  auto& lara = getWorld().getObjectManager().getLara();

  const auto enemyToLara = lara.m_state.position.position - m_state.position.position;
  auto bbox = m_skeleton->getInterpolationInfo().getNearestFrame()->bbox.toBBox();
  if(withXZCollRadius)
  {
    const auto r = collisionInfo.collisionRadius;
    bbox.minX -= r;
    bbox.maxX += r;
    bbox.minZ -= r;
    bbox.maxZ += r;
  }

  // determine which edge lara is closest to
  auto laraInBBox = util::pitch(enemyToLara, -m_state.rotation.Y);
  const auto distMinX = laraInBBox.X - bbox.minX;
  const auto distMaxX = bbox.maxX - laraInBBox.X;
  const auto distMinZ = laraInBBox.Z - bbox.minZ;
  const auto distMaxZ = bbox.maxZ - laraInBBox.Z;
  const auto closestEdge = std::min(std::min(distMinX, distMaxX), std::min(distMinZ, distMaxZ));
  if(closestEdge <= 0_len)
    return;

  // push lara out to the closest edge
  if(distMinX == closestEdge)
  {
    laraInBBox.X = bbox.minX;
  }
  else if(distMaxX == closestEdge)
  {
    laraInBBox.X = bbox.maxX;
  }
  else if(distMinZ == closestEdge)
  {
    laraInBBox.Z = bbox.minZ;
  }
  else
  {
    BOOST_ASSERT(distMaxZ == closestEdge);
    laraInBBox.Z = bbox.maxZ;
  }
  // update lara's position to where she was pushed
  lara.m_state.position.position = m_state.position.position + util::pitch(laraInBBox, m_state.rotation.Y);
  if(enableSpaz)
  {
    const auto midX = (bbox.minX + bbox.maxX) / 2;
    const auto midZ = (bbox.minZ + bbox.maxZ) / 2;
    const auto tmp = enemyToLara - util::pitch(core::TRVec{midX, 0_len, midZ}, m_state.rotation.Y);
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
  collisionInfo.badPositiveDistance = core::HeightLimit;
  collisionInfo.badNegativeDistance = -384_len;
  collisionInfo.badCeilingDistance = 0_len;
  const auto facingAngle = collisionInfo.facingAngle;
  collisionInfo.facingAngle = angleFromAtan(lara.m_state.position.position.X - collisionInfo.initialPosition.X,
                                            lara.m_state.position.position.Z - collisionInfo.initialPosition.Z);
  collisionInfo.initHeightInfo(lara.m_state.position.position, getWorld(), core::LaraWalkHeight);
  collisionInfo.facingAngle = facingAngle;
  if(collisionInfo.collisionType != CollisionInfo::AxisColl::None)
  {
    lara.m_state.position.position.X = collisionInfo.initialPosition.X;
    lara.m_state.position.position.Z = collisionInfo.initialPosition.Z;
  }
  else
  {
    collisionInfo.initialPosition = lara.m_state.position.position;
    lara.updateFloorHeight(-10_len);
  }
}

bool ModelObject::testBoneCollision(const ModelObject& other)
{
  m_state.touch_bits = 0;
  const auto boneSpheres
    = m_skeleton->getBoneCollisionSpheres(m_state, *m_skeleton->getInterpolationInfo().getNearestFrame(), nullptr);
  const auto laraSpheres = other.m_skeleton->getBoneCollisionSpheres(
    other.m_state, *other.m_skeleton->getInterpolationInfo().getNearestFrame(), nullptr);
  for(const auto& boneSphere : boneSpheres | boost::adaptors::indexed(0))
  {
    if(boneSphere.value().radius <= 0_len)
      continue;

    for(const auto& laraSphere : laraSpheres)
    {
      if(laraSphere.radius <= 0_len)
        continue;
      if(distance(laraSphere.getPosition(), boneSphere.value().getPosition())
         >= (boneSphere.value().radius + laraSphere.radius).get<float>())
        continue;

      m_state.touch_bits.set(boneSphere.index());
      break;
    }
  }

  return m_state.touch_bits.any();
}

gsl::not_null<std::shared_ptr<Particle>>
  ModelObject::emitParticle(const core::TRVec& localPosition,
                            const size_t boneIndex,
                            gsl::not_null<std::shared_ptr<Particle>> (*generate)(
                              world::World& world, const RoomBoundPosition&, const core::Speed&, const core::Angle&))
{
  BOOST_ASSERT(generate != nullptr);
  BOOST_ASSERT(boneIndex < m_skeleton->getBoneCount());

  const auto boneSpheres
    = m_skeleton->getBoneCollisionSpheres(m_state, *m_skeleton->getInterpolationInfo().getNearestFrame(), nullptr);
  BOOST_ASSERT(boneIndex < boneSpheres.size());

  auto roomPos = m_state.position;
  roomPos.position = core::TRVec{glm::vec3{translate(boneSpheres.at(boneIndex).m, localPosition.toRenderSystem())[3]}};
  auto particle = generate(getWorld(), roomPos, m_state.speed, m_state.rotation.Y);
  getWorld().getObjectManager().registerParticle(particle);

  return particle;
}

void ModelObject::updateLighting()
{
  m_lighting.update(core::Shade{core::Shade::type{-1}}, *m_state.position.room);
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
  auto result = std::make_shared<ModelObject>(&ser.context, RoomBoundPosition::create(ser["@position"]));
  result->serialize(ser);
  return result;
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
} // namespace engine::objects
