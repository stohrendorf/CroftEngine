#include "puzzlehole.h"

#include "engine/presenter.h"
#include "engine/world.h"
#include "hid/inputhandler.h"
#include "laraobject.h"

namespace engine::objects
{
void PuzzleHole::collide(CollisionInfo& /*collisionInfo*/)
{
  switch(m_state.type.get_as<TR1ItemId>())
  {
  case TR1ItemId::PuzzleDone1:
  case TR1ItemId::PuzzleDone2:
  case TR1ItemId::PuzzleDone3:
  case TR1ItemId::PuzzleDone4: return;
  default: break;
  }

  static const InteractionLimits limits{core::BoundingBox{{-200_len, 0_len, 312_len}, {200_len, 0_len, 512_len}},
                                        core::TRRotation{-10_deg, -30_deg, -10_deg},
                                        core::TRRotation{10_deg, 30_deg, 10_deg}};

  if(getWorld().getObjectManager().getLara().getCurrentAnimState() == loader::file::LaraStateId::Stop)
  {
    if(!getWorld().getPresenter().getInputHandler().getInputState().action
       || getWorld().getObjectManager().getLara().getHandStatus() != HandStatus::None
       || getWorld().getObjectManager().getLara().m_state.falling
       || !limits.canInteract(m_state, getWorld().getObjectManager().getLara().m_state))
      return;

    if(m_state.triggerState == TriggerState::Invisible)
    {
      getWorld().getObjectManager().getLara().playSoundEffect(TR1SoundEffect::LaraNo);
      return;
    }

    bool hasPuzzlePiece = false;
    switch(m_state.type.get_as<TR1ItemId>())
    {
    case TR1ItemId::PuzzleHole1: hasPuzzlePiece = getWorld().getInventory().tryTake(TR1ItemId::Puzzle1); break;
    case TR1ItemId::PuzzleHole2: hasPuzzlePiece = getWorld().getInventory().tryTake(TR1ItemId::Puzzle2); break;
    case TR1ItemId::PuzzleHole3: hasPuzzlePiece = getWorld().getInventory().tryTake(TR1ItemId::Puzzle3); break;
    case TR1ItemId::PuzzleHole4: hasPuzzlePiece = getWorld().getInventory().tryTake(TR1ItemId::Puzzle4); break;
    default: break;
    }
    if(!hasPuzzlePiece)
    {
      getWorld().getObjectManager().getLara().playSoundEffect(TR1SoundEffect::LaraNo);
      return;
    }

    getWorld().getObjectManager().getLara().alignForInteraction(core::TRVec{0_len, 0_len, 327_len}, m_state);

    do
    {
      getWorld().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::InsertPuzzle);
      getWorld().getObjectManager().getLara().updateImpl();
    } while(getWorld().getObjectManager().getLara().getCurrentAnimState() != loader::file::LaraStateId::InsertPuzzle);

    getWorld().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::Stop);
    getWorld().getObjectManager().getLara().setHandStatus(HandStatus::Grabbing);
    m_state.triggerState = TriggerState::Active;
  }
  else if(getWorld().getObjectManager().getLara().getCurrentAnimState() == loader::file::LaraStateId::InsertPuzzle
          && getWorld().getObjectManager().getLara().getSkeleton()->frame_number == 3372_frame
          && limits.canInteract(m_state, getWorld().getObjectManager().getLara().m_state))
  {
    TR1ItemId completeId;

    switch(m_state.type.get_as<TR1ItemId>())
    {
    case TR1ItemId::PuzzleHole1: completeId = TR1ItemId::PuzzleDone1; break;
    case TR1ItemId::PuzzleHole2: completeId = TR1ItemId::PuzzleDone2; break;
    case TR1ItemId::PuzzleHole3: completeId = TR1ItemId::PuzzleDone3; break;
    case TR1ItemId::PuzzleHole4: completeId = TR1ItemId::PuzzleDone4; break;
    default: BOOST_THROW_EXCEPTION(std::runtime_error("Invalid puzzle ID"));
    }

    const auto& model = getWorld().findAnimatedModelForType(completeId);
    Expects(model != nullptr);

    const auto parent = m_skeleton->getParent().lock();
    setParent(m_skeleton, nullptr);
    m_state.type = completeId;
    m_skeleton = std::make_shared<SkeletalModelNode>(toString(completeId), &getWorld(), model.get());
    m_skeleton->setAnimation(m_state.current_anim_state, model->animations, model->animations->firstFrame);
    setParent(m_skeleton, parent);
    SkeletalModelNode::buildMesh(m_skeleton, m_state.current_anim_state);
    m_lighting.bind(*m_skeleton);

    ModelObject::update();
  }
}
} // namespace engine::objects
