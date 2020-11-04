#include "puzzlehole.h"

#include "engine/presenter.h"
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

  if(getEngine().getObjectManager().getLara().getCurrentAnimState() == loader::file::LaraStateId::Stop)
  {
    if(!getEngine().getPresenter().getInputHandler().getInputState().action
       || getEngine().getObjectManager().getLara().getHandStatus() != HandStatus::None
       || getEngine().getObjectManager().getLara().m_state.falling
       || !limits.canInteract(m_state, getEngine().getObjectManager().getLara().m_state))
      return;

    if(m_state.triggerState == TriggerState::Invisible)
    {
      getEngine().getObjectManager().getLara().playSoundEffect(TR1SoundId::LaraNo);
      return;
    }

    bool hasPuzzlePiece = false;
    switch(m_state.type.get_as<TR1ItemId>())
    {
    case TR1ItemId::PuzzleHole1: hasPuzzlePiece = getEngine().getInventory().tryTake(TR1ItemId::Puzzle1); break;
    case TR1ItemId::PuzzleHole2: hasPuzzlePiece = getEngine().getInventory().tryTake(TR1ItemId::Puzzle2); break;
    case TR1ItemId::PuzzleHole3: hasPuzzlePiece = getEngine().getInventory().tryTake(TR1ItemId::Puzzle3); break;
    case TR1ItemId::PuzzleHole4: hasPuzzlePiece = getEngine().getInventory().tryTake(TR1ItemId::Puzzle4); break;
    default: break;
    }
    if(!hasPuzzlePiece)
    {
      getEngine().getObjectManager().getLara().playSoundEffect(TR1SoundId::LaraNo);
      return;
    }

    getEngine().getObjectManager().getLara().alignForInteraction(core::TRVec{0_len, 0_len, 327_len}, m_state);

    do
    {
      getEngine().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::InsertPuzzle);
      getEngine().getObjectManager().getLara().updateImpl();
    } while(getEngine().getObjectManager().getLara().getCurrentAnimState() != loader::file::LaraStateId::InsertPuzzle);

    getEngine().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::Stop);
    getEngine().getObjectManager().getLara().setHandStatus(HandStatus::Grabbing);
    m_state.triggerState = TriggerState::Active;
  }
  else if(getEngine().getObjectManager().getLara().getCurrentAnimState() == loader::file::LaraStateId::InsertPuzzle
          && getEngine().getObjectManager().getLara().getSkeleton()->frame_number == 3372_frame
          && limits.canInteract(m_state, getEngine().getObjectManager().getLara().m_state))
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

    const auto& model = getEngine().findAnimatedModelForType(completeId);
    Expects(model != nullptr);

    const auto parent = m_skeleton->getParent().lock();
    setParent(m_skeleton, nullptr);

    m_skeleton = std::make_shared<SkeletalModelNode>(toString(completeId), &getEngine(), model.get());
    m_skeleton->setAnimation(m_state.current_anim_state, model->animations, model->animations->firstFrame);
    loader::file::RenderMeshDataCompositor compositor;
    for(auto& bone : model->bones)
    {
      compositor.append(*bone.mesh);
    }
    m_skeleton->setRenderable(compositor.toMesh(*getEngine().getPresenter().getMaterialManager(), false, {}));

    setParent(m_skeleton, parent);

    m_state.type = completeId;
    ModelObject::update();
  }
}
} // namespace engine::objects
