#include "mutantegg.h"

#include "engine/laranode.h"
#include "engine/particle.h"
#include "mutant.h"

namespace engine
{
namespace items
{
bool shatterModel(ModelItemNode& item, const std::bitset<32>& meshMask, const core::Length& damageRadius)
{
  item.getSkeleton()->updatePose(item.m_state);
  const bool isTorsoBoss = item.m_state.type == TR1ItemId::TorsoBoss;

  const auto modelSourceType
    = item.m_state.type == TR1ItemId::WalkingMutant1 || item.m_state.type == TR1ItemId::WalkingMutant2
        ? TR1ItemId::FlyingMutant
        : item.m_state.type;
  const auto& modelType = item.getEngine().findAnimatedModelForType(modelSourceType);
  Expects(modelType != nullptr);
  const auto& models = modelType->models;
  Expects(models.size() == item.getSkeleton()->getChildren().size());
  BOOST_LOG_TRIVIAL(trace) << "Shatter model: " << modelType->models.size() << " meshes";

  for(size_t i = 0; i < modelType->models.size(); ++i)
  {
    if(!meshMask.test(i) || !item.getSkeleton()->getChild(i)->isVisible())
    {
      BOOST_LOG_TRIVIAL(trace) << "Shatter model: mesh " << i << " skipped";
      continue;
    }

    item.getSkeleton()->getChild(i)->setVisible(false);
    auto particle = std::make_shared<MeshShrapnelParticle>(
      core::RoomBoundPosition{item.m_state.position.room,
                              core::TRVec{item.getSkeleton()->getChild(i)->getTranslationWorld()}},
      item.getEngine(),
      modelType->models[i],
      isTorsoBoss,
      damageRadius);
    particle->negSpriteFrameId = gsl::narrow<int16_t>(modelType->mesh_base_index + i);
    setParent(particle, item.m_state.position.room->node);
    item.getEngine().getParticles().emplace_back(std::move(particle));

    BOOST_LOG_TRIVIAL(trace) << "Shatter model: mesh " << i << " converted";
  }

  for(const auto& child : item.getSkeleton()->getChildren())
  {
    if(child->isVisible())
      return false;
  }

  return true;
}

MutantEgg::MutantEgg(const gsl::not_null<Engine*>& engine,
                     const gsl::not_null<const loader::file::Room*>& room,
                     loader::file::Item item,
                     const loader::file::SkeletalModelType& animatedModel)
    : ModelItemNode{engine, room, item, true, animatedModel}
{
  m_state.activationState = floordata::ActivationState(uint16_t(item.activationState & ~0x3e00u));

  switch((item.activationState & 0x3e00u) >> 9u)
  {
  case 1:
    item.type = TR1ItemId::WalkingMutant1;
    if(engine->findAnimatedModelForType(TR1ItemId::FlyingMutant) != nullptr)
      m_childItem = std::make_shared<WalkingMutant>(
        engine, room, item, *engine->findAnimatedModelForType(TR1ItemId::FlyingMutant));
    break;
  case 2:
    item.type = TR1ItemId::CentaurMutant;
    if(engine->findAnimatedModelForType(item.type) != nullptr)
      m_childItem = std::make_shared<CentaurMutant>(engine, room, item, *engine->findAnimatedModelForType(item.type));
    break;
  case 4:
    item.type = TR1ItemId::TorsoBoss;
    if(engine->findAnimatedModelForType(item.type) != nullptr)
      m_childItem = std::make_shared<TorsoBoss>(engine, room, item, *engine->findAnimatedModelForType(item.type));
    break;
  case 8:
    item.type = TR1ItemId::WalkingMutant2;
    if(engine->findAnimatedModelForType(TR1ItemId::FlyingMutant) != nullptr)
      m_childItem = std::make_shared<WalkingMutant>(
        engine, room, item, *engine->findAnimatedModelForType(TR1ItemId::FlyingMutant));
    break;
  default:
    item.type = TR1ItemId::FlyingMutant;
    if(engine->findAnimatedModelForType(TR1ItemId::FlyingMutant) != nullptr)
      m_childItem = std::make_shared<FlyingMutant>(
        engine, room, item, *engine->findAnimatedModelForType(TR1ItemId::FlyingMutant));
    break;
  }

  if(m_childItem == nullptr)
  {
    BOOST_LOG_TRIVIAL(warning) << "Mutant egg does not have an item to hatch";
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
      BOOST_LOG_TRIVIAL(debug) << getSkeleton()->getId() << ": Hatching " << m_childItem->getSkeleton()->getId();
      m_state.goal_anim_state = 1_as;
      m_state.collidable = false;
      for(size_t i = 0; i < getSkeleton()->getChildren().size(); ++i)
        getSkeleton()->getChild(i)->setVisible(i < 24);

      shatterModel(*this, 0xfffe00, 0_len);

      if(m_childItem != nullptr)
      {
        m_childItem->m_state.position = m_state.position;
        m_childItem->m_state.rotation.Y = m_state.rotation.Y;
        addChild(m_state.position.room->node, m_childItem->getNode());

        m_childItem->applyTransform();
        m_childItem->updateLighting();

        m_childItem->m_state.touch_bits.reset();
        m_childItem->m_state.initCreatureInfo(getEngine());
        m_childItem->activate();
        m_childItem->m_state.triggerState = TriggerState::Active;

        getEngine().registerItem(m_childItem);
      }
    }
  }

  ModelItemNode::update();
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
} // namespace items
} // namespace engine
