#pragma once

#include "core/units.h"
#include "menustate.h"

namespace menu
{
class DeflateRingMenuState : public MenuState
{
private:
  static constexpr core::Frame Duration = 32_frame;
  core::Frame m_duration{Duration};
  std::unique_ptr<MenuState> m_next;
  core::Length m_initialRadius{};
  core::Length m_cameraSpeedY{};

public:
  explicit DeflateRingMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                std::unique_ptr<MenuState> next);

  void begin(engine::World& world) override;

  std::unique_ptr<MenuState> onFrame(engine::World& world, MenuDisplay& display) override;
  void handleObject(engine::World& world, MenuDisplay& display, MenuObject& object) override;
};
} // namespace menu
