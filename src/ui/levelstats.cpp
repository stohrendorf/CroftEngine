#include "levelstats.h"

#include "core/i18n.h"
#include "core/magic.h"
#include "engine/player.h"
#include "engine/presenter.h"
#include "hid/actions.h"
#include "hid/names.h"
#include "text.h"
#include "ui/ui.h"
#include "util/helpers.h"

#include <glm/vec2.hpp>
#include <string>

namespace ui
{
void LevelStats::draw(Ui& ui) const
{
  static constexpr auto HeightPerLine = 30;
  static constexpr auto Top = 50;

  const auto center = ui.getSize() / 2;

  int line = 0;
  auto drawLine = [&](const std::string& msg)
  {
    const Text text{msg};
    text.draw(ui, m_presenter->getTrFont(), center - glm::ivec2{text.getWidth() / 2, Top - line * HeightPerLine});
    ++line;
  };

  drawLine(m_title);
  drawLine(/* translators: TR charmap encoding */ _("KILLS %1%", m_player->kills));
  drawLine(/* translators: TR charmap encoding */ _("PICKUPS %1%", m_player->pickups));
  drawLine(/* translators: TR charmap encoding */ _("SECRETS %1% of %2%", m_player->secrets, m_totalSecrets));
  drawLine(
    /* translators: TR charmap encoding */ _("TIME TAKEN %1%", util::toTimeStr(m_player->timeSpent / core::FrameRate)));

  ++line;
  drawLine(/* translators: TR charmap encoding */ _("Use %1% to show detailed stats.",
                                                    getName(hid::Action::SecondaryInteraction)));
}
} // namespace ui
