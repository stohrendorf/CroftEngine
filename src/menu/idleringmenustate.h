#pragma once

#include "menustate.h"

namespace menu
{
class IdleRingMenuState : public MenuState
{
private:
  bool m_autoSelect;

public:
  explicit IdleRingMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform, bool autoSelect)
      : MenuState{ringTransform}
      , m_autoSelect{autoSelect}
  {
  }

  void handleObject(ui::Ui& ui, engine::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::World& world, MenuDisplay& display) override;
};
} // namespace menu
