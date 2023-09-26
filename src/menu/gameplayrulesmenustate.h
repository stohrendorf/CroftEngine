#pragma once

#include "engine/gameplayrules.h"
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
class GroupBox;
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

class GameplayRulesMenuState : public SelectedMenuState
{
private:
  std::shared_ptr<ui::widgets::CheckListBox> m_listBox{};
  std::vector<std::shared_ptr<ui::widgets::TextBox>> m_descriptions{};
  std::unique_ptr<MenuState> m_previous;
  std::unique_ptr<ui::widgets::GroupBox> m_container;
  engine::GameplayRules m_rules;

public:
  explicit GameplayRulesMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                  std::unique_ptr<MenuState> previous);

  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
  ~GameplayRulesMenuState() override;
};
} // namespace menu
