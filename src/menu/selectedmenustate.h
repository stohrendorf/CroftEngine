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

  std::unique_ptr<MenuState> onFrame(engine::World& world, MenuDisplay& display) override;
  void handleObject(engine::World& world, MenuDisplay& display, MenuObject& object) override;
};
} // namespace menu
