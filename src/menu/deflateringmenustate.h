#pragma once

#include "core/units.h"
#include "menustate.h"
#include "qs/qs.h"

#include <cstdint>
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

class DeflateRingMenuState final : public MenuState
{
  static constexpr core::Tick Duration = 32_tick / 2;
  core::Tick m_duration{Duration};
  std::unique_ptr<MenuState> m_next;
  core::Length m_initialRadius;
  core::Angle m_initialCameraRotX;
  core::Length m_cameraSpeedY;
  core::Length m_targetPosY;
  core::Length m_targetPosZ;
  core::Angle m_targetRotX;

public:
  enum class Direction : uint8_t
  {
    Up,
    Down,
    Backpack
  };

  explicit DeflateRingMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                Direction direction,
                                std::unique_ptr<MenuState> next);

  void begin(engine::world::World& world) override;

  void handleObjectTick(engine::world::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> tick(engine::world::World& world, MenuDisplay& display) override;
  void constructUi(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
};
} // namespace menu
