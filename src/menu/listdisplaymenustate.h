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
  static constexpr int Padding = 10;

  glm::ivec2 m_position;
  widgets::ListBox m_listBox;
  std::unique_ptr<ui::Label> m_heading;
  std::unique_ptr<ui::Label> m_background;

public:
  explicit ListDisplayMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                const std::string& heading,
                                size_t pageSize,
                                int width,
                                const glm::ivec2& position);
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) final;
  virtual std::unique_ptr<MenuState> onSelected(size_t idx, engine::world::World& world, MenuDisplay& display) = 0;
  virtual std::unique_ptr<MenuState> onAborted() = 0;

  size_t addEntry(const std::shared_ptr<widgets::Widget>& widget);
  void setPosition(const glm::ivec2& position);

  const auto& getListBox() const
  {
    return m_listBox;
  }
};
} // namespace menu
