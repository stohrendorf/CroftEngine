#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <utility>

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
  LevelStats(std::string title,
             size_t totalSecrets,
             std::shared_ptr<engine::Player> player,
             std::shared_ptr<engine::Presenter> presenter)
      : m_title{std::move(title)}
      , m_totalSecrets{totalSecrets}
      , m_player{std::move(player)}
      , m_presenter{std::move(presenter)}
  {
  }

  void draw(Ui& ui) const;

private:
  std::string m_title;
  size_t m_totalSecrets;
  std::shared_ptr<engine::Player> m_player;
  std::shared_ptr<engine::Presenter> m_presenter;
};
} // namespace ui
