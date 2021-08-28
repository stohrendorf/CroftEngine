#pragma once

#include "listdisplaymenustate.h"

#include <string>
#include <vector>

namespace ui::widgets
{
class MessageBox;
}

namespace menu
{
class SavegameListMenuState : public ListDisplayMenuState
{
private:
  std::unique_ptr<MenuState> m_previous;
  std::vector<bool> m_hasSavegame;
  const bool m_loading;
  std::shared_ptr<ui::widgets::MessageBox> m_confirmOverwrite;

public:
  explicit SavegameListMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                 std::unique_ptr<MenuState> previous,
                                 const std::string& heading,
                                 const engine::world::World& world,
                                 bool loading);

  std::unique_ptr<MenuState> onSelected(size_t idx, engine::world::World& world, MenuDisplay& display) override;
  std::unique_ptr<MenuState> onAborted() override;
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
};
} // namespace menu
