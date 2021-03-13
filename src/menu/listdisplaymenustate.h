#pragma once

#include "selectedmenustate.h"

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
  size_t m_selected = 0;
  std::vector<std::tuple<std::unique_ptr<ui::Label>, bool>> m_labels;
  std::unique_ptr<ui::Label> m_heading;
  std::unique_ptr<ui::Label> m_background;

public:
  static constexpr int PixelWidth = 272;
  static constexpr int LineHeight = 18;
  static constexpr int YPos = -80;
  static constexpr size_t PerPage = 10;
  static constexpr int TotalHeight = PerPage * LineHeight + 10;
  static constexpr int YOffset = YPos - TotalHeight;

  explicit ListDisplayMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform, const std::string& heading);
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override final;
  virtual std::unique_ptr<MenuState> onSelected(size_t idx, engine::world::World& world, MenuDisplay& display) = 0;
  virtual std::unique_ptr<MenuState> onAborted() = 0;

  size_t addEntry(const std::string& label);

  void setActive(size_t idx, bool active);

  const auto& getLabel(size_t idx) const
  {
    return std::get<0>(m_labels.at(idx));
  }
};
} // namespace menu
