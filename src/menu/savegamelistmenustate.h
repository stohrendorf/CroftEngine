#pragma once

#include "listdisplaymenustate.h"
#include "menustate.h"

#include <cstddef>
#include <gslu.h>
#include <memory>
#include <string>
#include <vector>

namespace engine::world
{
class World;
}

namespace ui
{
class Ui;
}

namespace ui::widgets
{
class MessageBox;
}

namespace menu
{
struct MenuDisplay;
struct MenuRingTransform;

class SavegameListMenuState : public ListDisplayMenuState
{
private:
  class SavegameEntry;

  std::unique_ptr<MenuState> m_previous;
  std::vector<bool> m_hasSavegame;
  std::vector<gslu::nn_shared<SavegameEntry>> m_entries;
  const bool m_loading;
  std::shared_ptr<ui::widgets::MessageBox> m_confirmOverwrite;
  enum class Order
  {
    Slot,
    DateAsc,
    DateDesc
  };
  Order m_order = Order::Slot;

  void sortEntries();

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
