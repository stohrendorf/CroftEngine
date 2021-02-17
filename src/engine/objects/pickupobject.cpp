#include "pickupobject.h"

#include "engine/presenter.h"
#include "engine/world.h"
#include "hid/inputhandler.h"
#include "laraobject.h"

namespace engine::objects
{
void PickupObject::collide(CollisionInfo& /*collisionInfo*/)
{
  m_state.rotation.Y = getWorld().getObjectManager().getLara().m_state.rotation.Y;
  m_state.rotation.Z = 0_deg;

  if(getWorld().getObjectManager().getLara().isInWater())
  {
    if(!getWorld().getObjectManager().getLara().isDiving())
    {
      return;
    }

    static const InteractionLimits limits{
      core::BoundingBox{{-512_len, -512_len, -512_len}, {512_len, 512_len, 512_len}},
      {-45_deg, -45_deg, -45_deg},
      {+45_deg, +45_deg, +45_deg}};

    m_state.rotation.X = -25_deg;

    if(!limits.canInteract(m_state, getWorld().getObjectManager().getLara().m_state))
    {
      return;
    }

    static const core::TRVec aimSpeed{0_len, -200_len, -350_len};

    if(getWorld().getObjectManager().getLara().getCurrentAnimState() == loader::file::LaraStateId::PickUp)
    {
      if(getWorld().getObjectManager().getLara().getSkeleton()->getFrame() == 2970_frame)
      {
        m_state.triggerState = TriggerState::Invisible;
        getWorld().getInventory().put(getWorld().getObjectManager().getLara(), m_state.type);
        getWorld().addPickupWidget(getCroppedImage());
        setParent(getNode(), nullptr);
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
        getWorld().getObjectManager().getLara().updateImpl();
      } while(getWorld().getObjectManager().getLara().getCurrentAnimState() != loader::file::LaraStateId::PickUp);
      getWorld().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::UnderwaterStop);
    }
  }
  else
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

    if(getWorld().getObjectManager().getLara().getCurrentAnimState() == loader::file::LaraStateId::PickUp)
    {
      if(getWorld().getObjectManager().getLara().getSkeleton()->getFrame() == 3443_frame)
      {
        if(m_state.type == TR1ItemId::ShotgunSprite)
        {
          const auto& shotgunLara = *getWorld().findAnimatedModelForType(TR1ItemId::LaraShotgunAnim);
          BOOST_ASSERT(shotgunLara.bones.size()
                       == getWorld().getObjectManager().getLara().getSkeleton()->getBoneCount());

          getWorld().getObjectManager().getLara().getSkeleton()->setMeshPart(7, shotgunLara.bones[7].mesh);
          getWorld().getObjectManager().getLara().getSkeleton()->rebuildMesh();
        }

        m_state.triggerState = TriggerState::Invisible;
        getWorld().getInventory().put(getWorld().getObjectManager().getLara(), m_state.type);
        getWorld().addPickupWidget(getCroppedImage());
        setParent(getNode(), nullptr);
        m_state.collidable = false;
      }
    }
    else
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
          getWorld().getObjectManager().getLara().updateImpl();
        } while(getWorld().getObjectManager().getLara().getCurrentAnimState() != loader::file::LaraStateId::PickUp);
        getWorld().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::Stop);
        getWorld().getObjectManager().getLara().setHandStatus(HandStatus::Grabbing);
      }
    }
  }
}
} // namespace engine::objects
