#pragma once

#include "core/angle.h"
#include "core/units.h"
#include "menustate.h"

namespace menu
{
class SwitchRingMenuState : public MenuState
{
private:
  static constexpr core::Frame Duration = 24_frame;

  core::Frame m_duration{Duration};
  core::Length m_radiusSpeed{};
  core::Angle m_targetCameraRotX{};
  size_t m_next;
  const bool m_down;

public:
  explicit SwitchRingMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform, size_t next, bool down);

  void begin(engine::World& world) override;

  void handleObject(ui::Ui& ui, engine::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::World& world, MenuDisplay& display) override;
};
} // namespace menu
