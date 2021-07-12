#pragma once

#include "hid/delayedkey.h"
#include "selectedmenustate.h"
#include "ui/widgets/groupbox.h"

#include <chrono>
#include <functional>
#include <optional>

namespace ui::widgets
{
class GridBox;
class SelectionBox;
}

namespace engine::world
{
class World;
}

namespace menu
{
class ControlsWidget;

class ControlsMenuState : public SelectedMenuState
{
private:
  enum class Mode
  {
    Display,
    ChangeKey,
    ConfirmApply,
    Apply,
    Discard,
    Error,
  };

  std::unique_ptr<MenuState> m_previous;
  std::shared_ptr<ControlsWidget> m_controls{};
  std::shared_ptr<ui::widgets::GridBox> m_layout{};
  size_t m_editingIndex = 0;
  std::vector<engine::NamedInputMappingConfig> m_editing;

  hid::DelayedKey m_resetKey;
  hid::DelayedKey m_deleteKey;

  std::shared_ptr<ui::widgets::SelectionBox> m_confirm{};
  std::shared_ptr<ui::widgets::SelectionBox> m_error{};

  Mode m_mode = Mode::Display;

  void handleDisplayInput(engine::world::World& world);
  void handleChangeKeyInput(engine::world::World& world);
  void handleConfirmInput(engine::world::World& world);
  void handleErrorInput(engine::world::World& world);

public:
  explicit ControlsMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                             std::unique_ptr<MenuState> previous,
                             const engine::world::World& world);

  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
};
} // namespace menu
