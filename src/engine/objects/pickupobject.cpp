#include "pickupobject.h"

#include "core/angle.h"
#include "core/boundingbox.h"
#include "core/genericvec.h"
#include "core/id.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/inventory.h"
#include "engine/items_tr1.h"
#include "engine/objectmanager.h"
#include "engine/objects/spriteobject.h"
#include "engine/player.h"
#include "engine/presenter.h"
#include "engine/skeletalmodelnode.h"
#include "engine/world/skeletalmodeltype.h"
#include "engine/world/world.h"
#include "hid/actions.h"
#include "hid/inputhandler.h"
#include "laraobject.h"
#include "loader/file/larastateid.h"
#include "object.h"
#include "objectstate.h"
#include "qs/quantity.h"
#include "render/scene/node.h"
#include "serialization/serialization.h"

#include <boost/assert.hpp>
#include <gslu.h>
#include <memory>
#include <vector>

namespace engine::objects
{
void PickupObject::collide(CollisionInfo& /*collisionInfo*/)
{
  m_state.rotation.Y = getWorld().getObjectManager().getLara().m_state.rotation.Y;
  m_state.rotation.Z = 0_deg;

  if(getWorld().getObjectManager().getLara().isDiving())
  {
    static const InteractionLimits limits{
      core::BoundingBox{{-512_len, -512_len, -512_len}, {512_len, 512_len, 512_len}},
      {-45_deg, -45_deg, -45_deg},
      {+45_deg, +45_deg, +45_deg}};

    m_state.rotation.X = -25_deg;

    if(!limits.canInteract(m_state, getWorld().getObjectManager().getLara().m_state))
    {
      return;
    }

    static const core::GenericVec<core::Speed> aimSpeed{0_spd, -200_spd, -350_spd};

    if(getWorld().getObjectManager().getLara().getCurrentAnimState() == loader::file::LaraStateId::PickUp)
    {
      if(getWorld().getObjectManager().getLara().getSkeleton()->getFrame() == 2970_frame)
      {
        m_state.triggerState = TriggerState::Invisible;
        ++getWorld().getPlayer().pickups;
        const auto oldType = m_state.type;
        const auto oldSprite = getSprite();
        const auto count = getWorld().getPlayer().getInventory().put(m_state.type, &getWorld());
        getWorld().addPickupWidget(getSprite(), count);
        if(oldType != m_state.type)
          getWorld().addPickupWidget(oldSprite, 1);
        getNode()->clear();
        m_state.collidable = false;
        return;
      }
    }
    else if(getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Action)
            && getWorld().getObjectManager().getLara().getCurrentAnimState()
                 == loader::file::LaraStateId::UnderwaterStop
            && getWorld().getObjectManager().getLara().alignTransform(aimSpeed, *this))
    {
      getWorld().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::PickUp);
      do
      {
        getWorld().getObjectManager().getLara().advanceFrame();
      } while(getWorld().getObjectManager().getLara().getCurrentAnimState() != loader::file::LaraStateId::PickUp);
      getWorld().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::UnderwaterStop);
    }
  }
  else if(getWorld().getObjectManager().getLara().isOnLand())
  {
    static const InteractionLimits limits{
      core::BoundingBox{{-256_len, -100_len, -256_len}, {256_len, 100_len, 100_len}},
      {-10_deg, 0_deg, 0_deg},
      {+10_deg, 0_deg, 0_deg}};

    m_state.rotation.X = 0_deg;

    if(!limits.canInteract(m_state, getWorld().getObjectManager().getLara().m_state))
    {
      return;
    }

    if(getWorld().getObjectManager().getLara().getCurrentAnimState() != loader::file::LaraStateId::PickUp)
    {
      if(getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Action)
         && getWorld().getObjectManager().getLara().getHandStatus() == HandStatus::None
         && !getWorld().getObjectManager().getLara().m_state.falling
         && getWorld().getObjectManager().getLara().getCurrentAnimState() == loader::file::LaraStateId::Stop)
      {
        getWorld().getObjectManager().getLara().alignForInteraction(core::TRVec{0_len, 0_len, -100_len}, m_state);

        getWorld().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::PickUp);
        do
        {
          getWorld().getObjectManager().getLara().advanceFrame();
        } while(getWorld().getObjectManager().getLara().getCurrentAnimState() != loader::file::LaraStateId::PickUp);
        getWorld().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::Stop);
        getWorld().getObjectManager().getLara().setHandStatus(HandStatus::Grabbing);
      }
    }
    else if(const auto& laraSkeleton = getWorld().getObjectManager().getLara().getSkeleton();
            laraSkeleton->getFrame() == 3443_frame)
    {
      if(m_state.type == TR1ItemId::ShotgunSprite)
      {
        const auto& shotgunLara = *getWorld().findAnimatedModelForType(TR1ItemId::LaraShotgunAnim);
        BOOST_ASSERT(shotgunLara.bones.size() == laraSkeleton->getBoneCount());

        laraSkeleton->setMesh(7, shotgunLara.bones[7].mesh);
        laraSkeleton->rebuildMesh();
      }

      m_state.triggerState = TriggerState::Invisible;
      ++getWorld().getPlayer().pickups;
      const auto oldType = m_state.type;
      const auto oldSprite = getSprite();
      const auto count = getWorld().getPlayer().getInventory().put(m_state.type, &getWorld());
      getWorld().addPickupWidget(getSprite(), count);
      if(oldType != m_state.type)
        getWorld().addPickupWidget(oldSprite, 1);
      getNode()->clear();
      m_state.collidable = false;
    }
  }
}

void PickupObject::serialize(const serialization::Serializer<world::World>& ser) const
{
  SpriteObject::serialize(ser);
}

void PickupObject::deserialize(const serialization::Deserializer<world::World>& ser)
{
  SpriteObject::deserialize(ser);
  // need a double-dispatch because the node is already lazily associated with its room
  ser << [this](const serialization::Deserializer<world::World>& ser)
  {
    ser << [this](const serialization::Deserializer<world::World>& /*ser*/)
    {
      if(!m_state.collidable)
        getNode()->clear();
    };
  };
}
} // namespace engine::objects
