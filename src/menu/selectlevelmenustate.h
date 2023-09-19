#pragma once

#include "listdisplaymenustate.h"

#include <vector>

namespace menu
{
class SelectLevelMenuState : public ListDisplayMenuState
{
public:
  explicit SelectLevelMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                std::unique_ptr<MenuState> previous,
                                const engine::world::World& world);

  std::unique_ptr<MenuState> onAborted() override;
  std::unique_ptr<MenuState> onSelected(size_t idx, engine::world::World& world, MenuDisplay& display) override;

private:
  std::unique_ptr<MenuState> m_previous;
  std::vector<size_t> m_indices{};
};
} // namespace menu
