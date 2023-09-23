#include "savegamelistmenustate.h"

#include "closepassportmenustate.h"
#include "core/i18n.h"
#include "core/magic.h"
#include "deflateringmenustate.h"
#include "donemenustate.h"
#include "engine/engine.h"
#include "engine/presenter.h"
#include "engine/script/reflection.h"
#include "engine/world/world.h"
#include "hid/actions.h"
#include "hid/inputhandler.h"
#include "listdisplaymenustate.h"
#include "menudisplay.h"
#include "menuring.h"
#include "menustate.h"
#include "requestloadmenustate.h"
#include "savegamecleanup.h"
#include "ui/ui.h"
#include "ui/widgets/groupbox.h"
#include "ui/widgets/label.h"
#include "ui/widgets/listbox.h"
#include "ui/widgets/messagebox.h"
#include "ui/widgets/widget.h"
#include "util/datetime.h"
#include "util/fsutil.h"
#include "util/helpers.h"

#include <algorithm>
#include <boost/format.hpp>
#include <boost/log/trivial.hpp>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <iomanip>
#include <locale>
#include <map>
#include <optional>
#include <ratio>
#include <sstream>
#include <system_error>
#include <type_traits>
#include <utility>

namespace menu
{
enum class CleanupAction
{
  Current,
  BeforeDate,
  AllOther,
  AllOtherForLevel,
  AllExceptNewestPerLevel,
  AllExceptNewestPerLevelAndCompact,
  Compact,
  OrderByDateAndCompact,
  OrderByDateLevelAndCompact,
  Cancel
};

class SavegameListMenuState::CleanupWidget final : public ui::widgets::Widget
{
public:
  explicit CleanupWidget(const std::optional<std::string>& slotDate)
      : ui::widgets::Widget{}
      , m_referenceDate{slotDate}
  {
    createList();
    m_container = std::make_shared<ui::widgets::GroupBox>(/* translators: TR charmap encoding */ _("Clean Up Saves"),
                                                          gsl::not_null{m_list});
    fitToContent();
  }

  ~CleanupWidget() override = default;

  void nextEntry()
  {
    m_list->nextEntry();
  }

  void prevEntry()
  {
    m_list->prevEntry();
  }

  [[nodiscard]] glm::ivec2 getPosition() const override
  {
    return m_container->getPosition();
  }

  [[nodiscard]] glm::ivec2 getSize() const override
  {
    return m_container->getSize();
  }

  void setPosition(const glm::ivec2& position) override
  {
    m_container->setPosition(position);
  }

  void setSize(const glm::ivec2& size) override
  {
    m_container->setSize(size);
  }

  void update(bool hasFocus) override
  {
    m_container->update(hasFocus);
  }

  void fitToContent() override
  {
    m_container->fitToContent();
  }

  void draw(ui::Ui& ui, const engine::Presenter& presenter) const override
  {
    m_container->draw(ui, presenter);
  }

  [[nodiscard]] auto getSelectedAction() const
  {
    return m_actionPerListItem.at(m_list->getSelected());
  }

