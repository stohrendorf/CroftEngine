#pragma once

#include "selectedmenustate.h"
#include "ui/widgets/groupbox.h"

#include <functional>

namespace ui::widgets
{
class GridBox;
}

namespace engine::world
{
class World;
}

namespace menu
{
class ControlsMenuState : public SelectedMenuState
{
private:
  class ControlsWidget;

  std::unique_ptr<MenuState> m_previous;
  std::shared_ptr<ui::widgets::GridBox> m_allControls{};
  std::vector<std::shared_ptr<ControlsWidget>> m_controls{};

public:
  explicit ControlsMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                             std::unique_ptr<MenuState> previous,
                             const engine::world::World& world);

  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
};
} // namespace menu
