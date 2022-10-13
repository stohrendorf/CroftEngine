#pragma once

#include <gslu.h>

namespace ui::widgets
{
class GridBox;
class GroupBox;
} // namespace ui::widgets

namespace engine
{
class Presenter;
}

namespace engine::world
{
class World;
}

namespace ui
{
class Ui;

class DetailedLevelStats final
{
public:
  explicit DetailedLevelStats(const engine::world::World& world);
  ~DetailedLevelStats();
  void draw(Ui& ui, const engine::Presenter& presenter, bool compass) const;

private:
  gslu::nn_shared<widgets::GridBox> m_grid;
  gslu::nn_shared<widgets::GroupBox> m_container;
};
} // namespace ui
