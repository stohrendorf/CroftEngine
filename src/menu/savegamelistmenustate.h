#pragma once

#include "listdisplaymenustate.h"

#include <string>
#include <vector>

namespace ui
{
struct Label;
}

namespace menu
{
class SavegameListMenuState : public ListDisplayMenuState
{
private:
  static constexpr size_t TotalSlots = 20;

  std::unique_ptr<MenuState> m_previous;
  std::vector<bool> m_hasSavegame;
  const bool m_loading;

public:
  explicit SavegameListMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                 std::unique_ptr<MenuState> previous,
                                 const std::string& heading,
                                 const engine::world::World& world,
                                 bool loading);

  std::unique_ptr<MenuState> onSelected(size_t idx, engine::world::World& world, MenuDisplay& display) override;
  std::unique_ptr<MenuState> onAborted() override;
};
} // namespace menu
