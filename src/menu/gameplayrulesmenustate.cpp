#include "gameplayrulesmenustate.h"

#include "core/i18n.h"
#include "engine/engine.h"
#include "engine/gameplayrules.h"
#include "engine/presenter.h"
#include "engine/world/world.h"
#include "hid/actions.h"
#include "hid/inputhandler.h"
#include "hid/inputstate.h"
#include "menustate.h"
#include "selectedmenustate.h"
#include "ui/core.h"
#include "ui/ui.h"
#include "ui/widgets/checkbox.h"
#include "ui/widgets/checklistbox.h"
#include "ui/widgets/groupbox.h"
#include "ui/widgets/label.h"
#include "ui/widgets/listbox.h"
#include "ui/widgets/textbox.h"
#include "ui/widgets/widget.h"

#include <functional>
#include <glm/fwd.hpp>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <string>
#include <tuple>
#include <utility>

namespace menu
{
constexpr int MaxDescriptionWidth = 500;

GameplayRulesMenuState::GameplayRulesMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                               std::unique_ptr<MenuState> previous,
                                               const engine::GameplayRules& rules)
    : SelectedMenuState{ringTransform}
    , m_previous{std::move(previous)}
    , m_rules{rules}

{
  auto listBox = gsl::make_shared<ui::widgets::CheckListBox>();
  m_listBox = listBox;                                   // cppcheck-suppress useInitializationList
  m_container = gsl::make_unique<ui::widgets::GroupBox>( // cppcheck-suppress useInitializationList
    /* translators: TR charmap encoding */ _("Rules"),
    listBox);

  listBox->addSetting(
    /* translators: TR charmap encoding */
    _("No Loads"),
    [this]()
    {
      return m_rules.noLoads;
    },
    [this]()
    {
      m_rules.noLoads = !m_rules.noLoads;
    });
  m_descriptions.emplace_back(std::make_shared<ui::widgets::TextBox>(
    /* translators: TR charmap encoding */ _("You cannot load any savegame."), MaxDescriptionWidth));

  listBox->addSetting(
    /* translators: TR charmap encoding */
    _("No Meds"),
    [this]()
    {
      return m_rules.noMeds;
    },
    [this]()
    {
      m_rules.noMeds = !m_rules.noMeds;
    });
  m_descriptions.emplace_back(std::make_shared<ui::widgets::TextBox>(
    /* translators: TR charmap encoding */ _("You cannot use and medi pack."), MaxDescriptionWidth));

  listBox->addSetting(
    /* translators: TR charmap encoding */
    _("Pistols Only"),
    [this]()
    {
      return m_rules.pistolsOnly;
    },
    [this]()
    {
      m_rules.pistolsOnly = !m_rules.pistolsOnly;
    });
  m_descriptions.emplace_back(std::make_shared<ui::widgets::TextBox>(
    /* translators: TR charmap encoding */ _("You can only use the pistols, no other weapons."), MaxDescriptionWidth));

  listBox->addSetting(
    /* translators: TR charmap encoding */
    _("No Cheats"),
    [this]()
    {
      return m_rules.noCheats;
    },
    [this]()
    {
      m_rules.noCheats = !m_rules.noCheats;
    });
  m_descriptions.emplace_back(std::make_shared<ui::widgets::TextBox>(
    /* translators: TR charmap encoding */ _("Disables all cheats."), MaxDescriptionWidth));

  listBox->addSetting(
    /* translators: TR charmap encoding */
    _("No Hits"),
    [this]()
    {
      return m_rules.noHits;
    },
    [this]()
    {
      m_rules.noHits = !m_rules.noHits;
    });
  m_descriptions.emplace_back(std::make_shared<ui::widgets::TextBox>(
    /* translators: TR charmap encoding */ _("Any enemy or trap hit is an instant kill, except spikes."),
    MaxDescriptionWidth));

  listBox->fitToContent();
  m_container->fitToContent();

  for(const auto& description : m_descriptions)
  {
    description->fitToContent();
    description->setSize({MaxDescriptionWidth, description->getSize().y});
  }
}

std::unique_ptr<MenuState>
  GameplayRulesMenuState::onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& /*display*/)
{
  m_container->setPosition(
    {(ui.getSize().x - m_container->getSize().x) / 2, ui.getSize().y - m_container->getSize().y - 90});
  m_container->update(true);
  m_container->draw(ui, world.getPresenter());
  world.getEngine().setGameplayRules(m_rules);

  {
    const auto& description = m_descriptions.at(m_listBox->getSelectedIndex());
    description->setPosition({(ui.getSize().x - MaxDescriptionWidth) / 2,
                              m_listBox->getPosition().y - description->getSize().y - 3 * ui::FontHeight});
    description->draw(ui, world.getPresenter());
  }

  if(world.getPresenter().getInputHandler().getInputState().menuZMovement.justChangedTo(hid::AxisMovement::Forward))
  {
    if(!m_listBox->prevEntry())
    {
      m_listBox->setSelectedEntry(m_listBox->getEntryCount() - 1);
    }
  }
  else if(world.getPresenter().getInputHandler().getInputState().menuZMovement.justChangedTo(
            hid::AxisMovement::Backward))
  {
    if(!m_listBox->nextEntry())
    {
      m_listBox->setSelectedEntry(0);
    }
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::PrimaryInteraction))
  {
    const auto& [getter, toggler, checkbox] = m_listBox->getSelected();
    toggler();
    checkbox->setChecked(getter());
  }
  else if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Return))
  {
    return std::move(m_previous);
  }

  return nullptr;
}

GameplayRulesMenuState::~GameplayRulesMenuState() = default;
} // namespace menu
