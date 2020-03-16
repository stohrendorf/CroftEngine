#include "mutantegg.h"

#include "engine/particle.h"
#include "laraobject.h"
#include "mutant.h"
#include "serialization/objectreference.h"
#include "serialization/quantity.h"

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
  const auto& modelType = object.getEngine().findAnimatedModelForType(modelSourceType);
  Expects(modelType != nullptr);
  const auto& models = modelType->renderMeshes;
  Expects(models.size() == object.getSkeleton()->getChildren().size());
  BOOST_LOG_TRIVIAL(trace) << "Shatter model: " << modelType->renderMeshes.size() << " meshes";

  for(size_t i = 0; i < modelType->renderMeshes.size(); ++i)
  {
    if(!meshMask.test(i) || !object.getSkeleton()->getChild(i)->isVisible())
    {
      BOOST_LOG_TRIVIAL(trace) << "Shatter model: mesh " << i << " skipped";
      continue;
    }

    object.getSkeleton()->getChild(i)->setVisible(false);
    auto particle = std::make_shared<MeshShrapnelParticle>(
      core::RoomBoundPosition{object.m_state.position.room,
                              core::TRVec{object.getSkeleton()->getChild(i)->getTranslationWorld()}},
      object.getEngine(),
      modelType->renderMeshes[i],
      isTorsoBoss,
      damageRadius);
    particle->negSpriteFrameId = gsl::narrow<int16_t>(modelType->mesh_base_index + i);
    setParent(particle, object.m_state.position.room->node);
    object.getEngine().getParticles().emplace_back(std::move(particle));

    BOOST_LOG_TRIVIAL(trace) << "Shatter model: mesh " << i << " converted";
  }

  for(const auto& child : object.getSkeleton()->getChildren())
  {
    if(child->isVisible())
      return false;
  }

  return true;
}

MutantEgg::MutantEgg(const gsl::not_null<Engine*>& engine,
                     const gsl::not_null<const loader::file::Room*>& room,
                     loader::file::Item item,
                     const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
    : ModelObject{engine, room, item, true, animatedModel}
{
  m_state.activationState = floordata::ActivationState(uint16_t(item.activationState & ~0x3e00u));

  switch((item.activationState & 0x3e00u) >> 9u)
  {
  case 1:
    item.type = TR1ItemId::WalkingMutant1;
    if(engine->findAnimatedModelForType(TR1ItemId::FlyingMutant) != nullptr)
      m_childObject = std::make_shared<WalkingMutant>(
        engine, room, item, engine->findAnimatedModelForType(TR1ItemId::FlyingMutant).get());
    break;
  case 2:
    item.type = TR1ItemId::CentaurMutant;
    if(engine->findAnimatedModelForType(item.type) != nullptr)
      m_childObject
        = std::make_shared<CentaurMutant>(engine, room, item, engine->findAnimatedModelForType(item.type).get());
    break;
  case 4:
    item.type = TR1ItemId::TorsoBoss;
    if(engine->findAnimatedModelForType(item.type) != nullptr)
      m_childObject
        = std::make_shared<TorsoBoss>(engine, room, item, engine->findAnimatedModelForType(item.type).get());
    break;
  case 8:
    item.type = TR1ItemId::WalkingMutant2;
    if(engine->findAnimatedModelForType(TR1ItemId::FlyingMutant) != nullptr)
      m_childObject = std::make_shared<WalkingMutant>(
        engine, room, item, engine->findAnimatedModelForType(TR1ItemId::FlyingMutant).get());
    break;
  default:
    item.type = TR1ItemId::FlyingMutant;
    if(engine->findAnimatedModelForType(TR1ItemId::FlyingMutant) != nullptr)
      m_childObject = std::make_shared<FlyingMutant>(
        engine, room, item, engine->findAnimatedModelForType(TR1ItemId::FlyingMutant).get());
    break;
  }

  if(m_childObject == nullptr)
  {
    BOOST_LOG_TRIVIAL(warning) << "Mutant egg does not have an object to hatch";
  }
  else
  {
    getEngine().registerObject(m_childObject);
  }

  for(size_t i = 0; i < getSkeleton()->getChildren().size(); ++i)
  {
    getSkeleton()->getChild(i)->setVisible((0xff0001ffu >> i) & 1u);
  }
}

void MutantEgg::update()
{
  if(m_state.goal_anim_state != 1_as)
  {
    if(m_state.activationState.isOneshot() || m_state.type == TR1ItemId::MutantEggBig
       || (getEngine().getLara().m_state.position.position - m_state.position.position).absMax() < 4096_len)
    {
      BOOST_LOG_TRIVIAL(debug) << getSkeleton()->getId() << ": Hatching " << m_childObject->getNode()->getId();
      m_state.goal_anim_state = 1_as;
      m_state.collidable = false;
      for(size_t i = 0; i < getSkeleton()->getChildren().size(); ++i)
        getSkeleton()->getChild(i)->setVisible(i < 24);

      shatterModel(*this, 0xfffe00, 0_len);

      if(m_childObject != nullptr)
      {
        m_childObject->m_state.position = m_state.position;
        m_childObject->m_state.rotation.Y = m_state.rotation.Y;
        addChild(m_state.position.room->node, m_childObject->getNode());

        m_childObject->applyTransform();
        m_childObject->updateLighting();

        m_childObject->m_state.touch_bits.reset();
        m_childObject->m_state.initCreatureInfo(getEngine());
        m_childObject->activate();
        m_childObject->m_state.triggerState = TriggerState::Active;
      }
    }
  }

  ModelObject::update();
}

void MutantEgg::collide(CollisionInfo& info)
{
  if(!isNear(getEngine().getLara(), info.collisionRadius))
    return;

  if(!testBoneCollision(getEngine().getLara()))
    return;

  if(!info.policyFlags.is_set(CollisionInfo::PolicyFlags::EnableBaddiePush))
    return;

  enemyPush(info, false, true);
}

void MutantEgg::serialize(const serialization::Serializer& ser)
{
  ModelObject::serialize(ser);
  ser(S_NV("childObject", serialization::ObjectReference{m_childObject}));
}
} // namespace engine::objects
