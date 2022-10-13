#include "detailedlevelstats.h"

#include "core.h"
#include "core/i18n.h"
#include "engine/player.h"
#include "engine/world/world.h"
#include "util/helpers.h"
#include "widgets/gridbox.h"
#include "widgets/groupbox.h"
#include "widgets/label.h"

#include <glm/vec2.hpp>

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

  auto label
    = std::make_shared<widgets::Label>(/* translators: TR charmap encoding */ pgettext("gamestats", "CURRENT"));
  label->fitToContent();
  m_grid->set(1, 0, std::move(label));

  label = std::make_shared<widgets::Label>(/* translators: TR charmap encoding */ pgettext("gamestats", "TOTAL"));
  label->fitToContent();
  m_grid->set(2, 0, std::move(label));

  label = std::make_shared<widgets::Label>(/* translators: TR charmap encoding */ _("Pickups"));
  label->fitToContent();
  m_grid->set(0, 1, std::move(label));
  label = std::make_shared<widgets::Label>(std::to_string(player.pickups));
  label->fitToContent();
  m_grid->set(1, 1, std::move(label));

  label = std::make_shared<widgets::Label>(std::to_string(player.pickupsTotal + player.pickups));
  label->fitToContent();
  m_grid->set(2, 1, std::move(label));

  label = std::make_shared<widgets::Label>(/* translators: TR charmap encoding */ _("Secrets"));
  label->fitToContent();
  m_grid->set(0, 2, std::move(label));
  label = std::make_shared<widgets::Label>(std::to_string(player.secrets));
  label->fitToContent();
  m_grid->set(1, 2, std::move(label));

  label = std::make_shared<widgets::Label>(std::to_string(player.secretsTotal + player.secrets));
  label->fitToContent();
  m_grid->set(2, 2, std::move(label));

  label = std::make_shared<widgets::Label>(/* translators: TR charmap encoding */ _("Kills"));
  label->fitToContent();
  m_grid->set(0, 3, std::move(label));
  label = std::make_shared<widgets::Label>(std::to_string(player.kills));
  label->fitToContent();
  m_grid->set(1, 3, std::move(label));

  label = std::make_shared<widgets::Label>(std::to_string(player.killsTotal + player.kills));
  label->fitToContent();
  m_grid->set(2, 3, std::move(label));

  label = std::make_shared<widgets::Label>(/* translators: TR charmap encoding */ _("Small Medi Packs"));
  label->fitToContent();
  m_grid->set(0, 4, std::move(label));
  label = std::make_shared<widgets::Label>(std::to_string(player.smallMedipacks));
  label->fitToContent();
  m_grid->set(1, 4, std::move(label));

  label = std::make_shared<widgets::Label>(std::to_string(player.smallMedipacksTotal + player.smallMedipacks));
  label->fitToContent();
  m_grid->set(2, 4, std::move(label));

  label = std::make_shared<widgets::Label>(/* translators: TR charmap encoding */ _("Large Medi Packs"));
  label->fitToContent();
  m_grid->set(0, 5, std::move(label));
  label = std::make_shared<widgets::Label>(std::to_string(player.largeMedipacks));
  label->fitToContent();
  m_grid->set(1, 5, std::move(label));

  label = std::make_shared<widgets::Label>(std::to_string(player.largeMedipacksTotal + player.largeMedipacks));
  label->fitToContent();
  m_grid->set(2, 5, std::move(label));

  label = std::make_shared<widgets::Label>(/* translators: TR charmap encoding */ _("Time Taken"));
  label->fitToContent();
  m_grid->set(0, 6, std::move(label));

  label = std::make_shared<widgets::Label>(util::toTimeStr(player.timeSpent / core::FrameRate));
  label->fitToContent();
  m_grid->set(1, 6, std::move(label));

  label
    = std::make_shared<widgets::Label>(util::toTimeStr((player.timeSpentTotal + player.timeSpent) / core::FrameRate));
  label->fitToContent();
  m_grid->set(2, 6, std::move(label));

  const auto& inv = player.getInventory();

  auto addAmmoStats = [this, &inv](const char* weaponName, engine::WeaponType weaponType, size_t& y0)
  {
    const auto& ammo = inv.getAmmo(weaponType);
    if(inv.count(ammo.weaponType) == 0 && ammo.shots != 0 && ammo.hits == 0 && ammo.hitsTotal == 0 && ammo.misses == 0
       && ammo.missesTotal == 0)
    {
      return;
    }

    m_grid->setExtents(3, y0 + 2);

    auto label = std::make_shared<widgets::Label>(/* translators: TR charmap encoding */ _("%1% Hits", weaponName));
    label->fitToContent();
    m_grid->set(0, y0, std::move(label));
    label = std::make_shared<widgets::Label>(std::to_string(ammo.hits));
    label->fitToContent();
    m_grid->set(1, y0, std::move(label));

    label = std::make_shared<widgets::Label>(std::to_string(ammo.hitsTotal + ammo.hits));
    label->fitToContent();
    m_grid->set(2, y0, std::move(label));

    label = std::make_shared<widgets::Label>(/* translators: TR charmap encoding */ _("%1% Misses", weaponName));
    label->fitToContent();
    m_grid->set(0, y0 + 1, std::move(label));
    label = std::make_shared<widgets::Label>(std::to_string(ammo.misses));
    label->fitToContent();
    m_grid->set(1, y0 + 1, std::move(label));

    label = std::make_shared<widgets::Label>(std::to_string(ammo.missesTotal + ammo.misses));
    label->fitToContent();
    m_grid->set(2, y0 + 1, std::move(label));

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
    label = std::make_shared<widgets::Label>(
      /* translators: TR charmap encoding; in the sense of "cheats are used" */ _("Cheats Used"));
    label->fitToContent();
    m_grid->set(0, row, std::move(label));
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
