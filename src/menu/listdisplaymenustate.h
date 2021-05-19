#pragma once

#include "selectedmenustate.h"
#include "ui/widgets/groupbox.h"

#include <string>
#include <vector>

namespace ui
{
struct Label;
}

namespace ui::widgets
{
class ListBox;
}

namespace menu
{
class ListDisplayMenuState : public SelectedMenuState
{
private:
  static constexpr int Padding = 10;

  glm::ivec2 m_position;
  std::shared_ptr<ui::widgets::ListBox> m_listBox;
  ui::widgets::GroupBox m_groupBox;

public:
  explicit ListDisplayMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                const std::string& heading,
                                size_t pageSize,
                                int width,
                                const glm::ivec2& position);
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) final;
  virtual std::unique_ptr<MenuState> onSelected(size_t idx, engine::world::World& world, MenuDisplay& display) = 0;
  virtual std::unique_ptr<MenuState> onAborted() = 0;

  size_t addEntry(const std::shared_ptr<ui::widgets::Widget>& widget);
  void setPosition(const glm::ivec2& position);

  [[nodiscard]] const auto& getListBox() const
  {
    return m_listBox;
  }
};
} // namespace menu
