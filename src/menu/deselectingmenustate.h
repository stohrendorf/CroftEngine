#pragma once

#include "menustate.h"

namespace menu
{
class DeselectingMenuState : public MenuState
{
public:
  explicit DeselectingMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform);

  void handleObject(ui::Ui& ui, engine::world::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
};
} // namespace menu
