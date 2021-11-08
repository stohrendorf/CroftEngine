#pragma once

#include "core/magic.h"
#include "core/units.h"
#include "menustate.h"
#include "qs/qs.h"

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
struct MenuDisplay;
struct MenuObject;
struct MenuRingTransform;

class ApplyItemTransformMenuState : public MenuState
{
private:
  static constexpr core::RenderFrame Duration = toAnimUnit(16_frame / 2);
  core::RenderFrame m_duration{0_rframe};

public:
  explicit ApplyItemTransformMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform)
      : MenuState{ringTransform}
  {
  }

  void handleObject(ui::Ui& ui, engine::world::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
};
} // namespace menu
