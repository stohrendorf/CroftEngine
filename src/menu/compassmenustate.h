#pragma once

#include "menu/menustate.h"
#include "selectedmenustate.h"

#include <gslu.h>
#include <memory>

namespace engine::world
{
class World;
}

namespace ui
{
class Ui;
class DetailedLevelStats;
} // namespace ui

namespace menu
{
struct MenuRingTransform;
struct MenuDisplay;

class CompassMenuState : public SelectedMenuState
{
private:
  std::unique_ptr<MenuState> m_previous;
  gslu::nn_shared<ui::DetailedLevelStats> m_stats;

public:
  explicit CompassMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                            std::unique_ptr<MenuState> previous,
                            engine::world::World& world);
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
};
} // namespace menu
