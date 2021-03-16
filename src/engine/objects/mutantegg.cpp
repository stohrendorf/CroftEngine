#include "mutantegg.h"

#include "engine/particle.h"
#include "engine/presenter.h"
#include "engine/world/rendermeshdata.h"
#include "laraobject.h"
#include "mutant.h"
#include "render/scene/mesh.h"
#include "serialization/objectreference.h"
#include "serialization/quantity.h"
#include "serialization/serialization.h"

namespace engine::objects
{
bool shatterModel(ModelObject& object, const std::bitset<32>& meshMask, const core::Length& damageRadius)
{
  object.getSkeleton()->updatePose();
  const bool isTorsoBoss = object.m_state.type == TR1ItemId::TorsoBoss;

  const auto modelSourceType
    = object.m_state.type == TR1ItemId::WalkingMutant1 || object.m_state.type == TR1ItemId::WalkingMutant2
        ? TR1ItemId::FlyingMutant
        : object.m_state.type;
  const auto& modelType = object.getWorld().findAnimatedModelForType(modelSourceType);
  Expects(modelType != nullptr);
  Expects(modelType->bones.size() == object.getSkeleton()->getBoneCount());
  BOOST_LOG_TRIVIAL(trace) << "Shatter model: " << modelType->bones.size() << " meshes";

  for(size_t i = 0; i < modelType->bones.size(); ++i)
  {
    if(!meshMask.test(i) || !object.getSkeleton()->isVisible(i))
    {
      BOOST_LOG_TRIVIAL(trace) << "Shatter model: mesh " << i << " skipped";
      continue;
    }

    object.getSkeleton()->setVisible(i, false);
    object.getSkeleton()->rebuildMesh();
    world::RenderMeshDataCompositor compositor;
    compositor.append(*modelType->bones[i].mesh);
    auto particle = std::make_shared<MeshShrapnelParticle>(
      core::RoomBoundPosition{object.m_state.position.room,
                              core::TRVec{object.getSkeleton()->getMeshPartTranslationWorld(i)}},
      object.getWorld(),
      compositor.toMesh(*object.getWorld().getPresenter().getMaterialManager(), false, {}),
      isTorsoBoss,
      damageRadius);
    particle->negSpriteFrameId = gsl::narrow<int16_t>((modelType->meshBaseIndex + i).index);
    setParent(particle, object.m_state.position.room->node);
    object.getWorld().getObjectManager().registerParticle(std::move(particle));

    BOOST_LOG_TRIVIAL(trace) << "Shatter model: mesh " << i << " converted";
  }

  for(size_t i = 0; i < object.getSkeleton()->getBoneCount(); ++i)
  {
    if(object.getSkeleton()->isVisible(i))
      return false;
  }

  return true;
}

MutantEgg::MutantEgg(const gsl::not_null<world::World*>& world,
                     const gsl::not_null<const world::Room*>& room,
                     loader::file::Item item,
                     const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
    : ModelObject{world, room, item, true, animatedModel}
{
  m_state.activationState = floordata::ActivationState(uint16_t(item.activationState & ~0x3e00u));

  switch((item.activationState & 0x3e00u) >> 9u)
  {
  case 1:
    item.type = TR1ItemId::WalkingMutant1;
    if(world->findAnimatedModelForType(TR1ItemId::FlyingMutant) != nullptr)
      m_childObject = std::make_shared<WalkingMutant>(
        world, room, item, world->findAnimatedModelForType(TR1ItemId::FlyingMutant).get());
    break;
  case 2:
    item.type = TR1ItemId::CentaurMutant;
    if(world->findAnimatedModelForType(item.type) != nullptr)
      m_childObject
        = std::make_shared<CentaurMutant>(world, room, item, world->findAnimatedModelForType(item.type).get());
    break;
  case 4:
    item.type = TR1ItemId::TorsoBoss;
    if(world->findAnimatedModelForType(item.type) != nullptr)
      m_childObject = std::make_shared<TorsoBoss>(world, room, item, world->findAnimatedModelForType(item.type).get());
    break;
  case 8:
    item.type = TR1ItemId::WalkingMutant2;
    if(world->findAnimatedModelForType(TR1ItemId::FlyingMutant) != nullptr)
      m_childObject = std::make_shared<WalkingMutant>(
        world, room, item, world->findAnimatedModelForType(TR1ItemId::FlyingMutant).get());
    break;
  default:
    item.type = TR1ItemId::FlyingMutant;
    if(world->findAnimatedModelForType(TR1ItemId::FlyingMutant) != nullptr)
      m_childObject = std::make_shared<FlyingMutant>(
        world, room, item, world->findAnimatedModelForType(TR1ItemId::FlyingMutant).get());
    break;
  }

  if(m_childObject == nullptr)
  {
    BOOST_LOG_TRIVIAL(warning) << "Mutant egg does not have an object to hatch";
  }
  else
  {
    getWorld().getObjectManager().registerObject(m_childObject);
  }

  for(size_t i = 0; i < getSkeleton()->getBoneCount(); ++i)
  {
    getSkeleton()->setVisible(i, (0xff0001ffu >> i) & 1u);
  }
  getSkeleton()->rebuildMesh();
}

void MutantEgg::update()
{
  if(m_state.goal_anim_state != 1_as)
  {
    if(m_state.activationState.isOneshot() || m_state.type == TR1ItemId::MutantEggBig
       || (getWorld().getObjectManager().getLara().m_state.position.position - m_state.position.position).absMax()
            < 4096_len)
    {
      BOOST_LOG_TRIVIAL(debug) << getSkeleton()->getName() << ": Hatching " << m_childObject->getNode()->getName();
      m_state.goal_anim_state = 1_as;
      m_state.collidable = false;
      for(size_t i = 0; i < getSkeleton()->getBoneCount(); ++i)
        getSkeleton()->setVisible(i, i < 24);
      getSkeleton()->rebuildMesh();

      shatterModel(*this, 0xfffe00, 0_len);

      if(m_childObject != nullptr)
      {
        m_childObject->m_state.position = m_state.position;
        m_childObject->m_state.rotation.Y = m_state.rotation.Y;
        addChild(m_state.position.room->node, m_childObject->getNode());

        m_childObject->applyTransform();

        m_childObject->m_state.touch_bits.reset();
        m_childObject->m_state.initCreatureInfo(getWorld());
        m_childObject->activate();
        m_childObject->m_state.triggerState = TriggerState::Active;
      }
    }
  }

  ModelObject::update();
}

void MutantEgg::collide(CollisionInfo& info)
{
  if(!isNear(getWorld().getObjectManager().getLara(), info.collisionRadius))
    return;

  if(!testBoneCollision(getWorld().getObjectManager().getLara()))
    return;

  if(!info.policies.is_set(CollisionInfo::PolicyFlags::EnableBaddiePush))
    return;

  enemyPush(info, false, true);
}

void MutantEgg::serialize(const serialization::Serializer<world::World>& ser)
{
  ModelObject::serialize(ser);
  ser(S_NV("childObject", serialization::ObjectReference{m_childObject}));
}
} // namespace engine::objects
