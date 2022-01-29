#pragma once

#include "menustate.h"

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
enum class MenuResult;
struct MenuDisplay;
struct MenuObject;
struct MenuRingTransform;

class RequestLoadMenuState : public MenuState
{
public:
  explicit RequestLoadMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform, size_t slot)
      : MenuState{ringTransform}
      , m_slot{slot}
  {
  }

  void handleObject(ui::Ui& ui, engine::world::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;

private:
  const size_t m_slot;
};
} // namespace menu
