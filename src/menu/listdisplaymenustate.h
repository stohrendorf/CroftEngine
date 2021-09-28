#pragma once

#include "selectedmenustate.h"
#include "ui/widgets/groupbox.h"

#include <cstddef>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <string>

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
class Widget;
class ListBox;
} // namespace ui::widgets

namespace menu
{
class MenuState;
struct MenuDisplay;
struct MenuRingTransform;

class ListDisplayMenuState : public SelectedMenuState
{
private:
  gsl::not_null<std::shared_ptr<ui::widgets::ListBox>> m_listBox;
  ui::widgets::GroupBox m_groupBox;

public:
  explicit ListDisplayMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                const std::string& heading,
                                size_t pageSize);
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
  virtual std::unique_ptr<MenuState> onSelected(size_t idx, engine::world::World& world, MenuDisplay& display) = 0;
  virtual std::unique_ptr<MenuState> onAborted() = 0;

  size_t append(const gsl::not_null<std::shared_ptr<ui::widgets::Widget>>& widget);

  [[nodiscard]] const auto& getListBox() const
  {
    return m_listBox;
  }

  void draw(ui::Ui& ui, engine::world::World& world, MenuDisplay& display);
};
} // namespace menu
