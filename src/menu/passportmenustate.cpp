#include "passportmenustate.h"

#include "donemenustate.h"
#include "engine/audioengine.h"
#include "engine/engine.h"
#include "engine/i18n.h"
#include "engine/presenter.h"
#include "engine/world.h"
#include "finishitemanimationmenustate.h"
#include "idleringmenustate.h"
#include "menudisplay.h"
#include "menuring.h"
#include "resetitemtransformmenustate.h"
#include "savegamelistmenustate.h"
#include "setitemtypemenustate.h"
#include "util.h"

namespace menu
{
void PassportMenuState::handleObject(engine::World& /*world*/, MenuDisplay& display, MenuObject& object)
{
  if(&object != &display.getCurrentRing().getSelectedObject())
    zeroRotation(object, 256_au);
  else
    rotateForSelection(object);
}

std::optional<std::unique_ptr<MenuState>> PassportMenuState::showLoadGamePage(engine::World& world,
                                                                              MenuDisplay& display)
{
  if(m_passportText == nullptr)
  {
    m_passportText = std::make_unique<ui::Label>(glm::ivec2{0, -16}, world.getEngine().i18n(engine::I18n::LoadGame));
    m_passportText->alignX = ui::Label::Alignment::Center;
    m_passportText->alignY = ui::Label::Alignment::Bottom;
  }
  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Action)
     || display.mode == InventoryMode::LoadMode)
  {
    display.objectTexts[0].reset();
    display.objectTexts[2].reset();
    return create<SavegameListMenuState>(std::move(display.m_currentState), m_passportText->text, world, true);
  }

  return std::nullopt;
}

std::optional<std::unique_ptr<MenuState>>
  PassportMenuState::showSaveGamePage(engine::World& world, MenuDisplay& display, bool isInGame)
{
  if(m_passportText == nullptr)
  {
    m_passportText = std::make_unique<ui::Label>(
      glm::ivec2{0, -16},
      world.getEngine().i18n(m_allowSave && isInGame ? engine::I18n::SaveGame : engine::I18n::NewGame));
    m_passportText->alignX = ui::Label::Alignment::Center;
    m_passportText->alignY = ui::Label::Alignment::Bottom;
  }
  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Action))
  {
    if(display.mode == InventoryMode::SaveMode || (m_allowSave && isInGame))
    {
      display.objectTexts[0].reset();
      display.objectTexts[2].reset();
      return create<SavegameListMenuState>(std::move(display.m_currentState), m_passportText->text, world, false);
    }
    else
    {
      return create<DoneMenuState>(MenuResult::NewGame);
    }
  }

  return std::nullopt;
}

void PassportMenuState::showExitGamePage(engine::World& world, MenuDisplay& display, bool isInGame)
{
  if(m_passportText == nullptr)
  {
    m_passportText = std::make_unique<ui::Label>(
      glm::ivec2{0, -16}, world.getEngine().i18n(!isInGame ? engine::I18n::ExitGame : engine::I18n::ExitToTitle));
    m_passportText->alignX = ui::Label::Alignment::Center;
    m_passportText->alignY = ui::Label::Alignment::Bottom;
  }
  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Action))
  {
    display.result = !isInGame ? MenuResult::ExitGame : MenuResult::ExitToTitle;
  }
}

void PassportMenuState::prevPage(const core::Frame& minFrame, MenuObject& passport, engine::World& world)
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

void PassportMenuState::nextPage(MenuObject& passport, engine::World& world)
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

std::unique_ptr<MenuState> PassportMenuState::onFrame(ui::Ui& ui, engine::World& world, MenuDisplay& display)
{
  auto& passport = display.getCurrentRing().getSelectedObject();
  passport.type = engine::TR1ItemId::PassportOpening;
  passport.initModel(world);

  if(passport.selectedRotationY == passport.rotationY && passport.animate())
    return nullptr;

  const bool isInGame = display.mode != InventoryMode::TitleMode && display.mode != InventoryMode::DeathMode;
  const bool hasSavedGames = world.hasSavedGames();

  display.objectTexts[0].reset();
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
    if(!hasSavedGames || display.mode == InventoryMode::SaveMode)
    {
      forcePageTurn = hid::AxisMovement::Right;
      break;
    }
    if(auto tmp = showLoadGamePage(world, display))
      return std::move(tmp.value());
    break;
  case SaveGamePage:
    if(!m_allowSave && display.mode != InventoryMode::TitleMode)
    {
      // can't save when dead, so just skip this page
      if(passport.animDirection == -1_frame)
        forcePageTurn = hid::AxisMovement::Left;
      else
        forcePageTurn = hid::AxisMovement::Right;
      break;
    }
    if(auto tmp = showSaveGamePage(world, display, isInGame))
      return std::move(tmp.value());
    break;
  case ExitGamePage: showExitGamePage(world, display, isInGame); break;
  default: Expects(page == -1); break;
  }

  if(m_passportText != nullptr)
    m_passportText->draw(ui, world.getPresenter().getTrFont(), world.getPresenter().getViewport());

  if(forcePageTurn == hid::AxisMovement::Left
     || world.getPresenter().getInputHandler().getInputState().xMovement.justChangedTo(hid::AxisMovement::Left))
  {
    if(hasSavedGames)
    {
      prevPage(0_frame, passport, world);
      return nullptr;
    }
    else if(m_allowSave || display.mode == InventoryMode::TitleMode)
    {
      prevPage(FramesPerPage, passport, world);
      return nullptr;
    }
  }
  else if(forcePageTurn == hid::AxisMovement::Right
          || world.getPresenter().getInputHandler().getInputState().xMovement.justChangedTo(hid::AxisMovement::Right))
  {
    nextPage(passport, world);
    return nullptr;
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Menu))
  {
    if(!m_allowExit && display.mode != InventoryMode::TitleMode)
      return nullptr;

    return close(display, page, passport);
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Action))
  {
    return close(display, page, passport);
  }

  return nullptr;
}

PassportMenuState::PassportMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                     InventoryMode mode,
                                     bool allowSave)
    : MenuState{ringTransform}
    , m_allowExit{mode != InventoryMode::DeathMode && mode != InventoryMode::TitleMode}
    , m_allowSave{allowSave && (mode != InventoryMode::DeathMode && mode != InventoryMode::TitleMode)}
    , m_forcePage{mode == InventoryMode::LoadMode   ? std::optional<int>{0}
                  : mode == InventoryMode::SaveMode ? std::optional<int>{1}
                                                    : std::nullopt}
{
}

std::unique_ptr<MenuState> PassportMenuState::close(MenuDisplay& /*display*/, int page, MenuObject& passport)
{
  m_passportText.reset();

  if(page == ExitGamePage)
  {
    passport.goalFrame = passport.lastMeshAnimFrame - 1_frame;
    passport.animDirection = 1_frame;
  }
  else
  {
    passport.goalFrame = 0_frame;
    passport.animDirection = -1_frame;
  }

  return create<FinishItemAnimationMenuState>(create<SetItemTypeMenuState>(
    engine::TR1ItemId::PassportClosed, create<ResetItemTransformMenuState>(create<IdleRingMenuState>(false))));
}
} // namespace menu
