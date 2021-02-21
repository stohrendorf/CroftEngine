#pragma once

#include "selectedmenustate.h"

#include <functional>

namespace ui
{
struct Label;
}

namespace engine
{
class Engine;
}

namespace menu
{
class RenderSettingsMenuState : public SelectedMenuState
{
private:
  static constexpr int16_t LineHeight = 18;
  static constexpr int16_t YPos = -80;
  static constexpr int16_t TotalHeight = 10 * LineHeight + 10;
  static constexpr int16_t YOffset = YPos - TotalHeight;

  std::unique_ptr<MenuState> m_previous;
  size_t m_selected = 0;
  std::vector<std::tuple<std::shared_ptr<ui::Label>, std::function<bool()>, std::function<void()>>> m_labels;
  std::unique_ptr<ui::Label> m_background;

public:
  static constexpr int16_t PixelWidth = 272;

  explicit RenderSettingsMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                   std::unique_ptr<MenuState> previous,
                                   engine::Engine& engine);
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::World& world, MenuDisplay& display) override;
};
} // namespace menu
