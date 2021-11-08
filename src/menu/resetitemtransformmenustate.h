#pragma once

#include "core/magic.h"
#include "core/units.h"
#include "menustate.h"
#include "qs/qs.h"

#include <memory>
#include <utility>

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

class ResetItemTransformMenuState : public MenuState
{
private:
  static constexpr core::RenderFrame Duration = toAnimUnit(16_frame / 2);
  core::RenderFrame m_duration{Duration};
  std::unique_ptr<MenuState> m_next;

public:
  explicit ResetItemTransformMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                       std::unique_ptr<MenuState>&& next)
      : MenuState{ringTransform}
      , m_next{std::move(next)}
  {
  }

  void handleObject(ui::Ui& ui, engine::world::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
  void begin(engine::world::World& world) override;
};
} // namespace menu
