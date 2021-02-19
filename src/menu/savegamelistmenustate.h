#pragma once

#include "menustate.h"

#include <string>
#include <vector>

namespace ui
{
struct Label;
}

namespace menu
{
class SavegameListMenuState : public MenuState
{
private:
  static constexpr int16_t PixelWidth = 272;
  static constexpr int16_t LineHeight = 18;
  static constexpr int16_t YPos = -80;
  static constexpr int16_t PerPage = 10;
  static constexpr int16_t TotalHeight = PerPage * LineHeight + 10;
  static constexpr int16_t YOffset = YPos - TotalHeight;
  static constexpr size_t TotalSlots = 20;

  std::unique_ptr<MenuState> m_previous;
  size_t m_selected = 0;
  std::vector<std::unique_ptr<ui::Label>> m_labels;
  std::vector<bool> m_hasSavegame;
  std::unique_ptr<ui::Label> m_heading;
  std::unique_ptr<ui::Label> m_background;
  const bool m_loading;

public:
  explicit SavegameListMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                 std::unique_ptr<MenuState> previous,
                                 const std::string& heading,
                                 const engine::World& world,
                                 bool loading);
  void handleObject(engine::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(engine::World& world, MenuDisplay& display) override;
};
} // namespace menu
