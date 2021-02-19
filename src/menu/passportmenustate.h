#pragma once

#include "core/units.h"
#include "menustate.h"

#include <optional>

namespace ui
{
struct Label;
}

namespace menu
{
enum class InventoryMode;

class PassportMenuState : public MenuState
{
private:
  static constexpr int LoadGamePage = 0;
  static constexpr int SaveGamePage = 1;
  static constexpr int ExitGamePage = 2;
  static constexpr core::Frame FramesPerPage = 5_frame;

  const bool m_allowExit;
  const bool m_allowSave;
  std::optional<int> m_forcePage;
  std::unique_ptr<ui::Label> m_passportText;

  std::unique_ptr<MenuState> close(MenuDisplay& display, int page, MenuObject& passport);

  std::optional<std::unique_ptr<MenuState>> showLoadGamePage(engine::World& world, MenuDisplay& display);
  std::optional<std::unique_ptr<MenuState>> showSaveGamePage(engine::World& world, MenuDisplay& display, bool isInGame);
  void showExitGamePage(engine::World& world, MenuDisplay& display, bool isInGame);
  void prevPage(const core::Frame& minFrame, MenuObject& passport, engine::World& world);
  void nextPage(MenuObject& passport, engine::World& world);

public:
  explicit PassportMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                             InventoryMode mode,
                             bool allowSave);

  void handleObject(engine::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::World& world, MenuDisplay& display) override;
};
} // namespace menu
