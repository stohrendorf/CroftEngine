#pragma once

#include "menustate.h"

namespace menu
{
class ClosePassportMenuState : public MenuState
{
public:
  explicit ClosePassportMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform, MenuObject& passport);

  void handleObject(ui::Ui& ui, engine::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::World& world, MenuDisplay& display) override;
};
} // namespace menu
