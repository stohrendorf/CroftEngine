#include "pickupobject.h"

#include "hid/inputhandler.h"
#include "laraobject.h"

namespace engine::objects
{
void PickupObject::collide(CollisionInfo& /*collisionInfo*/)
{
  m_state.rotation.Y = getEngine().getLara().m_state.rotation.Y;
  m_state.rotation.Z = 0_deg;

  if(getEngine().getLara().isInWater())
  {
    if(!getEngine().getLara().isDiving())
    {
      return;
    }

    static const InteractionLimits limits{
      core::BoundingBox{{-512_len, -512_len, -512_len}, {512_len, 512_len, 512_len}},
      {-45_deg, -45_deg, -45_deg},
      {+45_deg, +45_deg, +45_deg}};

    m_state.rotation.X = -25_deg;

    if(!limits.canInteract(m_state, getEngine().getLara().m_state))
    {
      return;
    }

    static const core::TRVec aimSpeed{0_len, -200_len, -350_len};

    if(getEngine().getLara().getCurrentAnimState() == loader::file::LaraStateId::PickUp)
    {
      if(getEngine().getLara().getSkeleton()->frame_number == 2970_frame)
      {
        m_state.triggerState = TriggerState::Invisible;
        getEngine().getInventory().put(m_state.type);
        setParent(getNode(), nullptr);
        m_state.collidable = false;
        return;
      }
    }
    else if(getEngine().getInputHandler().getInputState().action
            && getEngine().getLara().getCurrentAnimState() == loader::file::LaraStateId::UnderwaterStop
            && getEngine().getLara().alignTransform(aimSpeed, *this))
    {
      getEngine().getLara().setGoalAnimState(loader::file::LaraStateId::PickUp);
      do
      {
        getEngine().getLara().updateImpl();
      } while(getEngine().getLara().getCurrentAnimState() != loader::file::LaraStateId::PickUp);
      getEngine().getLara().setGoalAnimState(loader::file::LaraStateId::UnderwaterStop);
    }
  }
  else
  {
    static const InteractionLimits limits{
      core::BoundingBox{{-256_len, -100_len, -256_len}, {256_len, 100_len, 100_len}},
      {-10_deg, 0_deg, 0_deg},
      {+10_deg, 0_deg, 0_deg}};

    m_state.rotation.X = 0_deg;

    if(!limits.canInteract(m_state, getEngine().getLara().m_state))
    {
      return;
    }

    if(getEngine().getLara().getCurrentAnimState() == loader::file::LaraStateId::PickUp)
    {
      if(getEngine().getLara().getSkeleton()->frame_number == 3443_frame)
      {
        if(m_state.type == TR1ItemId::ShotgunSprite)
        {
          const auto& shotgunLara = *getEngine().findAnimatedModelForType(TR1ItemId::LaraShotgunAnim);
          BOOST_ASSERT(gsl::narrow<size_t>(shotgunLara.meshes.size())
                       == getEngine().getLara().getNode()->getChildren().size());

          getEngine().getLara().getNode()->getChild(7)->setRenderable(shotgunLara.models[7].get());
        }

        m_state.triggerState = TriggerState::Invisible;
        getEngine().getInventory().put(m_state.type);
        setParent(getNode(), nullptr);
        m_state.collidable = false;
      }
    }
    else
    {
      if(getEngine().getInputHandler().getInputState().action
         && getEngine().getLara().getHandStatus() == HandStatus::None && !getEngine().getLara().m_state.falling
         && getEngine().getLara().getCurrentAnimState() == loader::file::LaraStateId::Stop)
      {
        getEngine().getLara().alignForInteraction(core::TRVec{0_len, 0_len, -100_len}, m_state);

        getEngine().getLara().setGoalAnimState(loader::file::LaraStateId::PickUp);
        do
        {
          getEngine().getLara().updateImpl();
        } while(getEngine().getLara().getCurrentAnimState() != loader::file::LaraStateId::PickUp);
        getEngine().getLara().setGoalAnimState(loader::file::LaraStateId::Stop);
        getEngine().getLara().setHandStatus(HandStatus::Grabbing);
      }
    }
  }
}
} // namespace engine::objects
