#include "savegamelistmenustate.h"

#include "closepassportmenustate.h"
#include "core/i18n.h"
#include "core/magic.h"
#include "deflateringmenustate.h"
#include "donemenustate.h"
#include "engine/engine.h"
#include "engine/presenter.h"
#include "engine/world/world.h"
#include "hid/actions.h"
#include "hid/inputhandler.h"
#include "listdisplaymenustate.h"
#include "menudisplay.h"
#include "menuring.h"
#include "menustate.h"
#include "requestloadmenustate.h"
#include "ui/ui.h"
#include "ui/widgets/label.h"
#include "ui/widgets/listbox.h"
#include "ui/widgets/messagebox.h"
#include "util/helpers.h"

#include <boost/format.hpp>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <iomanip>
#include <locale>
#include <map>
#include <ratio>
#include <sstream>
#include <utility>

namespace menu
{
class SavegameListMenuState::SavegameEntry final : public ui::widgets::Widget
{
public:
  explicit SavegameEntry(const std::optional<size_t>& slot,
                         const std::string& label,
                         const std::optional<std::filesystem::file_time_type>& time)
      : m_slot{slot}
      , m_label{util::escape(label)}
      , m_time{time}
  {
    m_label.fitToContent();
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

private:
  std::optional<size_t> m_slot;
  ui::widgets::Label m_label;
  std::optional<std::filesystem::file_time_type> m_time;
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
  auto addSavegameEntry = [this, &world](const std::optional<size_t>& slot, const engine::SavegameInfo& info)
  {
    const auto timePoint
      = std::chrono::system_clock::to_time_t(std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        info.saveTime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now()));
    const auto localTime = localtime(&timePoint);
    Expects(localTime != nullptr);
    std::stringstream timeStr;
    timeStr.imbue(std::locale(world.getEngine().getLocale()));
    timeStr << std::put_time(localTime,
                             /* translators: TR charmap encoding */ pgettext("SavegameTime", "%d %B %Y %X"));

    const auto title = (boost::format(/* translators: TR charmap encoding */ pgettext("SavegameTitle", "%1% - %2%"))
                        % timeStr.str() % info.meta.title)
                         .str();
    const auto entry = gsl::make_shared<SavegameEntry>(slot, title, info.saveTime);
    append(entry);
    m_entries.emplace_back(entry);
  };

  const auto [quicksaveInfo, savedGames] = world.getSavedGames();
  m_hasQuicksave = quicksaveInfo.has_value();

  if(quicksaveInfo.has_value())
  {
    addSavegameEntry(std::nullopt, *quicksaveInfo);
  }
  else
  {
    const auto entry = gsl::make_shared<SavegameEntry>(
      std::nullopt, /* translators: TR charmap encoding */ pgettext("SavegameTitle", "- NO QUICKSAVE"), std::nullopt);
    append(entry);
    m_entries.emplace_back(entry);
  }

  std::filesystem::file_time_type mostRecentTime = std::filesystem::file_time_type::min();
  size_t mostRecentSlot = 1;
  for(size_t slot = 0; slot < core::SavegameSlots; ++slot)
  {
    if(auto it = savedGames.find(slot); it != savedGames.end())
    {
      if(it->second.saveTime > mostRecentTime)
      {
        mostRecentTime = it->second.saveTime;
        mostRecentSlot = slot + 1;
      }

      addSavegameEntry(slot, it->second);
      m_hasSavegame.emplace_back(true);
    }
    else
    {
      auto label = gsl::make_shared<SavegameEntry>(
        slot,
        /* translators: TR charmap encoding */ pgettext("SavegameTitle", "- EMPTY SLOT %1%", slot + 1),
        std::nullopt);
      append(label);
      m_entries.emplace_back(label);
      m_hasSavegame.emplace_back(false);
    }
  }

  getListBox()->setSelected(mostRecentSlot);
}

std::unique_ptr<MenuState>
  SavegameListMenuState::onSelected(size_t selectedIdx, engine::world::World& world, MenuDisplay& display)
{
  const auto slot = m_entries.at(selectedIdx)->getSlot();
  if(!m_loading)
  {
    if(!slot.has_value())
      return nullptr;

    if(m_hasSavegame.at(*slot))
    {
      m_overwriteConfirmation = std::make_shared<ui::widgets::MessageBox>(
        /* translators: TR charmap encoding */ _("Overwrite slot %1%?", *slot + 1));
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
  if(m_overwriteConfirmation == nullptr)
  {
    if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::StepLeft))
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
    else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::StepRight))
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

    return ListDisplayMenuState::onFrame(ui, world, display);
  }

  draw(ui, world, display);
  m_overwriteConfirmation->fitToContent();
  m_overwriteConfirmation->setPosition({(ui.getSize().x - m_overwriteConfirmation->getSize().x) / 2,
                                        ui.getSize().y - m_overwriteConfirmation->getSize().y - 90});

  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Left)
     || world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Right))
  {
    m_overwriteConfirmation->setConfirmed(!m_overwriteConfirmation->isConfirmed());
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Action))
  {
    if(m_overwriteConfirmation->isConfirmed())
    {
      world.save(m_entries.at(getListBox()->getSelected())->getSlot().value());
      return create<ClosePassportMenuState>(display.getCurrentRing().getSelectedObject(),
                                            create<DeflateRingMenuState>(DeflateRingMenuState::Direction::Backpack,
                                                                         create<DoneMenuState>(MenuResult::Closed)));
    }
    else
    {
      m_overwriteConfirmation.reset();
      return nullptr;
    }
  }

  m_overwriteConfirmation->update(true);
  m_overwriteConfirmation->draw(ui, world.getPresenter());
  return nullptr;
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
} // namespace menu
