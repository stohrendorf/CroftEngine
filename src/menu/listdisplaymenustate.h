#pragma once

#include "selectedmenustate.h"
#include "widgets/listbox.h"

#include <string>
#include <vector>

namespace ui
{
struct Label;
}

namespace menu
{
class ListDisplayMenuState : public SelectedMenuState
{
private:
  widgets::ListBox m_listBox;
  std::unique_ptr<ui::Label> m_heading;
  std::unique_ptr<ui::Label> m_background;

public:
  explicit ListDisplayMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform, const std::string& heading);
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) final;
  virtual std::unique_ptr<MenuState> onSelected(size_t idx, engine::world::World& world, MenuDisplay& display) = 0;
  virtual std::unique_ptr<MenuState> onAborted() = 0;

  auto addEntry(const std::string& label)
  {
    return m_listBox.addEntry(label);
  }

  void setActive(size_t idx, bool active)
  {
    m_listBox.setActive(idx, active);
  }
};
} // namespace menu
