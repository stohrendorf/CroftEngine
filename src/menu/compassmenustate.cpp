#include "compassmenustate.h"

#include "engine/engine.h"
#include "engine/engineconfig.h"
#include "engine/player.h"
#include "engine/presenter.h"
#include "engine/world/world.h"
#include "hid/inputhandler.h"
#include "menudisplay.h"
#include "menuring.h"
#include "ui/detailedlevelstats.h"

#include <gsl/gsl-lite.hpp>
#include <utility>

namespace menu
{
CompassMenuState::CompassMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                   std::unique_ptr<MenuState> previous,
                                   engine::world::World& world)
    : SelectedMenuState{ringTransform}
    , m_previous{std::move(previous)}
    , m_stats{gsl::make_shared<ui::DetailedLevelStats>(world)}
{
}

std::unique_ptr<MenuState> CompassMenuState::onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display)
{
  const auto& inputHandler = world.getEngine().getPresenter().getInputHandler();
  if(inputHandler.hasDebouncedAction(hid::Action::Return))
  {
    auto& object = display.getCurrentRing().getSelectedObject();
    object.animDirection = -1_frame;
    object.goalFrame = 0_frame;
    return std::move(m_previous);
  }

  m_stats->draw(ui, world.getPresenter(), true);

  return nullptr;
}
} // namespace menu