  [[nodiscard]] const auto& getReferenceDate() const
  {
    return m_referenceDate;
  }

private:
  void createList()
  {
    m_list = gsl::make_shared<ui::widgets::ListBox>();
    if(m_referenceDate.has_value())
    {
      m_actionPerListItem.emplace_back(CleanupAction::Current);
      m_list->append(
        gsl::make_shared<ui::widgets::Label>(/* translators: TR charmap encoding */ _("Clear Selected Slot")));

      m_actionPerListItem.emplace_back(CleanupAction::BeforeDate);
      m_list->append(gsl::make_shared<ui::widgets::Label>(
        /* translators: TR charmap encoding */ _("Clear Slots Before %1%", *m_referenceDate)));

      m_actionPerListItem.emplace_back(CleanupAction::AllOther);
      m_list->append(gsl::make_shared<ui::widgets::Label>(
        /* translators: TR charmap encoding */ _("Clear All Other Slots")));

      m_actionPerListItem.emplace_back(CleanupAction::AllOtherForLevel);
      m_list->append(gsl::make_shared<ui::widgets::Label>(
        /* translators: TR charmap encoding */ _("Clear All Other Level Slots")));
    }

    m_actionPerListItem.emplace_back(CleanupAction::AllExceptNewestPerLevel);
    m_list->append(gsl::make_shared<ui::widgets::Label>(
      /* translators: TR charmap encoding */ _("Clear All Slots, Keep Most Recent Level Slots, and Compact")));

    m_actionPerListItem.emplace_back(CleanupAction::Compact);
    m_list->append(gsl::make_shared<ui::widgets::Label>(
      /* translators: TR charmap encoding */ _("Compact")));

    m_actionPerListItem.emplace_back(CleanupAction::OrderByDateAndCompact);
    m_list->append(gsl::make_shared<ui::widgets::Label>(
      /* translators: TR charmap encoding */ _("Order by Date and Compact")));

    m_actionPerListItem.emplace_back(CleanupAction::OrderByDateLevelAndCompact);
    m_list->append(gsl::make_shared<ui::widgets::Label>(
      /* translators: TR charmap encoding */ _("Group by Level, Order by Date, and Compact")));

    m_actionPerListItem.emplace_back(CleanupAction::Cancel);
    m_list->append(gsl::make_shared<ui::widgets::Label>(
      /* translators: TR charmap encoding */ _("Cancel")));

    m_list->fitToContent();
  }

  std::shared_ptr<ui::widgets::GroupBox> m_container;
  std::shared_ptr<ui::widgets::ListBox> m_list;
  std::vector<CleanupAction> m_actionPerListItem;
  std::optional<std::string> m_referenceDate;
};

class SavegameListMenuState::SavegameEntry final : public ui::widgets::Widget
{
public:
  explicit SavegameEntry(const std::optional<size_t>& slot,
                         const std::string& label,
                         const std::string& levelTitle,
                         const std::optional<std::filesystem::file_time_type>& time)
      : m_slot{slot}
      , m_label{util::escape(label)}
      , m_time{time}
      , m_levelTitle{levelTitle}
  {
  }

  [[nodiscard]] glm::ivec2 getPosition() const override
  {
    return m_label.getPosition();
  }

  [[nodiscard]] glm::ivec2 getSize() const override
  {
    return m_label.getSize();
  }

  void setPosition(const glm::ivec2& position) override
  {
    m_label.setPosition(position);
  }

  void setSize(const glm::ivec2& size) override
  {
    m_label.setSize(size);
  }

  void update(bool hasFocus) override
  {
    m_label.update(hasFocus);
  }

  void fitToContent() override
  {
    m_label.fitToContent();
  }

  void draw(ui::Ui& ui, const engine::Presenter& presenter) const override
  {
    m_label.draw(ui, presenter);
  }

  [[nodiscard]] auto getSlot() const
  {
    return m_slot;
  }

  [[nodiscard]] const auto& getTime() const
  {
    return m_time;
  }

  [[nodiscard]] const auto& getLevelTitle() const
  {
    return m_levelTitle;
  }

private:
  std::optional<size_t> m_slot;
  ui::widgets::Label m_label;
  std::optional<std::filesystem::file_time_type> m_time;
  std::string m_levelTitle;
};

SavegameListMenuState::SavegameListMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                             std::unique_ptr<MenuState> previous,
                                             const std::string& heading,
                                             const engine::world::World& world,
                                             bool loading)
    : ListDisplayMenuState{ringTransform, heading, 10}
    , m_previous{std::move(previous)}
    , m_loading{loading}
{
  updateSavegameInfos(world);

  if(!world.getEngine().getEngineConfig()->selectFirstFreeOrOldestSlot)
  {
    selectMostRecentSlot();
  }
  else
  {
    selectFirstFreeOrOldestSlot();
  }
}

