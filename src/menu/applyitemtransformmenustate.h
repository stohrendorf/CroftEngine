#pragma once

#include "core/units.h"
#include "menustate.h"
#include "qs/qs.h"

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

class ApplyItemTransformMenuState : public MenuState
{
  static constexpr core::Tick Duration = 16_tick / 2;
  core::Tick m_duration{0_tick};

public:
  explicit ApplyItemTransformMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform)
      : MenuState{ringTransform}
  {
  }

  void handleObjectTick(engine::world::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> tick(engine::world::World& world, MenuDisplay& display) override;
  void constructUi(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
};
} // namespace menu
