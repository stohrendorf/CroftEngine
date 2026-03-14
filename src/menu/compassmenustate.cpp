#include "compassmenustate.h"

#include "core/units.h"
#include "engine/engine.h"
#include "engine/presenter.h"
#include "engine/world/world.h"
#include "hid/actions.h"
#include "hid/inputhandler.h"
#include "menudisplay.h"
#include "menuobject.h"
#include "menuring.h"
#include "qs/quantity.h"
#include "selectedmenustate.h"
#include "ui/detailedlevelstats.h"

#include <gsl-lite/gsl-lite.hpp>
#include <memory>
#include <utility>

namespace menu
{
CompassMenuState::CompassMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                   std::unique_ptr<MenuState> previous,
                                   engine::world::World& world)
    : SelectedMenuState{ringTransform}
    , m_previous{std::move(previous)}
    , m_stats{gsl_lite::make_shared<ui::DetailedLevelStats>(world)}
{
}

std::unique_ptr<MenuState> CompassMenuState::tick(engine::world::World& world, MenuDisplay& display)
{
  if(world.getEngine().getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Return))
  {
    auto& object = display.getCurrentRing().getSelectedObject();
    object.animDirection = -1_mframe;
    object.goalFrame = 0_mframe;
    return std::move(m_previous);
  }

  return nullptr;
}

void CompassMenuState::constructUi(ui::Ui& ui, engine::world::World& world, MenuDisplay& /*display*/)
{
  m_stats->draw(ui, world.getEngine().getPresenter(), true);
}
} // namespace menu