std::unique_ptr<MenuState>
  SavegameListMenuState::onSelected(size_t selectedIdx, engine::world::World& world, MenuDisplay& display)
{
  const auto slot = m_entries.at(selectedIdx)->getSlot();
  if(!m_loading)
  {
    if(!slot.has_value())
    {
      // don't allow the quicksave to be overwritten in the passport
      return nullptr;
    }

    if(m_hasSavegame.at(*slot))
    {
      m_overwriteConfirmation = std::make_shared<ui::widgets::MessageBox>(
        /* translators: TR charmap encoding */ _("Overwrite slot %1%?", *slot + 1));
      m_confirmOverwritePressedSince = std::chrono::steady_clock::now();
      return nullptr;
    }
    world.save(slot);
    return create<ClosePassportMenuState>(display.getCurrentRing().getSelectedObject(),
                                          create<DeflateRingMenuState>(DeflateRingMenuState::Direction::Backpack,
                                                                       create<DoneMenuState>(MenuResult::Closed)));
  }
  else if((slot.has_value() && m_hasSavegame.at(*slot)) || (!slot.has_value() && m_hasQuicksave))
  {
    return create<ClosePassportMenuState>(
      display.getCurrentRing().getSelectedObject(),
      create<DeflateRingMenuState>(DeflateRingMenuState::Direction::Backpack, create<RequestLoadMenuState>(slot)));
  }
  return std::move(m_previous);
}

std::unique_ptr<MenuState> SavegameListMenuState::onAborted()
{
  return std::move(m_previous);
}

std::unique_ptr<MenuState> SavegameListMenuState::onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display)
{
  if(m_overwriteConfirmation == nullptr && m_cleanupWidget == nullptr)
  {
    return onDefaultFrame(ui, world, display);
  }

  if(m_overwriteConfirmation != nullptr)
  {
    return onConfirmOverwriteFrame(ui, world, display);
  }
  else if(m_cleanupWidget != nullptr)
  {
    return onCleanupFrame(ui, world, display);
  }
  else
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("invalid savegame list menu state"));
  }
}

void SavegameListMenuState::sortEntries()
{
  clear();

  std::sort(m_entries.begin(),
            m_entries.end(),
            [this](const auto& lhs, const auto& rhs)
            {
              const auto& lhsT = lhs->getTime();
              const auto& rhsT = rhs->getTime();
              switch(m_ordering)
              {
              case Ordering::Slot:
                break;
              case Ordering::DateAsc:
                if(!lhsT.has_value() && !rhsT.has_value())
                  break;
                else if(lhsT.has_value() && !rhsT.has_value())
                  return true;
                else if(!lhsT.has_value() && rhsT.has_value())
                  return false;
                else if(*lhsT != *rhsT)
                  return *lhsT < *rhsT;
                break;
              case Ordering::DateDesc:
                if(!lhsT.has_value() && !rhsT.has_value())
                  break;
                else if(lhsT.has_value() && !rhsT.has_value())
                  return true;
                else if(!lhsT.has_value() && rhsT.has_value())
                  return false;
                else if(*lhsT != *rhsT)
                  return *lhsT > *rhsT;
                break;
              }

              return lhs->getSlot() < rhs->getSlot();
            });

  for(const auto& entry : m_entries)
    append(entry);
}

