#pragma once

#include "menustate.h"

namespace engine
{
enum class TR1ItemId;
}

namespace menu
{
class SetItemTypeMenuState : public MenuState
{
private:
  const engine::TR1ItemId m_type;
  std::unique_ptr<MenuState> m_next;

public:
  explicit SetItemTypeMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                const engine::TR1ItemId type,
                                std::unique_ptr<MenuState> next)
      : MenuState{ringTransform}
      , m_type{type}
      , m_next{std::move(next)}
  {
  }

  std::unique_ptr<MenuState> onFrame(gl::Image<gl::SRGBA8>& img, engine::World& world, MenuDisplay& display) override;
  void handleObject(engine::World& world, MenuDisplay& display, MenuObject& object) override;
};
} // namespace menu
