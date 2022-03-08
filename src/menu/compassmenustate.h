#pragma once

#include "menu/menustate.h"
#include "selectedmenustate.h"

#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <memory>

namespace engine::world
{
class World;
}

namespace ui
{
class Ui;
}

namespace ui::widgets
{
class GridBox;
class GroupBox;
} // namespace ui::widgets

namespace menu
{
struct MenuRingTransform;

class CompassMenuState : public SelectedMenuState
{
private:
  std::unique_ptr<MenuState> m_previous;
  gslu::nn_shared<ui::widgets::GridBox> m_grid;
  gslu::nn_shared<ui::widgets::GroupBox> m_container;

public:
  explicit CompassMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                            std::unique_ptr<MenuState> previous,
                            engine::world::World& world);
  std::unique_ptr<MenuState> onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display) override;
};
} // namespace menu
