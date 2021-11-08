#pragma once

#include "core/angle.h"
#include "core/magic.h"
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

class DeflateRingMenuState : public MenuState
{
private:
  static constexpr core::RenderFrame Duration = toAnimUnit(32_frame / 2);
  core::RenderFrame m_duration{Duration};
  std::unique_ptr<MenuState> m_next;
  core::Length m_initialRadius{};
  core::Angle m_initialCameraRotX{};
  core::Length m_cameraSpeedY{};
  const core::Length m_targetPosY;
  const core::Length m_targetPosZ;
  const core::Angle m_targetRotX;

public:
  enum class Direction
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
