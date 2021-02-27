#include "levelstats.h"

#include "engine/i18nprovider.h"
#include "engine/player.h"
#include "engine/presenter.h"
#include "label.h"

namespace ui
{
void LevelStats::draw(Ui& ui, const engine::I18nProvider& i18n) const
{
  {
    Label lbl{{0, -50}, m_title};
    lbl.alignX = Label::Alignment::Center;
    lbl.alignY = Label::Alignment::Center;
    lbl.draw(ui, m_presenter->getTrFont(), m_presenter->getViewport());
  }

  const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(m_player->timeSpent);
  static constexpr auto Minute = std::chrono::seconds{60};
  static constexpr auto Hour = 60 * Minute;
  if(seconds >= std::chrono::hours{1})
  {
    Label lbl{{0, 70},
              i18n(engine::I18n::StatsTimeTakenHours,
                   seconds.count() / Hour.count(),
                   (seconds.count() / Minute.count()) % Hour.count(),
                   seconds.count() % Minute.count())};
    lbl.alignX = Label::Alignment::Center;
    lbl.alignY = Label::Alignment::Center;
    lbl.draw(ui, m_presenter->getTrFont(), m_presenter->getViewport());
  }
  else
  {
    Label lbl{{0, 70},
              i18n(engine::I18n::StatsTimeTaken, seconds.count() / Minute.count(), seconds.count() % Minute.count())};
    lbl.alignX = Label::Alignment::Center;
    lbl.alignY = Label::Alignment::Center;
    lbl.draw(ui, m_presenter->getTrFont(), m_presenter->getViewport());
  }

  {
    Label lbl{{0, 40}, i18n(engine::I18n::StatsSecrets, m_player->secrets, m_totalSecrets)};
    lbl.alignX = Label::Alignment::Center;
    lbl.alignY = Label::Alignment::Center;
    lbl.draw(ui, m_presenter->getTrFont(), m_presenter->getViewport());
  }

  {
    Label lbl{{0, 10}, i18n(engine::I18n::StatsPickups, m_player->pickups)};
    lbl.alignX = Label::Alignment::Center;
    lbl.alignY = Label::Alignment::Center;
    lbl.draw(ui, m_presenter->getTrFont(), m_presenter->getViewport());
  }

  {
    Label lbl{{0, -20}, i18n(engine::I18n::StatsKills, m_player->kills)};
    lbl.alignX = Label::Alignment::Center;
    lbl.alignY = Label::Alignment::Center;
    lbl.draw(ui, m_presenter->getTrFont(), m_presenter->getViewport());
  }
}
} // namespace ui
