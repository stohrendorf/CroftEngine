#pragma once

#include "menustate.h"

namespace menu
{
class DeselectingMenuState : public MenuState
{
public:
  explicit DeselectingMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform);

  std::unique_ptr<MenuState> onFrame(engine::World& world, MenuDisplay& display) override;
  void handleObject(engine::World& world, MenuDisplay& display, MenuObject& object) override;
};
} // namespace menu
