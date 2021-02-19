#pragma once

#include "menustate.h"

namespace menu
{
class SelectedMenuState : public MenuState
{
public:
  explicit SelectedMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform)
      : MenuState{ringTransform}
  {
  }

  void handleObject(engine::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::World& world, MenuDisplay& display) override;
};
} // namespace menu
