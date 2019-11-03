#include "itemnode.h"

#include "engine/laranode.h"
#include "engine/particle.h"
#include "engine/script/reflection.h"
#include "render/scene/sprite.h"

#include <boost/range/adaptor/indexed.hpp>

namespace engine
{
namespace items
{
namespace
{
const char* toString(const TriggerState s)
{
  switch(s)
  {
  case TriggerState::Inactive: return "Inactive";
  case TriggerState::Active: return "Active";
  case TriggerState::Deactivated: return "Deactivated";
  case TriggerState::Invisible: return "Invisible";
  default: BOOST_THROW_EXCEPTION(std::domain_error("Invalid TriggerState"));
  }
}

TriggerState parseTriggerState(const std::string& s)
{
  if(s == "Inactive")
    return TriggerState::Inactive;
  if(s == "Active")
    return TriggerState::Active;
  if(s == "Deactivated")
    return TriggerState::Deactivated;
  if(s == "Invisible")
    return TriggerState::Invisible;
  BOOST_THROW_EXCEPTION(std::domain_error("Invalid TriggerState"));
}
} // namespace

void ItemNode::applyTransform()
{
  const glm::vec3 tr = m_state.position.position.toRenderSystem() - m_state.position.room->position.toRenderSystem();
  getNode()->setLocalMatrix(translate(glm::mat4{1.0f}, tr) * m_state.rotation.toMatrix());
}

ItemNode::ItemNode(const gsl::not_null<Engine*>& engine,
                   const gsl::not_null<const loader::file::Room*>& room,
                   const loader::file::Item& item,
                   const bool hasUpdateFunction)
    : m_engine{engine}
    , m_state{&engine->getSoundEngine(), room, item.type}
    , m_hasUpdateFunction{hasUpdateFunction}
{
  BOOST_ASSERT(room->isInnerPositionXZ(item.position));

  m_state.loadObjectInfo(engine->getScriptEngine());

  m_state.position.position = item.position;
  m_state.rotation.Y = item.rotation;
  m_state.shade = item.darkness;
  m_state.activationState = floordata::ActivationState(item.activationState);
  m_state.timer = m_state.activationState.getTimeout();

  m_state.floor = room->getSectorByAbsolutePosition(item.position)->floorHeight;

  if(m_state.activationState.isOneshot())
  {
    m_state.activationState.setOneshot(false);
    m_state.triggerState = TriggerState::Invisible;
  }

  if(m_state.activationState.isFullyActivated())
  {
    m_state.activationState.fullyDeactivate();
    m_state.activationState.setInverted(true);
    activate();
    m_state.triggerState = TriggerState::Active;
  }
}

void ItemNode::setCurrentRoom(const gsl::not_null<const loader::file::Room*>& newRoom)
{
  if(newRoom == m_state.position.room)
  {
    return;
  }

  addChild(newRoom->node, getNode());

  m_state.position.room = newRoom;
  applyTransform();
}

ModelItemNode::ModelItemNode(const gsl::not_null<Engine*>& engine,
                             const gsl::not_null<const loader::file::Room*>& room,
                             const loader::file::Item& item,
                             const bool hasUpdateFunction,
                             const loader::file::SkeletalModelType& animatedModel)
    : ItemNode{engine, room, item, hasUpdateFunction}
    , m_skeleton{std::make_shared<SkeletalModelNode>(
        std::string("skeleton(type:") + toString(item.type.get_as<TR1ItemId>()) + ")", engine, animatedModel)}
{
  m_skeleton->setAnimation(m_state, animatedModel.animations, animatedModel.animations->firstFrame);

  for(gsl::index boneIndex = 0; boneIndex < animatedModel.meshes.size(); ++boneIndex)
  {
    auto node = std::make_shared<render::scene::Node>(m_skeleton->getId() + "/bone:" + std::to_string(boneIndex));
    node->setDrawable(animatedModel.models[boneIndex].get());
    addChild(m_skeleton, node);
  }

  BOOST_ASSERT(m_skeleton->getChildren().size() == gsl::narrow<size_t>(animatedModel.meshes.size()));

  m_skeleton->updatePose(m_state);
  m_lighting.bind(*m_skeleton);
}

void ModelItemNode::update()
{
  const auto endOfAnim = m_skeleton->advanceFrame(m_state);

  m_state.is_hit = false;
  m_state.touch_bits = 0;

  if(endOfAnim)
  {
    const auto* cmd = m_state.anim->animCommandCount == 0
                        ? nullptr
                        : &getEngine().getAnimCommands().at(m_state.anim->animCommandIndex);
    for(uint16_t i = 0; i < m_state.anim->animCommandCount; ++i)
    {
      BOOST_ASSERT(cmd < &getEngine().getAnimCommands().back());
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
      case AnimCommandOpcode::PlaySound: cmd += 2; break;
      case AnimCommandOpcode::PlayEffect: cmd += 2; break;
      case AnimCommandOpcode::Kill: m_state.triggerState = TriggerState::Deactivated; break;
      default: break;
      }
    }

    m_skeleton->setAnimation(m_state, m_state.anim->nextAnimation, m_state.anim->nextFrame);
    m_state.goal_anim_state = m_state.current_anim_state;
    if(m_state.current_anim_state == m_state.required_anim_state)
      m_state.required_anim_state = 0_as;
  }

