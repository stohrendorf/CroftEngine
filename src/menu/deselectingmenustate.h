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

class DeselectingMenuState final : public MenuState
{
public:
  explicit DeselectingMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform);

  void handleObjectTick(engine::world::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> tick(engine::world::World& world, MenuDisplay& display) override;
  void constructUi(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
};
} // namespace menu
