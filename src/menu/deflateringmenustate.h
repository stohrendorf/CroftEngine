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

class DeflateRingMenuState : public MenuState
{
private:
  static constexpr core::Frame Duration = 32_frame / 2;
  core::Frame m_duration{Duration};
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

  void handleObject(ui::Ui& ui, engine::world::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
};
} // namespace menu
