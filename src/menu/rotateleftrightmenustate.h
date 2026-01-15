#pragma once

#include "core/units.h"
#include "menustate.h"
#include "qs/qs.h"

#include <cstddef>
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
struct MenuRing;
struct MenuDisplay;
struct MenuObject;
struct MenuRingTransform;

class RotateLeftRightMenuState final : public MenuState
{
  static constexpr core::Tick Duration = 24_tick / 2;
  size_t m_targetObject{0};
  core::Tick m_duration{Duration};
  // cppcheck-suppress syntaxError
  core::RotationSpeed m_rotSpeed;
  std::unique_ptr<MenuState> m_prev;

public:
  explicit RotateLeftRightMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                    bool left,
                                    const MenuRing& ring,
                                    std::unique_ptr<MenuState>&& prev);

  void handleObjectTick(engine::world::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> tick(engine::world::World& world, MenuDisplay& display) override;
  void constructUi(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
};
} // namespace menu
