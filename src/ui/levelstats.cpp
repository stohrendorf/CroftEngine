#include "levelstats.h"

#include "core/i18n.h"
#include "engine/player.h"
#include "engine/presenter.h"
#include "text.h"
#include "ui/ui.h"
#include "util/helpers.h"

#include <boost/format.hpp>
#include <chrono>
#include <gl/pixel.h>
#include <glm/vec2.hpp>

namespace ui
{
void LevelStats::draw(Ui& ui) const
{
  const auto center = ui.getSize() / 2;

  ui.drawBox({0, 0}, ui.getSize(), gl::SRGBA8{0, 0, 0, 224});

  {
    Text text{m_title};
    text.draw(ui, m_presenter->getTrFont(), center - glm::ivec2{text.getWidth() / 2, 50});
  }

  {
    Text text{/* translators: TR charmap encoding */ _("TIME TAKEN %1%",
                                                       util::toTimeStr(m_player->timeSpent / core::FrameRate))};
    text.draw(ui, m_presenter->getTrFont(), center - glm::ivec2{text.getWidth() / 2, -70});
  }

  {
    Text text{/* translators: TR charmap encoding */ _("SECRETS %1% of %2%", m_player->secrets, m_totalSecrets)};
    text.draw(ui, m_presenter->getTrFont(), center - glm::ivec2{text.getWidth() / 2, -40});
  }

  {
    Text text{/* translators: TR charmap encoding */ _("PICKUPS %1%", m_player->pickups)};
    text.draw(ui, m_presenter->getTrFont(), center - glm::ivec2{text.getWidth() / 2, -10});
  }

  {
    Text text{/* translators: TR charmap encoding */ _("KILLS %1%", m_player->kills)};
    text.draw(ui, m_presenter->getTrFont(), center - glm::ivec2{text.getWidth() / 2, 20});
  }
}
} // namespace ui
