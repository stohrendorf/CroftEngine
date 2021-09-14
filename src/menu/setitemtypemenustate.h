#pragma once

#include "menustate.h"

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

namespace engine
{
enum class TR1ItemId;
}

namespace menu
{
struct MenuDisplay;
struct MenuObject;
struct MenuRingTransform;

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

  void handleObject(ui::Ui& ui, engine::world::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
};
} // namespace menu
