#pragma once

#include <memory>
#include <string>

namespace engine
{
class Player;
class Presenter;
} // namespace engine

namespace ui
{
class Ui;

class LevelStats
{
public:
  LevelStats(const std::string& title,
             size_t totalSecrets,
             const std::shared_ptr<engine::Player>& player,
             const std::shared_ptr<engine::Presenter>& presenter)
      : m_title{title}
      , m_totalSecrets{totalSecrets}
      , m_player{player}
      , m_presenter{presenter}
  {
  }

  void draw(Ui& ui) const;

private:
  const std::string m_title;
  const size_t m_totalSecrets;
  const std::shared_ptr<engine::Player> m_player;
  const std::shared_ptr<engine::Presenter> m_presenter;
};
} // namespace ui
