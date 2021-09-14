#pragma once

#include "core/angle.h"
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
struct MenuRing;
struct MenuDisplay;
struct MenuObject;
struct MenuRingTransform;

class RotateLeftRightMenuState : public MenuState
{
private:
  static constexpr core::Frame Duration = 24_frame / 2;
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

  void handleObject(ui::Ui& ui, engine::world::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
};
} // namespace menu
