#pragma once

#include "core/angle.h"
#include "core/units.h"
#include "menustate.h"

namespace menu
{
class InflateRingMenuState : public MenuState
{
private:
  static constexpr core::Frame Duration = 24_frame / 2;
  core::Frame m_duration{Duration};
  core::Angle m_initialCameraRotX{};
  core::Length m_radiusSpeed{};
  core::Length m_cameraSpeedY{};
  const core::Length m_targetPosZ;

public:
  explicit InflateRingMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform, bool fromBackpack);

  void begin(engine::world::World& world) override;
  void handleObject(ui::Ui& ui, engine::world::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
};
} // namespace menu
