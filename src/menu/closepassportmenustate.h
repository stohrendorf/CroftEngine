#pragma once

#include "menustate.h"

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

class ClosePassportMenuState : public MenuState
{
private:
  std::unique_ptr<MenuState> m_next;

public:
  explicit ClosePassportMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                  MenuObject& passport,
                                  std::unique_ptr<MenuState>&& next);

  void handleObject(ui::Ui& ui, engine::world::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
};
} // namespace menu
