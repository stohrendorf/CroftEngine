#pragma once

#include "menustate.h"

namespace menu
{
class FinishItemAnimationMenuState : public MenuState
{
private:
  std::unique_ptr<MenuState> m_next;

public:
  explicit FinishItemAnimationMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                        std::unique_ptr<MenuState> next)
      : MenuState{ringTransform}
      , m_next{std::move(next)}
  {
  }

  std::unique_ptr<MenuState> onFrame(gl::Image<gl::SRGBA8>& img, engine::World& world, MenuDisplay& display) override;
  void handleObject(engine::World& world, MenuDisplay& display, MenuObject& object) override;
};
} // namespace menu
