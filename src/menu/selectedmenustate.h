#pragma once

#include "menustate.h"

namespace ui
{
class Text;
}

namespace menu
{
class SelectedMenuState : public MenuState
{
public:
  explicit SelectedMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform)
      : MenuState{ringTransform}
  {
  }

  void handleObject(ui::Ui& ui, engine::world::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;

private:
  std::unique_ptr<ui::Text> m_itemTitle;
};
} // namespace menu
