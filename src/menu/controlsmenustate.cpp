#include "controlsmenustate.h"

#include "controlswidget.h"
#include "core/i18n.h"
#include "engine/engine.h"
#include "engine/presenter.h"
#include "engine/world/world.h"
#include "hid/names.h"
#include "menudisplay.h"
#include "ui/widgets/gridbox.h"
#include "ui/widgets/label.h"

namespace menu
{
namespace
{
constexpr const auto ResetKey = hid::GlfwKey::Backspace;
constexpr const auto ResetKeyDuration = std::chrono::seconds(5);

std::shared_ptr<ControlsWidget> createControlsWidget(const engine::world::World& world)
{
  const auto createKeyLabel
    = [](const hid::InputHandler& inputHandler, hid::Action action) -> std::shared_ptr<ui::widgets::Widget>
  {
    const auto& actionMap = inputHandler.getActionMap();
    auto it = actionMap.find(action);
    if(it == actionMap.end())
    {
      return std::make_shared<ui::widgets::Label>(
        /* translators: TR charmap encoding */ pgettext("ButtonAssignment", "N/A"));
    }
    else
    {
      if(std::holds_alternative<hid::GlfwKey>(it->second))
      {
        return std::make_shared<ui::widgets::Label>(hid::getName(std::get<hid::GlfwKey>(it->second)));
      }
      else
      {
        return std::make_shared<ui::widgets::Label>(hid::getName(std::get<hid::GlfwGamepadButton>(it->second)));
      }
    }
  };
  return std::make_shared<ControlsWidget>(world.getEngine().getPresenter().getInputHandler().getActiveMappingName(),
                                          world.getEngine().getPresenter().getInputHandler(),
                                          createKeyLabel);
}
} // namespace

ControlsMenuState::ControlsMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                     std::unique_ptr<MenuState> previous,
                                     const engine::world::World& world)
    : SelectedMenuState{ringTransform}
    , m_previous{std::move(previous)}
    , m_layout{std::make_shared<ui::widgets::GridBox>()}
{
  m_layout->setExtents(1, 2);
  m_layout->set(0,
                1,
                std::make_shared<ui::widgets::Label>(
                  /* translators: TR charmap encoding */ _(
                    "To reset all mappings, hold %1% for %2% seconds.",
                    hid::getName(ResetKey),
                    std::chrono::duration_cast<std::chrono::seconds>(ResetKeyDuration).count())));
  m_controls = createControlsWidget(world);
  m_layout->set(0, 0, m_controls);
}

std::unique_ptr<MenuState> ControlsMenuState::onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& /*display*/)
{
  if(!m_resetKeyPressedSince.has_value())
  {
    if(world.getPresenter().getInputHandler().hasKey(ResetKey))
      m_resetKeyPressedSince = std::chrono::system_clock::now();
  }
  else
  {
    if(!world.getPresenter().getInputHandler().hasKey(ResetKey))
    {
      m_resetKeyPressedSince.reset();
    }
    else if(std::chrono::system_clock::now() - m_resetKeyPressedSince.value() >= ResetKeyDuration)
    {
      world.getEngine().getEngineConfig()->resetInputMappings();
      world.getEngine().getPresenter().getInputHandler().setActiveMapping(
        world.getEngine().getEngineConfig()->activeInputMapping);
      world.getEngine().getPresenter().getInputHandler().setMappings(
        world.getEngine().getEngineConfig()->inputMappings);
      m_controls->updateBindings(world.getPresenter().getInputHandler());
    }
  }

  m_layout->fitToContent();
  m_controls->fitToContent();

  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Backward))
  {
    m_controls->nextRow();
  }
  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Forward))
  {
    m_controls->prevRow();
  }
  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Right))
  {
    m_controls->nextColumn();
  }
  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Left))
  {
    m_controls->prevColumn();
  }

  const auto vp = world.getPresenter().getViewport();
  m_layout->setPosition({(vp.x - m_layout->getSize().x) / 2, vp.y - 90 - m_layout->getSize().y});
  m_layout->update(true);
  m_layout->draw(ui, world.getPresenter());

  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Menu))
  {
    return std::move(m_previous);
  }

  return nullptr;
}
} // namespace menu
