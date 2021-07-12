#include "levelstats.h"

#include "core/i18n.h"
#include "engine/player.h"
#include "engine/presenter.h"
#include "text.h"
#include "ui/ui.h"

namespace ui
{
void LevelStats::draw(Ui& ui) const
{
  const auto center = m_presenter->getViewport() / 2;

  ui.drawBox({0, 0}, m_presenter->getViewport(), gl::SRGBA8{0, 0, 0, 224});

  {
    Text text{m_title};
    text.draw(ui, m_presenter->getTrFont(), center - glm::ivec2{text.getWidth() / 2, 50});
  }

  const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(m_player->timeSpent);
  static constexpr auto Minute = std::chrono::seconds{60};
  static constexpr auto Hour = 60 * Minute;
  if(seconds >= std::chrono::hours{1})
  {
    Text text{/* translators: TR charmap encoding */ _("TIME TAKEN %1%:%2$02d:%3$02d",
                                                       seconds.count() / Hour.count(),
                                                       (seconds.count() / Minute.count()) % Hour.count(),
                                                       seconds.count() % Minute.count())};
    text.draw(ui, m_presenter->getTrFont(), center - glm::ivec2{text.getWidth() / 2, -70});
  }
  else
  {
    Text text{/* translators: TR charmap encoding */ _(
      "TIME TAKEN %1$02d:%2$02d", seconds.count() / Minute.count(), seconds.count() % Minute.count())};
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
