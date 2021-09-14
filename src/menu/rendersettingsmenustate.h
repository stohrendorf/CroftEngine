#pragma once

#include "menustate.h"
#include "selectedmenustate.h"

#include <cstddef>
#include <memory>
#include <vector>

namespace engine::world
{
class World;
}

namespace ui
{
class Ui;
}

namespace ui::widgets
{
class Checkbox;
}

namespace engine
{
class Engine;
}

namespace menu
{
struct MenuDisplay;
struct MenuRingTransform;

class RenderSettingsMenuState : public SelectedMenuState
{
private:
  class CheckListBox;
  std::vector<std::shared_ptr<CheckListBox>> m_listBoxes{};
  size_t m_currentListBox = 0;
  std::unique_ptr<MenuState> m_previous;
  std::shared_ptr<ui::widgets::Checkbox> m_anisotropyCheckbox;

public:
  explicit RenderSettingsMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                   std::unique_ptr<MenuState> previous,
                                   engine::Engine& engine);

  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
};
} // namespace menu
