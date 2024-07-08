#pragma once

#include "engine/engineconfig.h"
#include "hid/delayedkey.h"
#include "menustate.h"
#include "selectedmenustate.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace ui
{
class Ui;
}

namespace ui::widgets
{
class GridBox;
class SelectionBox;
} // namespace ui::widgets

namespace engine::world
{
class World;
}

namespace menu
{
class ControlsWidget;
struct MenuDisplay;
struct MenuRingTransform;

class ControlsMenuState : public SelectedMenuState
{
private:
  enum class Mode : uint8_t
  {
    Display,
    ChangeKey,
    ConfirmApply,
    Apply,
    Discard,
    Error,
  };

  std::unique_ptr<MenuState> m_previous;
  size_t m_editingIndex = 0;
  std::vector<engine::NamedInputMappingConfig> m_editing;
  std::shared_ptr<ui::widgets::GridBox> m_layout;
  std::shared_ptr<ControlsWidget> m_controls;

  hid::DelayedKey m_resetModernKey;
  hid::DelayedKey m_resetClassicKey;
  hid::DelayedKey m_deleteKey;

  std::shared_ptr<ui::widgets::SelectionBox> m_confirm;
  std::shared_ptr<ui::widgets::SelectionBox> m_error;

  Mode m_mode = Mode::Display;

  void handleDisplayInput(engine::world::World& world);
  void handleChangeKeyInput(engine::world::World& newInput);
  void handleConfirmInput(engine::world::World& world);
  void handleErrorInput(engine::world::World& world);

public:
  explicit ControlsMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                             std::unique_ptr<MenuState> previous,
                             const engine::world::World& world);

  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
};
} // namespace menu
