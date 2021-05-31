#pragma once

#include "menuringtransform.h"
#include "ui/label.h"

namespace ui
{
class Ui;
}

namespace menu
{
enum class InventoryMode
{
  GameMode,
  TitleMode,
  SaveMode,
  LoadMode,
  DeathMode
};

struct MenuObject;
struct MenuRing;
class MenuState;

enum class MenuResult
{
  None,
  Closed,
  ExitToTitle,
  ExitGame,
  NewGame,
  LaraHome,
  RequestLoad
};

struct MenuDisplay
{
  explicit MenuDisplay(InventoryMode mode, engine::world::World& world);
  ~MenuDisplay();

  const InventoryMode mode;
  std::optional<engine::TR1ItemId> inventoryChosen{};
  float streamVolume;
  bool allowMenuClose = true;
  bool allowSave = true;

  std::shared_ptr<MenuRingTransform> ringTransform = std::make_shared<MenuRingTransform>();
  std::unique_ptr<MenuState> m_currentState;

  void display(ui::Ui& ui, engine::world::World& world);
  MenuResult result = MenuResult::None;
  std::optional<size_t> requestLoad;

  std::vector<gsl::not_null<std::unique_ptr<MenuRing>>> rings;
  size_t currentRingIndex = 0;
  bool passOpen = false;
  bool doOptions(engine::world::World& world, MenuObject& object);
  void updateMenuObjectDescription(ui::Ui& ui, engine::world::World& world, const MenuObject& object);

  [[nodiscard]] MenuRing& getCurrentRing()
  {
    return *rings.at(currentRingIndex);
  }

  [[nodiscard]] const MenuRing& getCurrentRing() const
  {
    return *rings.at(currentRingIndex);
  }

private:
  [[nodiscard]] static std::vector<MenuObject> getOptionRingObjects(const engine::world::World& world,
                                                                    bool withHomePolaroid);
  [[nodiscard]] static std::vector<MenuObject> getMainRingObjects(const engine::world::World& world);
  [[nodiscard]] static std::vector<MenuObject> getKeysRingObjects(const engine::world::World& world);

  ui::Text m_upArrow;
  ui::Text m_downArrow;
};
} // namespace menu