  const auto* cmd
    = m_state.anim->animCommandCount == 0 ? nullptr : &getEngine().getAnimCommands().at(m_state.anim->animCommandIndex);
  for(uint16_t i = 0; i < m_state.anim->animCommandCount; ++i)
  {
    BOOST_ASSERT(cmd < &getEngine().getAnimCommands().back());
    const auto opcode = static_cast<AnimCommandOpcode>(*cmd);
    ++cmd;
    switch(opcode)
    {
    case AnimCommandOpcode::SetPosition: cmd += 3; break;
    case AnimCommandOpcode::StartFalling: cmd += 2; break;
    case AnimCommandOpcode::PlaySound:
      if(m_state.frame_number.get() == cmd[0])
      {
        playSoundEffect(static_cast<TR1SoundId>(cmd[1]));
      }
      cmd += 2;
      break;
    case AnimCommandOpcode::PlayEffect:
      if(m_state.frame_number.get() == cmd[0])
      {
        BOOST_LOG_TRIVIAL(debug) << "Anim effect: " << int(cmd[1]);
        getEngine().runEffect(cmd[1], this);
      }
      cmd += 2;
      break;
    default: break;
    }
  }

  applyMovement(false);
}

void ItemNode::activate()
{
  if(!m_hasUpdateFunction)
  {
    m_state.triggerState = TriggerState::Inactive;
    return;
  }

  m_isActive = true;
}

void ItemNode::deactivate()
{
  m_isActive = false;
}

std::shared_ptr<audio::SourceHandle> ItemNode::playSoundEffect(const core::SoundId id)
{
  return getEngine().getAudioEngine().playSound(id, &m_state);
}

bool ItemNode::triggerKey()
{
  if(getEngine().getLara().getHandStatus() != HandStatus::None)
  {
    return false;
  }

  if(m_state.triggerState != TriggerState::Active)
  {
    return false;
  }

  m_state.triggerState = TriggerState::Deactivated;
  return true;
}

void ItemNode::kill()
{
  if(this == getEngine().getLara().target.get())
  {
    getEngine().getLara().target.reset();
  }
  getEngine().scheduleDeletion(this);
  m_state.activationState.setLocked(true);
}

bool ItemNode::triggerPickUp()
{
  if(m_state.triggerState != TriggerState::Invisible)
  {
    return false;
  }

  m_state.triggerState = TriggerState::Deactivated;
  return true;
}

YAML::Node ItemNode::save() const
{
  YAML::Node n;
  n["state"] = m_state.save(*m_engine);
  n["active"] = m_isActive;

  for(const auto& child : getNode()->getChildren())
    if(auto idx = m_engine->indexOfModel(child->getDrawable()))
      n["meshes"].push_back(*idx);
    else
      n["meshes"].push_back(YAML::Node());

  if(n["meshes"].IsDefined())
    n["meshes"].SetStyle(YAML::EmitterStyle::Flow);

  return n;
}

void ItemNode::load(const YAML::Node& n)
{
  m_state.load(n["state"], *m_engine);
  m_isActive = n["active"].as<bool>();

  if(getNode()->getChildren().empty())
  {
    Expects(!n["meshes"].IsDefined());
  }
  else
  {
    Expects(!n["meshes"].IsDefined() || n["meshes"].size() <= getNode()->getChildren().size());
  }

  if(n["meshes"].IsDefined())
  {
    for(size_t i = 0; i < n["meshes"].size(); ++i)
    {
      auto m = n["meshes"][i];
      if(!m.IsNull())
        getNode()->getChildren()[i]->setDrawable(getEngine().getModel(m.as<size_t>()).get());
      else
        getNode()->getChildren()[i]->setDrawable(nullptr);
    }
  }

  addChild(m_state.position.room->node, getNode());
  applyTransform();
}

bool InteractionLimits::canInteract(const ItemState& item, const ItemState& lara) const
{
  const auto angle = lara.rotation - item.rotation;
  if(angle.X < minAngle.X || angle.X > maxAngle.X || angle.Y < minAngle.Y || angle.Y > maxAngle.Y
     || angle.Z < minAngle.Z || angle.Z > maxAngle.Z)
  {
    return false;
  }

  const auto offs = lara.position.position - item.position.position;
  const auto dist = glm::vec4{offs.toRenderSystem(), 0.0f} * item.rotation.toMatrix();
  return distance.contains(core::TRVec{glm::vec3{dist}});
}

void ModelItemNode::applyMovement(const bool forLara)
{
  if(m_state.falling)
  {
    if(m_state.fallspeed >= 128_spd)
    {
      m_state.fallspeed += 1_spd;
    }
    else
    {
      m_state.fallspeed += 6_spd;
    }

    if(forLara)
    {
      // we only add acceleration here
      m_state.speed = m_state.speed + m_skeleton->calculateFloorSpeed(m_state, 0_frame)
                      - m_skeleton->calculateFloorSpeed(m_state, -1_frame);
    }
  }
  else
  {
    m_state.speed = m_skeleton->calculateFloorSpeed(m_state);
  }

  move((util::pitch(m_state.speed * 1_frame, getMovementAngle())
        + core::TRVec{0_len, (m_state.falling ? m_state.fallspeed : 0_spd) * 1_frame, 0_len})
         .toRenderSystem());

  applyTransform();

  m_skeleton->updatePose(m_state);
  updateLighting();
}

loader::file::BoundingBox ModelItemNode::getBoundingBox() const
{
  return m_skeleton->getBoundingBox(m_state);
}

SpriteItemNode::SpriteItemNode(const gsl::not_null<Engine*>& engine,
                               const std::string& name,
                               const gsl::not_null<const loader::file::Room*>& room,
                               const loader::file::Item& item,
                               const bool hasUpdateFunction,
                               const loader::file::Sprite& sprite,
                               const gsl::not_null<std::shared_ptr<render::scene::Material>>& material)
    : ItemNode{engine, room, item, hasUpdateFunction}
{
  const auto model = std::make_shared<render::scene::Sprite>(
    sprite.x0, -sprite.y0, sprite.x1, -sprite.y1, sprite.t0, sprite.t1, material, render::scene::Sprite::Axis::Y);

  m_node = std::make_shared<render::scene::Node>(name);
  m_node->setDrawable(model);
  m_node->addUniformSetter("u_diffuseTexture",
                           [texture = sprite.texture](const render::scene::Node& /*node*/,
                                                      render::gl::ProgramUniform& uniform) { uniform.set(*texture); });
  m_node->addUniformSetter(
    "u_lightAmbient",
    [brightness = item.getBrightness()](const render::scene::Node& /*node*/, render::gl::ProgramUniform& uniform) {
      uniform.set(brightness);
    });

  addChild(room->node, m_node);

  applyTransform();
}

bool ModelItemNode::isNear(const ModelItemNode& other, const core::Length& radius) const
{
  const auto aFrame = getSkeleton()->getInterpolationInfo(m_state).getNearestFrame();
  const auto aBBox = aFrame->bbox.toBBox();
  const auto bFrame = other.getSkeleton()->getInterpolationInfo(other.m_state).getNearestFrame();
  const auto bBBox = bFrame->bbox.toBBox();
  if(other.m_state.position.position.Y + bBBox.minY >= m_state.position.position.Y + aBBox.maxY
     || m_state.position.position.Y + aBBox.minY >= other.m_state.position.position.Y + bBBox.maxY)
  {
    return false;
  }

  const auto xz = util::pitch(other.m_state.position.position - m_state.position.position, -m_state.rotation.Y);
  return xz.X >= aBBox.minX - radius && xz.X <= aBBox.maxX + radius && xz.Z >= aBBox.minZ - radius
         && xz.Z <= aBBox.maxZ + radius;
}

bool ModelItemNode::isNear(const Particle& other, const core::Length& radius) const
{
  const auto frame = getSkeleton()->getInterpolationInfo(m_state).getNearestFrame();
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

void ModelItemNode::enemyPush(LaraNode& lara,
                              CollisionInfo& collisionInfo,
                              const bool enableSpaz,
                              const bool withXZCollRadius)
{
  const auto laraPosWorld = lara.m_state.position.position - m_state.position.position;
  auto laraPosLocal = util::pitch(laraPosWorld, -m_state.rotation.Y);
  const auto keyFrame = m_skeleton->getInterpolationInfo(m_state).getNearestFrame();
  auto itemBBox = keyFrame->bbox.toBBox();
  if(withXZCollRadius)
  {
    const auto r = collisionInfo.collisionRadius;
    itemBBox.minX -= r;
    itemBBox.maxX += r;
    itemBBox.minZ -= r;
    itemBBox.maxZ += r;
  }
  if(laraPosLocal.X < itemBBox.minX || laraPosLocal.X > itemBBox.maxX || laraPosLocal.Z < itemBBox.minZ
     || laraPosLocal.Z > itemBBox.maxZ)
    return;

  const auto left = laraPosLocal.X - itemBBox.minX;
  const auto right = itemBBox.maxX - laraPosLocal.X;
  const auto back = laraPosLocal.Z - itemBBox.minZ;
  const auto front = itemBBox.maxZ - laraPosLocal.Z;

  if(left <= right && left <= front && left <= back)
  {
    laraPosLocal.X = itemBBox.minX;
  }
  else if(right <= left && right <= front && right <= back)
  {
    laraPosLocal.X = itemBBox.maxX;
  }
  else if(front <= left && front <= right && front <= back)
  {
    laraPosLocal.Z = itemBBox.maxZ;
  }
  else
  {
    laraPosLocal.Z = itemBBox.minZ;
  }
  // update lara's position to where she was pushed
  lara.m_state.position.position = m_state.position.position + util::pitch(laraPosLocal, m_state.rotation.Y);
  if(enableSpaz)
  {
    const auto midX = (keyFrame->bbox.toBBox().minX + keyFrame->bbox.toBBox().maxX) / 2;
    const auto midZ = (keyFrame->bbox.toBBox().minZ + keyFrame->bbox.toBBox().maxZ) / 2;
    const auto tmp = laraPosWorld - util::pitch(core::TRVec{midX, 0_len, midZ}, m_state.rotation.Y);
    const auto a = angleFromAtan(tmp.X, tmp.Z) - 180_deg;
    getEngine().getLara().hit_direction = core::axisFromAngle(lara.m_state.rotation.Y - a, 45_deg).get();
    if(getEngine().getLara().hit_frame == 0_frame)
    {
      lara.playSoundEffect(TR1SoundId::LaraOof);
    }
    getEngine().getLara().hit_frame += 1_frame;
    if(getEngine().getLara().hit_frame > 34_frame)
    {
      getEngine().getLara().hit_frame = 34_frame;
    }
  }
  collisionInfo.badPositiveDistance = core::HeightLimit;
  collisionInfo.badNegativeDistance = -384_len;
  collisionInfo.badCeilingDistance = 0_len;
  const auto facingAngle = collisionInfo.facingAngle;
  collisionInfo.facingAngle = angleFromAtan(lara.m_state.position.position.X - collisionInfo.oldPosition.X,
                                            lara.m_state.position.position.Z - collisionInfo.oldPosition.Z);
  collisionInfo.initHeightInfo(lara.m_state.position.position, getEngine(), core::LaraWalkHeight);
  collisionInfo.facingAngle = facingAngle;
  if(collisionInfo.collisionType != CollisionInfo::AxisColl::None)
  {
    lara.m_state.position.position.X = collisionInfo.oldPosition.X;
    lara.m_state.position.position.Z = collisionInfo.oldPosition.Z;
  }
  else
  {
    collisionInfo.oldPosition = lara.m_state.position.position;
    lara.updateFloorHeight(-10_len);
  }
}

bool ModelItemNode::testBoneCollision(const ModelItemNode& other)
{
  m_state.touch_bits = 0;
  const auto itemSpheres = m_skeleton->getBoneCollisionSpheres(
    m_state, *m_skeleton->getInterpolationInfo(m_state).getNearestFrame(), nullptr);
  const auto laraSpheres = other.m_skeleton->getBoneCollisionSpheres(
    other.m_state, *other.m_skeleton->getInterpolationInfo(other.m_state).getNearestFrame(), nullptr);
  for(const auto& itemSphere : itemSpheres | boost::adaptors::indexed(0))
  {
    if(itemSphere.value().radius <= 0_len)
      continue;

    for(const auto& laraSphere : laraSpheres)
    {
      if(laraSphere.radius <= 0_len)
        continue;
      if(glm::distance(laraSphere.getPosition(), itemSphere.value().getPosition())
         >= (itemSphere.value().radius + laraSphere.radius).get_as<float>())
        continue;

      m_state.touch_bits.set(itemSphere.index());
      break;
    }
  }

  return m_state.touch_bits != 0;
}

gsl::not_null<std::shared_ptr<Particle>>
  ModelItemNode::emitParticle(const core::TRVec& localPosition,
                              const size_t boneIndex,
                              gsl::not_null<std::shared_ptr<Particle>> (*generate)(
                                Engine& engine, const core::RoomBoundPosition&, const core::Speed&, const core::Angle&))
{
  BOOST_ASSERT(generate != nullptr);
  BOOST_ASSERT(boneIndex < m_skeleton->getChildren().size());

  const auto itemSpheres = m_skeleton->getBoneCollisionSpheres(
    m_state, *m_skeleton->getInterpolationInfo(m_state).getNearestFrame(), nullptr);
  BOOST_ASSERT(boneIndex < itemSpheres.size());

  auto roomPos = m_state.position;
  roomPos.position = core::TRVec{glm::vec3{translate(itemSpheres.at(boneIndex).m, localPosition.toRenderSystem())[3]}};
  auto particle = generate(getEngine(), roomPos, m_state.speed, m_state.rotation.Y);
  getEngine().getParticles().push_back(particle);

  return particle;
}

void ModelItemNode::load(const YAML::Node& n)
{
  ItemNode::load(n);

  m_skeleton->load(n["skeleton"]);
  m_skeleton->updatePose(m_state);
}

YAML::Node ModelItemNode::save() const
{
  auto n = ItemNode::save();
  n["skeleton"] = m_skeleton->save();
  return n;
}

bool ItemState::stalkBox(const Engine& engine, const loader::file::Box& targetBox) const
{
  const auto laraToBoxDistX = (targetBox.xmin + targetBox.xmax) / 2 - engine.getLara().m_state.position.position.X;
  const auto laraToBoxDistZ = (targetBox.zmin + targetBox.zmax) / 2 - engine.getLara().m_state.position.position.Z;

  if(abs(laraToBoxDistX) > 3 * core::SectorSize || abs(laraToBoxDistZ) > 3 * core::SectorSize)
  {
    return false;
  }

  const auto laraAxisBack = *axisFromAngle(engine.getLara().m_state.rotation.Y + 180_deg, 45_deg);
  core::Axis laraToBoxAxis;
  if(laraToBoxDistZ > 0_len)
  {
    if(laraToBoxDistX > 0_len)
    {
      laraToBoxAxis = core::Axis::PosX;
    }
    else
    {
      laraToBoxAxis = core::Axis::NegZ;
    }
  }
  else if(laraToBoxDistX > 0_len)
  {
    // Z <= 0, X > 0
    laraToBoxAxis = core::Axis::NegX;
  }
  else
  {
    // Z <= 0, X <= 0
    laraToBoxAxis = core::Axis::PosZ;
  }

  if(laraAxisBack == laraToBoxAxis)
  {
    return false;
  }

  core::Axis itemToLaraAxis;
  if(position.position.Z <= engine.getLara().m_state.position.position.Z)
  {
    if(position.position.X <= engine.getLara().m_state.position.position.X)
    {
      itemToLaraAxis = core::Axis::PosZ;
    }
    else
    {
      itemToLaraAxis = core::Axis::NegX;
    }
  }
  else
  {
    if(position.position.X > engine.getLara().m_state.position.position.X)
    {
      itemToLaraAxis = core::Axis::PosX;
    }
    else
    {
      itemToLaraAxis = core::Axis::NegZ;
    }
  }

  if(laraAxisBack != itemToLaraAxis)
  {
    return true;
  }

  switch(laraAxisBack)
  {
  case core::Axis::PosZ: return laraToBoxAxis == core::Axis::NegZ;
  case core::Axis::PosX: return laraToBoxAxis == core::Axis::NegX;
  case core::Axis::NegZ: return laraToBoxAxis == core::Axis::PosZ;
  case core::Axis::NegX: return laraToBoxAxis == core::Axis::PosX;
  }

  BOOST_THROW_EXCEPTION(std::runtime_error("Unreachable code reached"));
}

bool ItemState::isInsideZoneButNotInBox(const Engine& engine,
                                        const int16_t zoneId,
                                        const loader::file::Box& targetBox) const
{
  Expects(creatureInfo != nullptr);

  const auto zoneRef = loader::file::Box::getZoneRef(
    engine.roomsAreSwapped(), creatureInfo->pathFinder.fly, creatureInfo->pathFinder.step);

  if(zoneId != targetBox.*zoneRef)
  {
    return false;
  }

  if(!creatureInfo->pathFinder.canVisit(targetBox))
  {
    return false;
  }

  return position.position.Z <= targetBox.zmin || position.position.Z >= targetBox.zmax
         || position.position.X <= targetBox.xmin || position.position.X >= targetBox.xmax;
}

bool ItemState::inSameQuadrantAsBoxRelativeToLara(const Engine& engine, const loader::file::Box& targetBox) const
{
  const auto laraToBoxX = (targetBox.xmin + targetBox.xmax) / 2 - engine.getLara().m_state.position.position.X;
  const auto laraToBoxZ = (targetBox.zmin + targetBox.zmax) / 2 - engine.getLara().m_state.position.position.Z;
  if(abs(laraToBoxX) < 5 * core::SectorSize && abs(laraToBoxZ) < 5 * core::SectorSize)
    return false;

  const auto laraToNpcX = position.position.X - engine.getLara().m_state.position.position.X;
  const auto laraToNpcZ = position.position.Z - engine.getLara().m_state.position.position.Z;
  return ((laraToNpcZ > 0_len) == (laraToBoxZ > 0_len)) || ((laraToNpcX > 0_len) == (laraToBoxX > 0_len));
}

void ItemState::initCreatureInfo(const Engine& engine)
{
  if(creatureInfo != nullptr)
    return;

  creatureInfo = std::make_shared<ai::CreatureInfo>(engine, type);
  collectZoneBoxes(engine);
}

void ItemState::collectZoneBoxes(const Engine& engine)
{
  const auto zoneRef1
    = loader::file::Box::getZoneRef(false, creatureInfo->pathFinder.fly, creatureInfo->pathFinder.step);
  const auto zoneRef2
    = loader::file::Box::getZoneRef(true, creatureInfo->pathFinder.fly, creatureInfo->pathFinder.step);

  box = position.room->getInnerSectorByAbsolutePosition(position.position)->box;
  const auto zoneData1 = box->*zoneRef1;
  const auto zoneData2 = box->*zoneRef2;
  creatureInfo->pathFinder.boxes.clear();
  for(const auto& levelBox : engine.getBoxes())
  {
    if(levelBox.*zoneRef1 == zoneData1 || levelBox.*zoneRef2 == zoneData2)
    {
      creatureInfo->pathFinder.boxes.emplace_back(&levelBox);
    }
  }
}

YAML::Node ItemState::save(const Engine& engine) const
{
  YAML::Node n;
  n["type"] = type;
  n["position"] = position.position.save();
  n["position"]["room"] = std::distance(&engine.getRooms()[0], position.room.get());
  n["rotation"] = rotation.save();
  n["speed"] = speed;
  n["fallSpeed"] = fallspeed;
  n["state"] = current_anim_state;
  n["goal"] = goal_anim_state;
  n["required"] = required_anim_state;
  if(anim != nullptr)
    n["id"] = std::distance(&engine.getAnimations()[0], anim);
  n["frame"] = frame_number;
  n["health"] = health;
  n["triggerState"] = toString(triggerState);
  n["timer"] = timer;
  n["activationState"] = activationState.save();
  n["floor"] = floor;
  n["touchBits"] = touch_bits.to_ulong();
  if(box != nullptr)
    n["box"] = std::distance(&engine.getBoxes()[0], box);
  n["shade"] = shade;

  n["falling"] = falling;
  n["isHit"] = is_hit;
  n["collidable"] = collidable;
  n["alreadyLookedAt"] = already_looked_at;

  if(creatureInfo != nullptr)
    n["creatureInfo"] = creatureInfo->save(engine);

  return n;
}

void ItemState::load(const YAML::Node& n, const Engine& engine)
{
  if(core::TypeId{n["type"].as<core::TypeId::type>()} != type)
    BOOST_THROW_EXCEPTION(std::domain_error("Item state has wrong type"));

  position.position.load(n["position"]);
  position.room = &engine.getRooms().at(n["position"]["room"].as<size_t>());
  rotation.load(n["rotation"]);
  speed = n["speed"].as<core::Speed>();
  fallspeed = n["fallSpeed"].as<core::Speed>();
  current_anim_state = n["state"].as<core::AnimStateId>();
  goal_anim_state = n["goal"].as<core::AnimStateId>();
  required_anim_state = n["required"].as<core::AnimStateId>();
  if(!n["id"].IsDefined())
    anim = nullptr;
  else
    anim = &engine.getAnimations().at(n["id"].as<size_t>());
  frame_number = n["frame"].as<core::Frame>();
  health = n["health"].as<core::Health>();
  triggerState = parseTriggerState(n["triggerState"].as<std::string>());
  timer = n["timer"].as<core::Frame>();
  activationState.load(n["activationState"]);

  floor = n["floor"].as<core::Length>();
  touch_bits = n["touchBits"].as<uint32_t>();
  if(!n["box"].IsDefined())
    box = nullptr;
  else
    box = &engine.getBoxes().at(n["box"].as<size_t>());
  shade = n["shade"].as<int16_t>();

  falling = n["falling"].as<bool>();
  is_hit = n["isHit"].as<bool>();
  collidable = n["collidable"].as<bool>();
  already_looked_at = n["alreadyLookedAt"].as<bool>();

  if(!n["creatureInfo"].IsDefined())
  {
    creatureInfo = nullptr;
  }
  else
  {
    creatureInfo = std::make_shared<ai::CreatureInfo>(engine, type);
    creatureInfo->load(n["creatureInfo"], engine);
  }
}

glm::vec3 ItemState::getPosition() const
{
  return position.position.toRenderSystem();
}
void ItemState::loadObjectInfo(const sol::state& engine)
{
  health = core::Health{engine["getObjectInfo"].call<script::ObjectInfo>(type.get()).hit_points};
}

ItemState::~ItemState() = default;

void ItemNode::playShotMissed(const core::RoomBoundPosition& pos)
{
  const auto particle = std::make_shared<RicochetParticle>(pos, getEngine());
  setParent(particle, m_state.position.room->node);
  getEngine().getParticles().emplace_back(particle);
  getEngine().getAudioEngine().playSound(TR1SoundId::Ricochet, particle.get());
}

boost::optional<core::Length> ItemNode::getWaterSurfaceHeight() const
{
  return m_state.position.room->getWaterSurfaceHeight(m_state.position);
}

void ItemNode::updateLighting()
{
  auto tmp = m_state.position;
  tmp.position += getBoundingBox().getCenter();
  m_lighting.updateDynamic(m_state.shade, tmp, m_engine->getRooms());
}
} // namespace items
} // namespace engine
