#pragma once

#include "core/units.h"
#include "menustate.h"
#include "ui/text.h"

#include <memory>
#include <optional>

namespace engine::world
{
class World;
}

namespace ui
{
class Ui;
}

namespace menu
{
enum class InventoryMode;
struct MenuDisplay;
struct MenuObject;
struct MenuRingTransform;
enum class SaveGamePageMode;

class PassportMenuState : public MenuState
{
private:
  bool m_allowExit;
  SaveGamePageMode m_saveGamePageMode;
  std::optional<int> m_forcePage = std::nullopt;
  std::unique_ptr<ui::Text> m_passportText;

  std::optional<std::unique_ptr<MenuState>> showLoadGamePage(engine::world::World& world, MenuDisplay& display);
  std::optional<std::unique_ptr<MenuState>> showSaveGamePage(engine::world::World& world, MenuDisplay& display);
  void showExitGamePage(engine::world::World& world, MenuDisplay& display, bool returnToTitle);
  void prevPage(const core::Frame& minFrame, MenuObject& passport, engine::world::World& world);
  void nextPage(MenuObject& passport, engine::world::World& world);

public:
  static constexpr int LoadGamePage = 0;
  static constexpr int SaveGamePage = 1;
  static constexpr int ExitGamePage = 2;
  static constexpr core::Frame FramesPerPage = 5_frame;

  explicit PassportMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                             bool allowExit,
                             SaveGamePageMode saveGamePageMode);

  void handleObject(ui::Ui& ui, engine::world::World& world, MenuDisplay& display, MenuObject& object) override;
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
};
} // namespace menu
