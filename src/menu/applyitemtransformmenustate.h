#pragma once

#include "core/units.h"
#include "menustate.h"

namespace menu
{
class ApplyItemTransformMenuState : public MenuState
{
private:
  static constexpr core::Frame Duration = 16_frame / 2;
  core::Frame m_duration{0_frame};

public:
  explicit ApplyItemTransformMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform)
      : MenuState{ringTransform}
  {
  }

  void handleObject(ui::Ui& ui, engine::world::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
};
} // namespace menu
