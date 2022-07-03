#pragma once

#include "menustate.h"
#include "selectedmenustate.h"

#include <cstddef>
#include <gslu.h>
#include <memory>
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
class Checkbox;
class Tab;
class TabBox;
template<typename T>
class ValueSelector;
} // namespace ui::widgets

namespace engine
{
class Engine;
}

namespace menu
{
struct MenuDisplay;
struct MenuRingTransform;

class RenderSettingsMenuState : public SelectedMenuState
{
private:
  class CheckListBox;
  std::vector<std::shared_ptr<CheckListBox>> m_listBoxes{};
  std::unique_ptr<MenuState> m_previous;
  std::shared_ptr<ui::widgets::ValueSelector<uint32_t>> m_anisotropySelector;
  std::shared_ptr<ui::widgets::ValueSelector<uint8_t>> m_uiScaleSelector;
  std::shared_ptr<ui::widgets::ValueSelector<uint8_t>> m_renderResolutionDivisorSelector;
  std::shared_ptr<ui::widgets::ValueSelector<uint8_t>> m_dustDensitySelector;
  std::shared_ptr<ui::widgets::ValueSelector<uint8_t>> m_crtVersionSelector;
  gslu::nn_unique<ui::widgets::TabBox> m_tabs;

public:
  explicit RenderSettingsMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                   std::unique_ptr<MenuState> previous,
                                   engine::Engine& engine);

  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
  ~RenderSettingsMenuState() override;
};
} // namespace menu