std::unique_ptr<MenuState>
  SavegameListMenuState::onDefaultFrame(ui::Ui& ui, engine::world::World& world, menu::MenuDisplay& display)
{
  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::PrevScreen))
  {
    switch(m_ordering)
    {
    case Ordering::Slot:
      m_ordering = Ordering::DateDesc;
      break;
    case Ordering::DateAsc:
      m_ordering = Ordering::Slot;
      break;
    case Ordering::DateDesc:
      m_ordering = Ordering::DateAsc;
      break;
    }
    sortEntries();
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::NextScreen))
  {
    switch(m_ordering)
    {
    case Ordering::Slot:
      m_ordering = Ordering::DateAsc;
      break;
    case Ordering::DateAsc:
      m_ordering = Ordering::DateDesc;
      break;
    case Ordering::DateDesc:
      m_ordering = Ordering::Slot;
      break;
    }
    sortEntries();
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::SecondaryInteraction))
  {
    const auto slot = m_entries.at(getListBox()->getSelected())->getSlot();
    if(!slot.has_value())
    {
      draw(ui, world, display);
      // don't allow management using the quicksave slot
      return nullptr;
    }

    std::optional<std::string> saveTime;
    if(const auto it = m_savegameInfos.find(*slot); it != m_savegameInfos.end())
    {
      saveTime = util::toSavegameTime(it->second.saveTime, world.getEngine().getLocale());
    }
    m_cleanupWidget = std::make_shared<CleanupWidget>(saveTime);
    draw(ui, world, display);
    return nullptr;
  }

  return ListDisplayMenuState::onFrame(ui, world, display);
}

std::unique_ptr<MenuState>
  SavegameListMenuState::onConfirmOverwriteFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display)
{
  draw(ui, world, display);
  m_overwriteConfirmation->fitToContent();
  m_overwriteConfirmation->setPosition({(ui.getSize().x - m_overwriteConfirmation->getSize().x) / 2,
                                        ui.getSize().y - m_overwriteConfirmation->getSize().y - 90});

  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::MenuLeft)
     || world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::MenuRight))
  {
    m_overwriteConfirmation->toggleConfirmed();
  }
  else if(world.getPresenter().getInputHandler().hasAction(hid::Action::PrimaryInteraction))
  {
    const auto justConfirmed
      = world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::PrimaryInteraction);
    if(justConfirmed && !m_overwriteConfirmation->isConfirmed())
    {
      m_overwriteConfirmation.reset();
      return nullptr;
    }

    const auto overwriteConfirmed
      = world.getEngine().getEngineConfig()->delaySaveEnabled
          ? m_confirmOverwritePressedSince
              <= std::chrono::steady_clock::now()
                   - std::chrono::seconds{world.getEngine().getEngineConfig()->delaySaveDurationSeconds}
          : justConfirmed;
    if(overwriteConfirmed)
    {
      const auto slot = m_entries.at(getListBox()->getSelected())->getSlot();
      gsl_Assert(slot.has_value());
      world.save(*slot);
      return create<ClosePassportMenuState>(display.getCurrentRing().getSelectedObject(),
                                            create<DeflateRingMenuState>(DeflateRingMenuState::Direction::Backpack,
                                                                         create<DoneMenuState>(MenuResult::Closed)));
    }
  }
  else
  {
    m_confirmOverwritePressedSince = std::chrono::steady_clock::now();
  }

  m_overwriteConfirmation->update(true);
  m_overwriteConfirmation->draw(ui, world.getPresenter());
  return nullptr;
}

std::unique_ptr<MenuState>
  SavegameListMenuState::onCleanupFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display)
{
  draw(ui, world, display);
  m_cleanupWidget->fitToContent();
  m_cleanupWidget->setPosition(
    {(ui.getSize().x - m_cleanupWidget->getSize().x) / 2, ui.getSize().y - m_cleanupWidget->getSize().y - 90});
  m_cleanupWidget->draw(ui, world.getPresenter());

  if(m_cleanupConfirmation != nullptr)
  {
    m_cleanupConfirmation->fitToContent();
    m_cleanupConfirmation->setPosition({(ui.getSize().x - m_cleanupConfirmation->getSize().x) / 2,
                                        ui.getSize().y - m_cleanupConfirmation->getSize().y - 90});
    m_cleanupConfirmation->update(true);
    m_cleanupConfirmation->draw(ui, world.getPresenter());
    if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Return))
    {
      m_cleanupConfirmation.reset();
    }
    else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::MenuLeft)
            || world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::MenuRight))
    {
      m_cleanupConfirmation->toggleConfirmed();
    }
    else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::PrimaryInteraction))
    {
      if(!m_cleanupConfirmation->isConfirmed())
      {
        m_cleanupConfirmation.reset();
        return nullptr;
      }

      m_cleanupConfirmation.reset();

      cleanupSaves(world);
    }

    return nullptr;
  }

  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Return))
  {
    m_cleanupWidget.reset();
    return nullptr;
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::MenuUp))
  {
    m_cleanupWidget->prevEntry();
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::MenuDown))
  {
    m_cleanupWidget->nextEntry();
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::PrimaryInteraction))
  {
    initCleanupConfirmation();
    return nullptr;
  }

  m_cleanupWidget->update(true);
  m_cleanupWidget->draw(ui, world.getPresenter());
  return nullptr;
}

