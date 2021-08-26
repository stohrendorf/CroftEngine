#pragma once

#include "selectedmenustate.h"
#include "ui/widgets/groupbox.h"

#include <string>
#include <vector>

namespace ui::widgets
{
class ListBox;
}

namespace menu
{
class ListDisplayMenuState : public SelectedMenuState
{
private:
  std::shared_ptr<ui::widgets::ListBox> m_listBox;
  ui::widgets::GroupBox m_groupBox;

public:
  explicit ListDisplayMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                const std::string& heading,
                                size_t pageSize);
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display);
  virtual std::unique_ptr<MenuState> onSelected(size_t idx, engine::world::World& world, MenuDisplay& display) = 0;
  virtual std::unique_ptr<MenuState> onAborted() = 0;

  size_t append(const std::shared_ptr<ui::widgets::Widget>& widget);

  [[nodiscard]] const auto& getListBox() const
  {
    return m_listBox;
  }

  void draw(ui::Ui& ui, engine::world::World& world, MenuDisplay& display);
};
} // namespace menu
