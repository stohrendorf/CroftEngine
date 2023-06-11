#pragma once

#include "listdisplaymenustate.h"
#include "menustate.h"

#include <chrono>
#include <cstddef>
#include <gslu.h>
#include <map>
#include <memory>
#include <string>
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
class MessageBox;
}

namespace engine
{
struct SavegameInfo;
}

namespace menu
{
struct MenuDisplay;
struct MenuRingTransform;

class SavegameListMenuState : public ListDisplayMenuState
{
private:
  class SavegameEntry;

  std::unique_ptr<MenuState> m_previous;
  bool m_hasQuicksave = false;
  std::vector<bool> m_hasSavegame;
  std::vector<gslu::nn_shared<SavegameEntry>> m_entries;
  bool m_loading;
  std::shared_ptr<ui::widgets::MessageBox> m_overwriteConfirmation;
  std::shared_ptr<ui::widgets::MessageBox> m_cleanupConfirmation;
  enum class Ordering
  {
    Slot,
    DateAsc,
    DateDesc
  };
  Ordering m_ordering = Ordering::Slot;
  std::chrono::steady_clock::time_point m_confirmOverwritePressedSince{};

  class CleanupWidget;
  std::shared_ptr<CleanupWidget> m_cleanupWidget;
  std::map<size_t, engine::SavegameInfo> m_savegameInfos;

  void sortEntries();
  [[nodiscard]] std::unique_ptr<MenuState>
    onDefaultFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display);
  [[nodiscard]] std::unique_ptr<MenuState>
    onConfirmOverwriteFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display);
  [[nodiscard]] std::unique_ptr<MenuState>
    onCleanupFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display);
  void selectMostRecentSlot();
  void selectFirstFreeOrOldestSlot();

  void updateSavegameInfos(const engine::world::World& world);
  void initCleanupConfirmation();
  void cleanupSaves(const engine::world::World& world);

public:
  explicit SavegameListMenuState(const std::shared_ptr<MenuRingTransform>& slot,
                                 std::unique_ptr<MenuState> previous,
                                 const std::string& heading,
                                 const engine::world::World& world,
                                 bool loading);

  std::unique_ptr<MenuState> onSelected(size_t idx, engine::world::World& world, MenuDisplay& display) override;
  std::unique_ptr<MenuState> onAborted() override;
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
};
} // namespace menu