void SavegameListMenuState::selectMostRecentSlot()
{
  std::filesystem::file_time_type mostRecentTime = std::filesystem::file_time_type::min();
  size_t mostRecentSlot = 1;
  for(size_t slot = 0; slot < core::SavegameSlots; ++slot)
  {
    if(auto it = m_savegameInfos.find(slot); it != m_savegameInfos.end())
    {
      if(it->second.saveTime > mostRecentTime)
      {
        mostRecentTime = it->second.saveTime;
        mostRecentSlot = slot + 1;
      }
    }
  }

  getListBox()->setSelected(mostRecentSlot);
}

void SavegameListMenuState::selectFirstFreeOrOldestSlot()
{
  std::filesystem::file_time_type oldestTime = std::filesystem::file_time_type::max();
  size_t bestSlot = 1;
  for(size_t slot = 0; slot < core::SavegameSlots; ++slot)
  {
    if(auto it = m_savegameInfos.find(slot); it != m_savegameInfos.end())
    {
      if(it->second.saveTime < oldestTime)
      {
        oldestTime = it->second.saveTime;
        bestSlot = slot + 1;
      }
    }
    else
    {
      bestSlot = slot + 1;
      break;
    }
  }

  getListBox()->setSelected(bestSlot);
}

void SavegameListMenuState::updateSavegameInfos(const engine::world::World& world)
{
  const auto [quicksaveInfo, savedGames] = world.getSavedGames();
  m_savegameInfos = savedGames;
  m_hasQuicksave = quicksaveInfo.has_value();

  m_entries.clear();
  clear();

  const auto levelFilepathsTitles = world.getEngine().getScriptEngine().getGameflow().getLevelFilepathsTitles();

  auto addSavegameEntry
    = [this, &world, &levelFilepathsTitles](const std::optional<size_t>& slot, const engine::SavegameInfo& info)
  {
    const auto timeStr = util::toSavegameTime(info.saveTime, world.getEngine().getLocale());

    std::string levelTitle;
    if(slot.has_value())
    {
      const auto titlesIt = std::find_if(levelFilepathsTitles.cbegin(),
                                         levelFilepathsTitles.cend(),
                                         [&info](const auto& entry)
                                         {
                                           return util::preferredEqual(entry.first, info.meta.filename);
                                         });
      gsl_Assert(titlesIt != levelFilepathsTitles.cend());
      auto titleIt = titlesIt->second.find(world.getEngine().getLocaleWithoutEncoding());
      if(titleIt == titlesIt->second.end())
      {
        titleIt = titlesIt->second.find("en_GB");
      }
      gsl_Assert(titleIt != titlesIt->second.end());
      levelTitle = titleIt->second;
    }
    else
    {
      levelTitle = /* translators: TR charmap encoding */ pgettext("SavegameTitle", "Quicksave");
    }
    const auto title = (boost::format(/* translators: TR charmap encoding */ pgettext("SavegameTitle", "%1% - %2%"))
                        % timeStr % levelTitle)
                         .str();
    const auto entry = gsl::make_shared<SavegameEntry>(slot, title, levelTitle, info.saveTime);
    append(entry);
    m_entries.emplace_back(entry);
  };

  if(quicksaveInfo.has_value())
  {
    addSavegameEntry(std::nullopt, *quicksaveInfo);
  }
  else
  {
    const auto entry = gsl::make_shared<SavegameEntry>(
      std::nullopt,
      /* translators: TR charmap encoding */ pgettext("SavegameTitle", "- NO QUICKSAVE"),
      std::string{},
      std::nullopt);
    append(entry);
    m_entries.emplace_back(entry);
  }

  for(size_t slot = 0; slot < core::SavegameSlots; ++slot)
  {
    if(auto it = m_savegameInfos.find(slot); it != m_savegameInfos.end())
    {
      addSavegameEntry(slot, it->second);
      m_hasSavegame.emplace_back(true);
    }
    else
    {
      auto label = gsl::make_shared<SavegameEntry>(
        slot,
        /* translators: TR charmap encoding */ pgettext("SavegameTitle", "- EMPTY SLOT %1%", slot + 1),
        std::string{},
        std::nullopt);
      append(label);
      m_entries.emplace_back(label);
      m_hasSavegame.emplace_back(false);
    }
  }
}

