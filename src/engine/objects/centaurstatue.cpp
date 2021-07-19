#include "centaurstatue.h"

#include "engine/particle.h"
#include "engine/presenter.h"
#include "engine/world/rendermeshdata.h"
#include "laraobject.h"
#include "mutant.h"
#include "mutantegg.h"
#include "render/scene/mesh.h"
#include "serialization/objectreference.h"
#include "serialization/quantity.h"
#include "serialization/serialization.h"

namespace engine::objects
{
CentaurStatue::CentaurStatue(const gsl::not_null<world::World*>& world,
                             const gsl::not_null<const world::Room*>& room,
                             loader::file::Item item,
                             const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
    : ModelObject{world, room, item, true, animatedModel}
{
  if(const auto& model = world->findAnimatedModelForType(TR1ItemId::CentaurMutant); model != nullptr)
  {
    item.type = TR1ItemId::CentaurMutant;
    item.activationState = 0;
    m_childObject = std::make_shared<CentaurMutant>(world, room, item, model.get());
    m_childObject->m_state.activationState.setOneshot(true);
    m_childObject->getSkeleton()->setAnimation(
      m_childObject->m_state.current_anim_state, &model->animations[7], model->animations[7].firstFrame + 36_frame);
    m_childObject->m_state.goal_anim_state = m_childObject->m_state.current_anim_state = model->animations[7].state_id;
    m_childObject->m_state.rotation.Y = m_state.rotation.Y;
    getWorld().getObjectManager().registerObject(m_childObject);
  }
  else
  {
    BOOST_LOG_TRIVIAL(warning) << "Mutant egg does not have an object to hatch";
  }
}

void CentaurStatue::update()
{
  const auto d = getWorld().getObjectManager().getLara().m_state.location.position - m_state.location.position;
  if(abs(d.Y) >= core::SectorSize)
    return;

  static constexpr auto Radius = core::SectorSize * 7 / 2;
  if(util::square(d.X) + util::square(d.Z) >= util::square(Radius))
    return;

  shatterModel(*this, 0xffffffffu, 0_len);
  kill();
  m_state.triggerState = TriggerState::Deactivated;
  m_childObject->m_state.touch_bits = 0;
  m_childObject->activate();
  m_childObject->m_state.triggerState = TriggerState::Active;
  m_childObject->playSoundEffect(TR1SoundEffect::Explosion2);
}

void CentaurStatue::serialize(const serialization::Serializer<world::World>& ser)
{
  ModelObject::serialize(ser);
  ser(S_NV("childObject", serialization::ObjectReference{m_childObject}));
}
} // namespace engine::objects
