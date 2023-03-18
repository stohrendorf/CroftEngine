#include "passportmenustate.h"

#include "closepassportmenustate.h"
#include "core/i18n.h"
#include "donemenustate.h"
#include "engine/audioengine.h"
#include "engine/items_tr1.h"
#include "engine/presenter.h"
#include "engine/soundeffects_tr1.h"
#include "engine/world/world.h"
#include "hid/actions.h"
#include "hid/inputhandler.h"
#include "hid/inputstate.h"
#include "idleringmenustate.h"
#include "menu/menuobject.h"
#include "menu/menustate.h"
#include "menudisplay.h"
#include "menuring.h"
#include "qs/qs.h"
#include "savegamelistmenustate.h"
#include "ui/ui.h"
#include "util.h"

#include <boost/assert.hpp>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <utility>

namespace menu
{
void PassportMenuState::handleObject(ui::Ui& /*ui*/,
                                     engine::world::World& /*world*/,
                                     MenuDisplay& display,
                                     MenuObject& object)
{
  if(&object != &display.getCurrentRing().getSelectedObject())
    zeroRotation(object, 256_au);
  else
    rotateForSelection(object);
}

std::optional<std::unique_ptr<MenuState>> PassportMenuState::showLoadGamePage(engine::world::World& world,
                                                                              MenuDisplay& display)
{
  const auto title = /* translators: TR charmap encoding */ _("Load Game");

  if(m_passportText == nullptr)
  {
    m_passportText = std::make_unique<ui::Text>(title);
  }

  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::PrimaryInteraction))
  {
    return create<SavegameListMenuState>(std::move(display.m_currentState), title, world, true);
  }

  return std::nullopt;
}

std::optional<std::unique_ptr<MenuState>> PassportMenuState::showSaveGamePage(engine::world::World& world,
                                                                              MenuDisplay& display)
{
  const char* title = nullptr;
  switch(m_saveGamePageMode)
  {
  case SaveGamePageMode::Skip:
    BOOST_ASSERT(false);
    return nullptr;
  case SaveGamePageMode::NewGame:
    title = /* translators: TR charmap encoding */ _("New Game");
    break;
  case SaveGamePageMode::Save:
    title = /* translators: TR charmap encoding */ _("Save Game");
    break;
  case SaveGamePageMode::Restart:
    title = /* translators: TR charmap encoding */ _("Restart Level");
    break;
  }

  if(m_passportText == nullptr)
  {
    m_passportText = std::make_unique<ui::Text>(title);
  }

  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::PrimaryInteraction))
  {
    if(m_saveGamePageMode == SaveGamePageMode::Save)
    {
      return create<SavegameListMenuState>(std::move(display.m_currentState), title, world, false);
    }
    else
    {
      return create<DoneMenuState>(m_saveGamePageMode == SaveGamePageMode::NewGame ? MenuResult::NewGame
                                                                                   : MenuResult::RestartLevel);
    }
  }

  return std::nullopt;
}

void PassportMenuState::showExitGamePage(engine::world::World& world, MenuDisplay& display, bool returnToTitle)
{
  if(m_passportText == nullptr)
  {
    m_passportText
      = std::make_unique<ui::Text>(returnToTitle ? /* translators: TR charmap encoding */ _("Exit to Title")
                                                 : /* translators: TR charmap encoding */ _("Exit Game"));
  }

  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::PrimaryInteraction))
  {
    display.result = returnToTitle ? MenuResult::ExitToTitle : MenuResult::ExitGame;
  }
}

void PassportMenuState::prevPage(const core::Frame& minFrame, MenuObject& passport, engine::world::World& world)
{
  passport.goalFrame -= FramesPerPage;
  passport.animDirection = -1_frame;
  if(const auto firstFrame = passport.openFrame + minFrame; passport.goalFrame < firstFrame)
  {
    passport.goalFrame = firstFrame;
  }
  else
  {
    world.getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuGamePageTurn, nullptr);
    m_passportText.reset();
  }
}

