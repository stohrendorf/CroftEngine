#include "detailedlevelstats.h"

#include "core.h"
#include "core/i18n.h"
#include "core/magic.h"
#include "core/units.h"
#include "engine/inventory.h"
#include "engine/player.h"
#include "engine/weapontype.h"
#include "engine/world/world.h"
#include "qs/qs.h"
#include "ui.h"
#include "util/helpers.h"
#include "widgets/gridbox.h"
#include "widgets/groupbox.h"
#include "widgets/label.h"

#include <boost/format.hpp>
#include <cstddef>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <memory>
#include <string>
#include <utility>

namespace ui
{
DetailedLevelStats::DetailedLevelStats(const engine::world::World& world)
    : m_grid{std::make_shared<widgets::GridBox>(glm::ivec2{FontHeight, 2 * OutlineBorderWidth})}
    , m_container{std::make_shared<widgets::GroupBox>(util::escape(world.getTitle()), m_grid)}
{
  const auto& player = world.getPlayer();

  m_grid->setExtents(3, 7);
  m_grid->setAlignRight(1, true);
  m_grid->setAlignRight(2, true);

  m_grid->set(
    1, 0, std::make_shared<widgets::Label>(/* translators: TR charmap encoding */ pgettext("gamestats", "CURRENT")));

  m_grid->set(
    2, 0, std::make_shared<widgets::Label>(/* translators: TR charmap encoding */ pgettext("gamestats", "TOTAL")));

  m_grid->set(0, 1, std::make_shared<widgets::Label>(/* translators: TR charmap encoding */ _("Pickups")));
  m_grid->set(1, 1, std::make_shared<widgets::Label>(std::to_string(player.pickups)));

  m_grid->set(2, 1, std::make_shared<widgets::Label>(std::to_string(player.pickupsTotal + player.pickups)));

  m_grid->set(0, 2, std::make_shared<widgets::Label>(/* translators: TR charmap encoding */ _("Secrets")));
  m_grid->set(1, 2, std::make_shared<widgets::Label>(std::to_string(player.secrets)));

  m_grid->set(2, 2, std::make_shared<widgets::Label>(std::to_string(player.secretsTotal + player.secrets)));

  m_grid->set(0, 3, std::make_shared<widgets::Label>(/* translators: TR charmap encoding */ _("Kills")));
  m_grid->set(1, 3, std::make_shared<widgets::Label>(std::to_string(player.kills)));

  m_grid->set(2, 3, std::make_shared<widgets::Label>(std::to_string(player.killsTotal + player.kills)));

  m_grid->set(0, 4, std::make_shared<widgets::Label>(/* translators: TR charmap encoding */ _("Small Medi Packs")));
  m_grid->set(1, 4, std::make_shared<widgets::Label>(std::to_string(player.smallMedipacks)));

  m_grid->set(
    2, 4, std::make_shared<widgets::Label>(std::to_string(player.smallMedipacksTotal + player.smallMedipacks)));

  m_grid->set(0, 5, std::make_shared<widgets::Label>(/* translators: TR charmap encoding */ _("Large Medi Packs")));
  m_grid->set(1, 5, std::make_shared<widgets::Label>(std::to_string(player.largeMedipacks)));

  m_grid->set(
    2, 5, std::make_shared<widgets::Label>(std::to_string(player.largeMedipacksTotal + player.largeMedipacks)));

  m_grid->set(0, 6, std::make_shared<widgets::Label>(/* translators: TR charmap encoding */ _("Time Taken")));

  m_grid->set(1, 6, std::make_shared<widgets::Label>(util::toTimeStr(player.timeSpent / core::FrameRate)));

  m_grid->set(
    2,
    6,
    std::make_shared<widgets::Label>(util::toTimeStr((player.timeSpentTotal + player.timeSpent) / core::FrameRate)));

  const auto& inv = player.getInventory();

  auto addAmmoStats = [this, &inv](const char* weaponName, engine::WeaponType weaponType, size_t& y0)
  {
    const auto& ammo = inv.getAmmo(weaponType);
    if(inv.count(ammo.weaponType) == 0 && ammo.shots == 0 && ammo.hits == 0 && ammo.hitsTotal == 0 && ammo.misses == 0
       && ammo.missesTotal == 0)
    {
      return;
    }

    m_grid->setExtents(3, y0 + 2);

    m_grid->set(
      0, y0, std::make_shared<widgets::Label>(/* translators: TR charmap encoding */ _("%1% Hits", weaponName)));
    m_grid->set(1, y0, std::make_shared<widgets::Label>(std::to_string(ammo.hits)));

    m_grid->set(2, y0, std::make_shared<widgets::Label>(std::to_string(ammo.hitsTotal + ammo.hits)));

    m_grid->set(
      0, y0 + 1, std::make_shared<widgets::Label>(/* translators: TR charmap encoding */ _("%1% Misses", weaponName)));
    m_grid->set(1, y0 + 1, std::make_shared<widgets::Label>(std::to_string(ammo.misses)));

    m_grid->set(2, y0 + 1, std::make_shared<widgets::Label>(std::to_string(ammo.missesTotal + ammo.misses)));

    y0 += 2;
  };

  size_t row = 7;
  addAmmoStats(/* translators: TR charmap encoding */ _("Pistols"), engine::WeaponType::Pistols, row);
  addAmmoStats(/* translators: TR charmap encoding */ _("Shotgun"), engine::WeaponType::Shotgun, row);
  addAmmoStats(/* translators: TR charmap encoding */ _("Magnums"), engine::WeaponType::Magnums, row);
  addAmmoStats(/* translators: TR charmap encoding */ _("Uzis"), engine::WeaponType::Uzis, row);

  if(player.usedCheats)
  {
    m_grid->setExtents(3, row + 1);
    m_grid->set(0,
                row,
                std::make_shared<widgets::Label>(
                  /* translators: TR charmap encoding; in the sense of "cheats are used" */ _("Cheats Used")));
  }

  m_grid->fitToContent();
  m_container->fitToContent();
}

DetailedLevelStats::~DetailedLevelStats() = default;

void DetailedLevelStats::draw(Ui& ui, const engine::Presenter& presenter, bool compass) const
{
  m_grid->fitToContent();
  m_container->fitToContent();
  m_grid->update(false);

  if(compass)
  {
    m_container->setPosition(
      {(ui.getSize().x - m_container->getSize().x) / 2, ui.getSize().y - m_container->getSize().y - 90});
  }
  else
  {
    m_container->setPosition((ui.getSize() - m_container->getSize()) / 2);
  }
  m_container->draw(ui, presenter);
}
} // namespace ui
