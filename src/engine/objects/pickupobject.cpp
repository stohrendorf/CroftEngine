#include "pickupobject.h"

#include "engine/presenter.h"
#include "hid/inputhandler.h"
#include "laraobject.h"

namespace engine::objects
{
void PickupObject::collide(CollisionInfo& /*collisionInfo*/)
{
  m_state.rotation.Y = getEngine().getObjectManager().getLara().m_state.rotation.Y;
  m_state.rotation.Z = 0_deg;

  if(getEngine().getObjectManager().getLara().isInWater())
  {
    if(!getEngine().getObjectManager().getLara().isDiving())
    {
      return;
    }

    static const InteractionLimits limits{
      core::BoundingBox{{-512_len, -512_len, -512_len}, {512_len, 512_len, 512_len}},
      {-45_deg, -45_deg, -45_deg},
      {+45_deg, +45_deg, +45_deg}};

    m_state.rotation.X = -25_deg;

    if(!limits.canInteract(m_state, getEngine().getObjectManager().getLara().m_state))
    {
      return;
    }

    static const core::TRVec aimSpeed{0_len, -200_len, -350_len};

    if(getEngine().getObjectManager().getLara().getCurrentAnimState() == loader::file::LaraStateId::PickUp)
    {
      if(getEngine().getObjectManager().getLara().getSkeleton()->frame_number == 2970_frame)
      {
        m_state.triggerState = TriggerState::Invisible;
        getEngine().getInventory().put(getEngine().getObjectManager().getLara(), m_state.type);
        setParent(getNode(), nullptr);
        m_state.collidable = false;
        return;
      }
    }
    else if(getEngine().getPresenter().getInputHandler().getInputState().action
            && getEngine().getObjectManager().getLara().getCurrentAnimState()
                 == loader::file::LaraStateId::UnderwaterStop
            && getEngine().getObjectManager().getLara().alignTransform(aimSpeed, *this))
    {
      getEngine().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::PickUp);
      do
      {
        getEngine().getObjectManager().getLara().updateImpl();
      } while(getEngine().getObjectManager().getLara().getCurrentAnimState() != loader::file::LaraStateId::PickUp);
      getEngine().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::UnderwaterStop);
    }
  }
  else
  {
    static const InteractionLimits limits{
      core::BoundingBox{{-256_len, -100_len, -256_len}, {256_len, 100_len, 100_len}},
      {-10_deg, 0_deg, 0_deg},
      {+10_deg, 0_deg, 0_deg}};

    m_state.rotation.X = 0_deg;

    if(!limits.canInteract(m_state, getEngine().getObjectManager().getLara().m_state))
    {
      return;
    }

    if(getEngine().getObjectManager().getLara().getCurrentAnimState() == loader::file::LaraStateId::PickUp)
    {
      if(getEngine().getObjectManager().getLara().getSkeleton()->frame_number == 3443_frame)
      {
        if(m_state.type == TR1ItemId::ShotgunSprite)
        {
          const auto& shotgunLara = *getEngine().findAnimatedModelForType(TR1ItemId::LaraShotgunAnim);
          BOOST_ASSERT(shotgunLara.bones.size()
                       == getEngine().getObjectManager().getLara().getSkeleton()->getBoneCount());

          getEngine().getObjectManager().getLara().getSkeleton()->setMeshPart(7, shotgunLara.bones[7].mesh);
          getEngine().getObjectManager().getLara().getSkeleton()->rebuildMesh();
        }

        m_state.triggerState = TriggerState::Invisible;
        getEngine().getInventory().put(getEngine().getObjectManager().getLara(), m_state.type);
        setParent(getNode(), nullptr);
        m_state.collidable = false;
      }
    }
    else
    {
      if(getEngine().getPresenter().getInputHandler().getInputState().action
         && getEngine().getObjectManager().getLara().getHandStatus() == HandStatus::None
         && !getEngine().getObjectManager().getLara().m_state.falling
         && getEngine().getObjectManager().getLara().getCurrentAnimState() == loader::file::LaraStateId::Stop)
      {
        getEngine().getObjectManager().getLara().alignForInteraction(core::TRVec{0_len, 0_len, -100_len}, m_state);

        getEngine().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::PickUp);
        do
        {
          getEngine().getObjectManager().getLara().updateImpl();
        } while(getEngine().getObjectManager().getLara().getCurrentAnimState() != loader::file::LaraStateId::PickUp);
        getEngine().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::Stop);
        getEngine().getObjectManager().getLara().setHandStatus(HandStatus::Grabbing);
      }
    }
  }
}
} // namespace engine::objects
