#pragma once

#include "menustate.h"

#include <memory>
#include <utility>

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

class FinishItemAnimationMenuState : public MenuState
{
private:
  std::unique_ptr<MenuState> m_next;

public:
  explicit FinishItemAnimationMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                        std::unique_ptr<MenuState> next)
      : MenuState{ringTransform}
      , m_next{std::move(next)}
  {
  }

  void handleObject(ui::Ui& ui, engine::world::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
};
} // namespace menu
