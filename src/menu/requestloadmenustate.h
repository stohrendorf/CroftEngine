#pragma once

#include "menustate.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

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

class RequestLoadMenuState : public MenuState
{
public:
  explicit RequestLoadMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                const std::optional<size_t>& slot)
      : MenuState{ringTransform}
      , m_slot{slot}
  {
  }

  void handleObject(ui::Ui& ui, engine::world::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;

private:
  std::optional<size_t> m_slot;
};
} // namespace menu
