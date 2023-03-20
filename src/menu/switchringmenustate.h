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
struct MenuDisplay;
struct MenuObject;
struct MenuRingTransform;

class SwitchRingMenuState : public MenuState
{
private:
  static constexpr core::Frame Duration = 24_frame / 2;

  core::Frame m_duration{Duration};
  core::Length m_radiusSpeed{};
  core::Angle m_targetCameraRotX{};
  size_t m_next;
  bool m_down;

public:
  explicit SwitchRingMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform, size_t next, bool down);

  void begin(engine::world::World& world) override;

  void handleObject(ui::Ui& ui, engine::world::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
};
} // namespace menu
