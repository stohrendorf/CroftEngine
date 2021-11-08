#include "centaurstatue.h"

#include "aiagent.h"
#include "core/angle.h"
#include "core/genericvec.h"
#include "core/id.h"
#include "core/magic.h"
#include "core/units.h"
#include "engine/floordata/floordata.h"
#include "engine/items_tr1.h"
#include "engine/location.h"
#include "engine/objectmanager.h"
#include "engine/skeletalmodelnode.h"
#include "engine/soundeffects_tr1.h"
#include "engine/world/animation.h"
#include "engine/world/skeletalmodeltype.h"
#include "engine/world/world.h"
#include "laraobject.h"
#include "loader/file/item.h"
#include "modelobject.h"
#include "mutant.h"
#include "object.h"
#include "objectstate.h"
#include "qs/qs.h"
#include "serialization/objectreference.h"
#include "serialization/serialization.h"
#include "shatter.h"
#include "util/helpers.h"

#include <bitset>
#include <boost/log/trivial.hpp>
#include <exception>
#include <map>

namespace engine::objects
{
CentaurStatue::CentaurStatue(const std::string& name,
                             const gsl::not_null<world::World*>& world,
                             const gsl::not_null<const world::Room*>& room,
                             loader::file::Item item,
                             const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
    : ModelObject{name, world, room, item, true, animatedModel, true}
{
  if(const auto& model = world->findAnimatedModelForType(TR1ItemId::CentaurMutant); model != nullptr)
  {
    item.type = TR1ItemId::CentaurMutant;
    item.activationState = 0;
    m_childObject = std::make_shared<CentaurMutant>(
      makeObjectName(item.type.get_as<TR1ItemId>(), getWorld().getObjectManager().getObjectCounter()),
      world,
      room,
      item,
      gsl::not_null{model.get()});
    auto& childState = m_childObject->m_state;
    childState.activationState.setOneshot(true);
    m_childObject->getSkeleton()->setAnimation(
      childState.current_anim_state, gsl::not_null{&model->animations[7]}, model->animations[7].firstFrame + 36_frame);
    childState.goal_anim_state = childState.current_anim_state = model->animations[7].state_id;
    childState.rotation.Y = m_state.rotation.Y;
    getWorld().getObjectManager().registerObject(gsl::not_null{m_childObject});
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

  static constexpr auto Radius = core::SectorSize * 3.5f;
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

void CentaurStatue::collide(CollisionInfo& info)
{
  collideWithLara(info);
}
} // namespace engine::objects
