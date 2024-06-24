#pragma once

#include "selectedmenustate.h"

#include <gslu.h>
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
class TabBox;
class TextBox;
class CheckListBox;
} // namespace ui::widgets

namespace engine
{
class Engine;
}

namespace menu
{
struct MenuDisplay;
struct MenuRingTransform;
class MenuState;

class SettingsMenuState : public SelectedMenuState
{
private:
  std::vector<std::shared_ptr<ui::widgets::CheckListBox>> m_listBoxes{};
  std::vector<std::vector<std::shared_ptr<ui::widgets::TextBox>>> m_descriptions{};
  std::unique_ptr<MenuState> m_previous;
  gslu::nn_unique<ui::widgets::TabBox> m_tabs;

public:
  explicit SettingsMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                             std::unique_ptr<MenuState> previous,
                             engine::Engine& engine);

  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
  ~SettingsMenuState() override;
};
} // namespace menu