void SavegameListMenuState::initCleanupConfirmation()
{
  static constexpr int MaxTextWidth = 500;

  const auto slot = m_entries.at(getListBox()->getSelected())->getSlot();
  const auto levelTitle = m_entries.at(getListBox()->getSelected())->getLevelTitle();
  switch(m_cleanupWidget->getSelectedAction())
  {
  case CleanupAction::Current:
    gsl_Assert(slot.has_value());
    m_cleanupConfirmation = std::make_shared<ui::widgets::MessageBox>(
      /* translators: TR charmap encoding */ _("Clear slot %1%?", *slot + 1));
    break;
  case CleanupAction::BeforeDate:
  {
    const auto referenceDate = m_cleanupWidget->getReferenceDate();
    gsl_Assert(referenceDate.has_value());
    m_cleanupConfirmation = std::make_shared<ui::widgets::MessageBox>(
      /* translators: TR charmap encoding */ _("Clear Slots Before %1%?", referenceDate.value()));
    break;
  }
  case CleanupAction::AllOther:
    gsl_Assert(slot.has_value());
    m_cleanupConfirmation = std::make_shared<ui::widgets::MessageBox>(ui::breakLines(
      /* translators: TR charmap encoding */ _("Clear All Other Slots?\n\n"
                                               "This will erase all of your data, except for Slot %1%.",
                                               *slot + 1),
      MaxTextWidth));
    break;
  case CleanupAction::AllOtherForLevel:
    gsl_Assert(slot.has_value());
    gsl_Assert(!levelTitle.empty());
    m_cleanupConfirmation = std::make_shared<ui::widgets::MessageBox>(
      ui::breakLines(/* translators: TR charmap encoding */ _("Clear All Other Level Slots?\n\n"
                                                              "This will erase all of your level data"
                                                              " for %1%, except for Slot %2%.",
                                                              levelTitle,
                                                              *slot + 1),
                     MaxTextWidth));
    break;
  case CleanupAction::AllExceptNewestPerLevel:
    m_cleanupConfirmation = std::make_shared<ui::widgets::MessageBox>(
      ui::breakLines(/* translators: TR charmap encoding */ _("Clear Historic Slots?\n\n"
                                                              "This will erase all of your saves for each"
                                                              " level, except for the most recent ones."),
                     MaxTextWidth));
    break;
  case CleanupAction::AllExceptNewestPerLevelAndCompact:
    m_cleanupConfirmation = std::make_shared<ui::widgets::MessageBox>(
      ui::breakLines(/* translators: TR charmap encoding */ _("Clear Historic Slots?\n\n"
                                                              "This will erase all of your saves for each"
                                                              " level, except for the most recent ones, and"
                                                              " move all slots to the start of the list."),
                     MaxTextWidth));
    break;
  case CleanupAction::Compact:
    m_cleanupConfirmation = std::make_shared<ui::widgets::MessageBox>(
      ui::breakLines(/* translators: TR charmap encoding */ _("Compact All Slots?\n\n"
                                                              "This will move all slots to the start of the list."),
                     MaxTextWidth));
    break;
  case CleanupAction::OrderByDateAndCompact:
    m_cleanupConfirmation = std::make_shared<ui::widgets::MessageBox>(ui::breakLines(
      /* translators: TR charmap encoding */ _("Order by Date and Compact All Slots?\n\n"
                                               "This will order your saves"
                                               " by date and move all slots to the start of the list."),
      MaxTextWidth));
    break;
  case CleanupAction::OrderByDateLevelAndCompact:
    m_cleanupConfirmation = std::make_shared<ui::widgets::MessageBox>(
      ui::breakLines(/* translators: TR charmap encoding */ _("Order by Level and Date and Compact All Slots?\n\n"
                                                              "This will order your saves by level,"
                                                              " then by date, and move all slots"
                                                              " to the start of the list."),
                     MaxTextWidth));
    break;
  case CleanupAction::Cancel:
    m_cleanupWidget.reset();
    break;
  }
}

