#pragma once

#include "menustate.h"

#include <cstdint>
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
enum class MenuResult : uint8_t;
struct MenuDisplay;
struct MenuObject;
struct MenuRingTransform;

class DoneMenuState : public MenuState
{
public:
  explicit DoneMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform, MenuResult result)
      : MenuState{ringTransform}
      , m_result{result}
  {
  }

  void handleObject(ui::Ui& ui, engine::world::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;

private:
  MenuResult m_result;
};
} // namespace menu
