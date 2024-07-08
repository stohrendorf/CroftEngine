#pragma once

#include "menustate.h"

#include <cstddef>
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

class RequestLevelMenuState : public MenuState
{
public:
  explicit RequestLevelMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform, size_t sequenceIndex)
      : MenuState{ringTransform}
      , m_sequenceIndex{sequenceIndex}
  {
  }

  void handleObject(ui::Ui& ui, engine::world::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;

private:
  size_t m_sequenceIndex;
};
} // namespace menu