void SavegameListMenuState::cleanupSaves(const engine::world::World& world)
{
  const auto slot = m_entries.at(getListBox()->getSelected())->getSlot();
  switch(m_cleanupWidget->getSelectedAction())
  {
  case CleanupAction::Current:
    gsl_Assert(slot.has_value());
    deleteSave(world.getEngine(), *slot);
    updateSavegameInfos(world);
    break;
  case CleanupAction::BeforeDate:
    gsl_Assert(slot.has_value());
    deleteSavesBefore(world.getEngine(), m_savegameInfos, m_savegameInfos.at(*slot).saveTime);
    updateSavegameInfos(world);
    break;
  case CleanupAction::AllOther:
    gsl_Assert(slot.has_value());
    deleteSavesExcept(world.getEngine(), m_savegameInfos, *slot);
    updateSavegameInfos(world);
    break;
  case CleanupAction::AllOtherForLevel:
  {
    gsl_Assert(slot.has_value());
    const auto referenceName = m_savegameInfos.at(*slot).meta.filename;
    for(const auto& [n, info] : m_savegameInfos)
    {
      if(n != *slot && util::preferredEqual(info.meta.filename, referenceName))
      {
        deleteSave(world.getEngine(), n);
      }
    }
    updateSavegameInfos(world);
    break;
  }
  case CleanupAction::AllExceptNewestPerLevel:
    deleteSavesPerLevelExceptNewest(world.getEngine(), m_savegameInfos);
    updateSavegameInfos(world);
    break;
  case CleanupAction::AllExceptNewestPerLevelAndCompact:
    deleteSavesPerLevelExceptNewest(world.getEngine(), m_savegameInfos);
    updateSavegameInfos(world);
    setSelected(compactSaves(world.getEngine(), m_savegameInfos, slot));
    updateSavegameInfos(world);
    break;
  case CleanupAction::Compact:
    setSelected(compactSaves(world.getEngine(), m_savegameInfos, slot));
    updateSavegameInfos(world);
    break;
  case CleanupAction::OrderByDateAndCompact:
    orderByDate(world.getEngine(), m_savegameInfos);
    updateSavegameInfos(world);
    setSelected(compactSaves(world.getEngine(), m_savegameInfos, slot));
    updateSavegameInfos(world);
    break;
  case CleanupAction::OrderByDateLevelAndCompact:
    orderByLevelThenDate(world.getEngine(), m_savegameInfos);
    updateSavegameInfos(world);
    setSelected(compactSaves(world.getEngine(), m_savegameInfos, slot));
    updateSavegameInfos(world);
    break;
  case CleanupAction::Cancel:
    break;
  }

  // must be re-initialized, because we might end up in an empty slot, which will then make items in the list invalid
  m_cleanupWidget.reset();
}
} // namespace menu
