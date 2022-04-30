#include "compassmenustate.h"

#include "core/i18n.h"
#include "engine/engine.h"
#include "engine/engineconfig.h"
#include "engine/player.h"
#include "engine/presenter.h"
#include "engine/world/world.h"
#include "hid/actions.h"
#include "hid/inputhandler.h"
#include "menu/selectedmenustate.h"
#include "menudisplay.h"
#include "menuring.h"
#include "ui/core.h"
#include "ui/ui.h"
#include "ui/widgets/gridbox.h"
#include "ui/widgets/groupbox.h"
#include "ui/widgets/label.h"
#include "util/helpers.h"

#include <algorithm>
#include <boost/throw_exception.hpp>
#include <cstddef>
#include <glm/fwd.hpp>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <stdexcept>
#include <tuple>
#include <utility>

namespace menu
{
CompassMenuState::CompassMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                   std::unique_ptr<MenuState> previous,
                                   engine::world::World& world)
    : SelectedMenuState{ringTransform}
    , m_previous{std::move(previous)}
    , m_grid{std::make_shared<ui::widgets::GridBox>(glm::ivec2{ui::FontHeight, 2 * ui::OutlineBorderWidth})}
    , m_container{std::make_shared<ui::widgets::GroupBox>(util::escape(world.getTitle()), m_grid)}
{
  const auto& player = world.getPlayer();

  m_grid->setExtents(3, 7);
  m_grid->setAlignRight(1, true);
  m_grid->setAlignRight(2, true);

  auto label
    = std::make_shared<ui::widgets::Label>(/* translators: TR charmap encoding */ pgettext("gamestats", "CURRENT"));
  label->fitToContent();
  m_grid->set(1, 0, std::move(label));

  label = std::make_shared<ui::widgets::Label>(/* translators: TR charmap encoding */ pgettext("gamestats", "TOTAL"));
  label->fitToContent();
  m_grid->set(2, 0, std::move(label));

  label = std::make_shared<ui::widgets::Label>(/* translators: TR charmap encoding */ _("Pickups"));
  label->fitToContent();
  m_grid->set(0, 1, std::move(label));
  label = std::make_shared<ui::widgets::Label>(std::to_string(player.pickups));
  label->fitToContent();
  m_grid->set(1, 1, std::move(label));

  label = std::make_shared<ui::widgets::Label>(std::to_string(player.pickupsTotal + player.pickups));
  label->fitToContent();
  m_grid->set(2, 1, std::move(label));

  label = std::make_shared<ui::widgets::Label>(/* translators: TR charmap encoding */ _("Secrets"));
  label->fitToContent();
  m_grid->set(0, 2, std::move(label));
  label = std::make_shared<ui::widgets::Label>(std::to_string(player.secrets));
  label->fitToContent();
  m_grid->set(1, 2, std::move(label));

  label = std::make_shared<ui::widgets::Label>(std::to_string(player.secretsTotal + player.secrets));
  label->fitToContent();
  m_grid->set(2, 2, std::move(label));

  label = std::make_shared<ui::widgets::Label>(/* translators: TR charmap encoding */ _("Kills"));
  label->fitToContent();
  m_grid->set(0, 3, std::move(label));
  label = std::make_shared<ui::widgets::Label>(std::to_string(player.kills));
  label->fitToContent();
  m_grid->set(1, 3, std::move(label));

  label = std::make_shared<ui::widgets::Label>(std::to_string(player.killsTotal + player.kills));
  label->fitToContent();
  m_grid->set(2, 3, std::move(label));

  label = std::make_shared<ui::widgets::Label>(/* translators: TR charmap encoding */ _("Small Medi Packs"));
  label->fitToContent();
  m_grid->set(0, 4, std::move(label));
  label = std::make_shared<ui::widgets::Label>(std::to_string(player.smallMedipacks));
  label->fitToContent();
  m_grid->set(1, 4, std::move(label));

  label = std::make_shared<ui::widgets::Label>(std::to_string(player.smallMedipacksTotal + player.smallMedipacks));
  label->fitToContent();
  m_grid->set(2, 4, std::move(label));

  label = std::make_shared<ui::widgets::Label>(/* translators: TR charmap encoding */ _("Large Medi Packs"));
  label->fitToContent();
  m_grid->set(0, 5, std::move(label));
  label = std::make_shared<ui::widgets::Label>(std::to_string(player.largeMedipacks));
  label->fitToContent();
  m_grid->set(1, 5, std::move(label));

  label = std::make_shared<ui::widgets::Label>(std::to_string(player.largeMedipacksTotal + player.largeMedipacks));
  label->fitToContent();
  m_grid->set(2, 5, std::move(label));

  label = std::make_shared<ui::widgets::Label>(/* translators: TR charmap encoding */ _("Time Taken"));
  label->fitToContent();
  m_grid->set(0, 6, std::move(label));

  label = std::make_shared<ui::widgets::Label>(
    util::toTimeStr(std::chrono::duration_cast<std::chrono::seconds>(player.timeSpent)));
  label->fitToContent();
  m_grid->set(1, 6, std::move(label));

  label = std::make_shared<ui::widgets::Label>(
    util::toTimeStr(std::chrono::duration_cast<std::chrono::seconds>(player.timeSpentTotal + player.timeSpent)));
  label->fitToContent();
  m_grid->set(2, 6, std::move(label));

  const auto& inv = player.getInventory();

  auto addAmmoStats = [this, &inv](const char* weaponName, engine::WeaponType weaponType, size_t& y0)
  {
    const auto& ammo = inv.getAmmo(weaponType);
    if(inv.count(ammo.weaponType) == 0 && ammo.ammo && ammo.hits == 0 && ammo.hitsTotal == 0 && ammo.misses == 0
       && ammo.missesTotal == 0)
    {
      return;
    }

    m_grid->setExtents(3, y0 + 2);

    auto label = std::make_shared<ui::widgets::Label>(/* translators: TR charmap encoding */ _("%1% Hits", weaponName));
    label->fitToContent();
    m_grid->set(0, y0, std::move(label));
    label = std::make_shared<ui::widgets::Label>(std::to_string(ammo.hits));
    label->fitToContent();
    m_grid->set(1, y0, std::move(label));

    label = std::make_shared<ui::widgets::Label>(std::to_string(ammo.hitsTotal + ammo.hits));
    label->fitToContent();
    m_grid->set(2, y0, std::move(label));

    label = std::make_shared<ui::widgets::Label>(/* translators: TR charmap encoding */ _("%1% Misses", weaponName));
    label->fitToContent();
    m_grid->set(0, y0 + 1, std::move(label));
    label = std::make_shared<ui::widgets::Label>(std::to_string(ammo.misses));
    label->fitToContent();
    m_grid->set(1, y0 + 1, std::move(label));

    label = std::make_shared<ui::widgets::Label>(std::to_string(ammo.missesTotal + ammo.misses));
    label->fitToContent();
    m_grid->set(2, y0 + 1, std::move(label));

    y0 += 2;
  };

  size_t row = 7;
  addAmmoStats(/* translators: TR charmap encoding */ _("Pistols"), engine::WeaponType::Pistols, row);
  addAmmoStats(/* translators: TR charmap encoding */ _("Shotgun"), engine::WeaponType::Shotgun, row);
  addAmmoStats(/* translators: TR charmap encoding */ _("Uzis"), engine::WeaponType::Uzis, row);
  addAmmoStats(/* translators: TR charmap encoding */ _("Magnums"), engine::WeaponType::Magnums, row);

  if(player.usedCheats)
  {
    m_grid->setExtents(3, row + 1);
    label = std::make_shared<ui::widgets::Label>(
      /* translators: TR charmap encoding; in the sense of "cheats are used" */ _("Cheats Used"));
    label->fitToContent();
    m_grid->set(0, row, std::move(label));
  }

  m_grid->fitToContent();
  m_container->fitToContent();
}

std::unique_ptr<MenuState> CompassMenuState::onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display)
{
  const auto& inputHandler = world.getEngine().getPresenter().getInputHandler();
  if(inputHandler.hasDebouncedAction(hid::Action::Menu))
  {
    auto& object = display.getCurrentRing().getSelectedObject();
    object.animDirection = -1_frame;
    object.goalFrame = 0_frame;
    return std::move(m_previous);
  }

  m_grid->fitToContent();
  m_container->fitToContent();
  m_grid->update(false);

  m_container->setPosition(
    {(ui.getSize().x - m_container->getSize().x) / 2, ui.getSize().y - m_container->getSize().y - 90});
  m_container->draw(ui, world.getPresenter());

  return nullptr;
}
} // namespace menu
