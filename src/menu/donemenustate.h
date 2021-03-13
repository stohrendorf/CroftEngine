#pragma once

#include "menustate.h"

namespace menu
{
enum class MenuResult;

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
  const MenuResult m_result;
};
} // namespace menu
