#pragma once

#include "menustate.h"

#include <memory>

namespace engine::world
{
class World;
}

namespace ui
{
class Ui;
class Text;
} // namespace ui

namespace menu
{
struct MenuDisplay;
struct MenuObject;
struct MenuRingTransform;

class SelectedMenuState : public MenuState
{
public:
  explicit SelectedMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform);
  ~SelectedMenuState() override;

  void handleObject(ui::Ui& ui, engine::world::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;

private:
  std::unique_ptr<ui::Text> m_itemTitle;
};
} // namespace menu