void PassportMenuState::nextPage(MenuObject& passport, engine::world::World& world)
{
  passport.goalFrame += FramesPerPage;
  passport.animDirection = 1_frame;
  if(const auto lastFrame = passport.lastMeshAnimFrame - FramesPerPage - 1_frame; passport.goalFrame > lastFrame)
  {
    passport.goalFrame = lastFrame;
  }
  else
  {
    world.getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuGamePageTurn, nullptr);
    m_passportText.reset();
  }
}

std::unique_ptr<MenuState> PassportMenuState::onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display)
{
  auto& passport = display.getCurrentRing().getSelectedObject();
  passport.type = engine::TR1ItemId::PassportOpening;
  passport.initModel(world, display.getLightsBuffer());

  if(passport.selectedRotationY == passport.rotationY && passport.animate())
    return nullptr;

  const bool hasSavedGames = world.hasSavedGames();

  const auto localFrame = passport.goalFrame - passport.openFrame;
  auto page = localFrame / FramesPerPage;
  hid::AxisMovement forcePageTurn = hid::AxisMovement::Null;
  if(localFrame % FramesPerPage != 0_frame)
  {
    page = -1;
  }
  else if(m_forcePage.value_or(page) != page)
  {
    page = -1;
    if(page < *m_forcePage)
      forcePageTurn = hid::AxisMovement::Right;
    else if(page > *m_forcePage)
      forcePageTurn = hid::AxisMovement::Left;
    else
      m_forcePage.reset();
  }

  switch(page)
  {
  case LoadGamePage:
    if(!hasSavedGames)
    {
      forcePageTurn = hid::AxisMovement::Right;
      break;
    }
    if(auto tmp = showLoadGamePage(world, display))
      return std::move(*tmp);
    break;
  case SaveGamePage:
    if(m_saveGamePageMode == SaveGamePageMode::Skip)
    {
      if(passport.animDirection == -1_frame)
        forcePageTurn = hid::AxisMovement::Left;
      else
        forcePageTurn = hid::AxisMovement::Right;
      break;
    }
    if(auto tmp = showSaveGamePage(world, display))
      return std::move(*tmp);
    break;
  case ExitGamePage:
    showExitGamePage(world, display, display.mode != InventoryMode::TitleMode);
    break;
  default:
    gsl_Assert(page == -1);
    break;
  }

  if(m_passportText != nullptr)
    m_passportText->draw(
      ui, world.getPresenter().getTrFont(), {(ui.getSize().x - m_passportText->getWidth()) / 2, ui.getSize().y - 16});

  if(forcePageTurn == hid::AxisMovement::Left
     || world.getPresenter().getInputHandler().getInputState().menuXMovement.justChangedTo(hid::AxisMovement::Left))
  {
    if(hasSavedGames)
    {
      prevPage(0_frame, passport, world);
      return nullptr;
    }
    else
    {
      if(m_saveGamePageMode == SaveGamePageMode::Save || display.mode == InventoryMode::TitleMode)
      {
        prevPage(SaveGamePage * FramesPerPage, passport, world);
        return nullptr;
      }
    }
  }
  else if(forcePageTurn == hid::AxisMovement::Right
          || world.getPresenter().getInputHandler().getInputState().menuXMovement.justChangedTo(
            hid::AxisMovement::Right))
  {
    nextPage(passport, world);
    return nullptr;
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Return))
  {
    if(!m_allowExit && display.mode != InventoryMode::TitleMode)
      return nullptr;

    return create<ClosePassportMenuState>(passport, create<IdleRingMenuState>(false));
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::PrimaryInteraction))
  {
    return create<ClosePassportMenuState>(passport, create<IdleRingMenuState>(false));
  }

  return nullptr;
}

PassportMenuState::PassportMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                     bool allowExit,
                                     SaveGamePageMode saveGamePageMode)
    : MenuState{ringTransform}
    , m_allowExit{allowExit}
    , m_saveGamePageMode{saveGamePageMode}
{
}
} // namespace menu
