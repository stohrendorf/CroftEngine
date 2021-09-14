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
}

namespace menu
{
struct MenuDisplay;
struct MenuObject;
struct MenuRingTransform;

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

  void handleObject(ui::Ui& ui, engine::world::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
};
} // namespace menu
