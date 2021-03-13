#pragma once

#include "menustate.h"

namespace menu
{
enum class MenuResult;

class RequestLoadMenuState : public MenuState
{
public:
  explicit RequestLoadMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform, size_t index)
      : MenuState{ringTransform}
      , m_index{index}
  {
  }

  void handleObject(ui::Ui& ui, engine::world::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;

private:
  const size_t m_index;
};
} // namespace menu
