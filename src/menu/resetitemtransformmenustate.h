#pragma once

#include "core/units.h"
#include "menustate.h"

namespace menu
{
class ResetItemTransformMenuState : public MenuState
{
private:
  static constexpr core::Frame Duration = 16_frame;
  core::Frame m_duration{Duration};
  std::unique_ptr<MenuState> m_next;

public:
  explicit ResetItemTransformMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                       std::unique_ptr<MenuState>&& next)
      : MenuState{ringTransform}
      , m_next{std::move(next)}
  {
  }

  void handleObject(engine::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(engine::World& world, MenuDisplay& display) override;
  void begin(engine::World& world) override;
};
} // namespace menu
