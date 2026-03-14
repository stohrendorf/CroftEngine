#pragma once

#include "core/units.h"
#include "menustate.h"
#include "qs/qs.h"

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

class ResetItemTransformMenuState final : public MenuState
{
  static constexpr core::Tick Duration = 16_tick / 2;
  core::Tick m_duration{Duration};
  std::unique_ptr<MenuState> m_next;

public:
  explicit ResetItemTransformMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                       std::unique_ptr<MenuState>&& next)
      : MenuState{ringTransform}
      , m_next{std::move(next)}
  {
  }

  void handleObjectTick(engine::world::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> tick(engine::world::World& world, MenuDisplay& display) override;
  void constructUi(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
  void begin(engine::world::World& world) override;
};
} // namespace menu
