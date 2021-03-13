#pragma once

#include "core/units.h"
#include "menustate.h"

namespace menu
{
class DeflateRingMenuState : public MenuState
{
private:
  static constexpr core::Frame Duration = 32_frame / 2;
  core::Frame m_duration{Duration};
  std::unique_ptr<MenuState> m_next;
  core::Length m_initialRadius{};
  core::Length m_cameraSpeedY{};
  const core::Length m_target;

public:
  explicit DeflateRingMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                bool down,
                                std::unique_ptr<MenuState> next);

  void begin(engine::world::World& world) override;

  void handleObject(ui::Ui& ui, engine::world::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
};
} // namespace menu
