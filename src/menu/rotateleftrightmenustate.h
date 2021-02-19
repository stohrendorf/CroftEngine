#pragma once

#include "core/angle.h"
#include "core/units.h"
#include "menustate.h"

namespace menu
{
struct MenuRing;

class RotateLeftRightMenuState : public MenuState
{
private:
  static constexpr core::Frame Duration = 24_frame;
  size_t m_targetObject{0};
  core::Frame m_duration{Duration};
  // cppcheck-suppress syntaxError
  QS_COMBINE_UNITS(core::Angle, /, core::Frame) m_rotSpeed;
  std::unique_ptr<MenuState> m_prev;

public:
  explicit RotateLeftRightMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                    bool left,
                                    const MenuRing& ring,
                                    std::unique_ptr<MenuState>&& prev);

  void handleObject(engine::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::World& world, MenuDisplay& display) override;
};
} // namespace menu
