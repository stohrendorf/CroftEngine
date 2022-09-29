#include "puzzlehole.h"

#include "core/angle.h"
#include "core/boundingbox.h"
#include "core/id.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/inventory.h"
#include "engine/items_tr1.h"
#include "engine/lighting.h"
#include "engine/objectmanager.h"
#include "engine/player.h"
#include "engine/presenter.h"
#include "engine/skeletalmodelnode.h"
#include "engine/soundeffects_tr1.h"
#include "engine/world/animation.h"
#include "engine/world/skeletalmodeltype.h"
#include "engine/world/world.h"
#include "hid/actions.h"
#include "hid/inputhandler.h"
#include "laraobject.h"
#include "loader/file/larastateid.h"
#include "modelobject.h"
#include "object.h"
#include "objectstate.h"
#include "qs/quantity.h"
#include "render/scene/node.h"

#include <boost/throw_exception.hpp>
#include <memory>
#include <stdexcept>

namespace engine::objects
{
void PuzzleHole::collide(CollisionInfo& /*collisionInfo*/)
{
  switch(m_state.type.get_as<TR1ItemId>())
  {
  case TR1ItemId::PuzzleDone1:
  case TR1ItemId::PuzzleDone2:
  case TR1ItemId::PuzzleDone3:
  case TR1ItemId::PuzzleDone4:
    return;
  default:
    break;
  }

  static const InteractionLimits limits{core::BoundingBox{{-200_len, 0_len, 312_len}, {200_len, 0_len, 512_len}},
                                        core::TRRotation{-10_deg, -30_deg, -10_deg},
                                        core::TRRotation{10_deg, 30_deg, 10_deg}};

  auto& lara = getWorld().getObjectManager().getLara();
  if(lara.getCurrentAnimState() == loader::file::LaraStateId::Stop)
  {
    if(!getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Action)
       || lara.getHandStatus() != HandStatus::None || lara.m_state.falling
       || !limits.canInteract(m_state, lara.m_state))
      return;

    if(m_state.triggerState == TriggerState::Invisible)
    {
      lara.playSoundEffect(TR1SoundEffect::LaraNo);
      return;
    }

    bool hasPuzzlePiece = false;
    switch(m_state.type.get_as<TR1ItemId>())
    {
    case TR1ItemId::PuzzleHole1:
      hasPuzzlePiece = getWorld().getPlayer().getInventory().tryTake(TR1ItemId::Puzzle1);
      break;
    case TR1ItemId::PuzzleHole2:
      hasPuzzlePiece = getWorld().getPlayer().getInventory().tryTake(TR1ItemId::Puzzle2);
      break;
    case TR1ItemId::PuzzleHole3:
      hasPuzzlePiece = getWorld().getPlayer().getInventory().tryTake(TR1ItemId::Puzzle3);
      break;
    case TR1ItemId::PuzzleHole4:
      hasPuzzlePiece = getWorld().getPlayer().getInventory().tryTake(TR1ItemId::Puzzle4);
      break;
    default:
      break;
    }
    if(!hasPuzzlePiece)
    {
      lara.playSoundEffect(TR1SoundEffect::LaraNo);
      return;
    }

    lara.alignForInteraction(core::TRVec{0_len, 0_len, 327_len}, m_state);

    do
    {
      lara.setGoalAnimState(loader::file::LaraStateId::InsertPuzzle);
      lara.advanceFrame();
    } while(lara.getCurrentAnimState() != loader::file::LaraStateId::InsertPuzzle);

    lara.setGoalAnimState(loader::file::LaraStateId::Stop);
    lara.setHandStatus(HandStatus::Grabbing);
    m_state.triggerState = TriggerState::Active;
  }
  else if(lara.getCurrentAnimState() == loader::file::LaraStateId::InsertPuzzle
          && lara.getSkeleton()->getFrame() == 3372_frame && limits.canInteract(m_state, lara.m_state))
  {
    swapPuzzleState();
  }
}

void PuzzleHole::initMesh()
{
  const auto& model = getWorld().findAnimatedModelForType(m_state.type);
  gsl_Assert(model != nullptr);

  const auto parent = m_skeleton->getParent().lock();
  setParent(gsl::not_null{m_skeleton}, nullptr);
  m_skeleton = std::make_shared<SkeletalModelNode>(
    toString(m_state.type.get_as<TR1ItemId>()), gsl::not_null{&getWorld()}, gsl::not_null{model.get()}, false);
  m_skeleton->setAnimation(
    m_state.current_anim_state, gsl::not_null{&model->animations[0]}, model->animations->firstFrame);
  setParent(gsl::not_null{m_skeleton}, parent);
  SkeletalModelNode::buildMesh(m_skeleton, m_state.current_anim_state);
  m_lighting.bind(*m_skeleton);

  ModelObject::update();
}

void PuzzleHole::swapPuzzleState()
{
  switch(m_state.type.get_as<TR1ItemId>())
  {
  case TR1ItemId::PuzzleHole1:
    m_state.type = TR1ItemId::PuzzleDone1;
    break;
  case TR1ItemId::PuzzleHole2:
    m_state.type = TR1ItemId::PuzzleDone2;
    break;
  case TR1ItemId::PuzzleHole3:
    m_state.type = TR1ItemId::PuzzleDone3;
    break;
  case TR1ItemId::PuzzleHole4:
    m_state.type = TR1ItemId::PuzzleDone4;
    break;
  case TR1ItemId::PuzzleDone1:
    m_state.type = TR1ItemId::PuzzleHole1;
    break;
  case TR1ItemId::PuzzleDone2:
    m_state.type = TR1ItemId::PuzzleHole2;
    break;
  case TR1ItemId::PuzzleDone3:
    m_state.type = TR1ItemId::PuzzleHole3;
    break;
  case TR1ItemId::PuzzleDone4:
    m_state.type = TR1ItemId::PuzzleHole4;
    break;
  default:
    BOOST_THROW_EXCEPTION(std::domain_error("Invalid puzzle ID"));
  }

  initMesh();
}

PuzzleHole::PuzzleHole(const std::string& name,
                       const gsl::not_null<world::World*>& world,
                       const gsl::not_null<const world::Room*>& room,
                       const loader::file::Item& item,
                       const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
    : ModelObject{name, world, room, item, false, animatedModel, false}
{
  initMesh();
}

PuzzleHole::PuzzleHole(const gsl::not_null<world::World*>& world, const Location& location)
    : ModelObject{world, location}
{
}
} // namespace engine::objects
