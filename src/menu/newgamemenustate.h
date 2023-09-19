#pragma once

#include "listdisplaymenustate.h"

namespace menu
{
class NewGameMenuState : public ListDisplayMenuState
{
public:
  explicit NewGameMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                            std::unique_ptr<MenuState> previous);

  std::unique_ptr<MenuState> onSelected(size_t idx, engine::world::World& world, MenuDisplay& display) override;
  std::unique_ptr<MenuState> onAborted() override;

private:
  std::unique_ptr<MenuState> m_previous;
};
} // namespace menu
