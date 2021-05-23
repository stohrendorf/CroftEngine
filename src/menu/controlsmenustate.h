#pragma once

#include "selectedmenustate.h"
#include "ui/widgets/groupbox.h"

#include <functional>

namespace ui
{
struct Label;
}

namespace ui::widgets
{
class GroupBox;
}

namespace engine
{
class Engine;
}

namespace menu
{
class ControlsMenuState : public SelectedMenuState
{
private:
  std::shared_ptr<ui::widgets::GroupBox> m_keyboardControls{};
  std::shared_ptr<ui::widgets::GroupBox> m_gamepadControls{};
  std::unique_ptr<MenuState> m_previous;

public:
  explicit ControlsMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                             std::unique_ptr<MenuState> previous,
                             engine::Engine& engine);

  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
};
} // namespace menu
