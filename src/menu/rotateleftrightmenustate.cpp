#include "rotateleftrightmenustate.h"

#include "core/units.h"
#include "menudisplay.h"
#include "menuring.h"
#include "menuringtransform.h"
#include "menustate.h"
#include "util.h"

#include <cstddef>
#include <limits>
#include <memory>
#include <utility>

namespace menu
{
void RotateLeftRightMenuState::handleObjectTick(engine::world::World& /*world*/,
                                                MenuDisplay& /*display*/,
                                                MenuObject& object)
{
  zeroRotation(object, 512_au);
}

RotateLeftRightMenuState::RotateLeftRightMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                                   const bool left,
                                                   const MenuRing& ring,
                                                   std::unique_ptr<MenuState>&& prev)
    : MenuState{ringTransform}
    , m_targetObject{ring.currentObject + (left ? -1 : 1)}
    , m_rotSpeed{(left ? -ring.getAnglePerItem() : ring.getAnglePerItem()) / Duration / 1_frame * 1_tick}
    , m_prev{std::move(prev)}
{
  if(m_targetObject == std::numeric_limits<size_t>::max())
  {
    m_targetObject = ring.list.size() - 1;
  }
  if(m_targetObject >= ring.list.size())
  {
    m_targetObject = 0;
  }
}

std::unique_ptr<MenuState> RotateLeftRightMenuState::tick(engine::world::World& /*world*/, MenuDisplay& display)
{
  m_ringTransform->ringRotation += m_rotSpeed * 1_frame;
  m_duration -= 1_tick;
  if(m_duration != 0_tick)
    return nullptr;

  display.getCurrentRing().currentObject = m_targetObject;
  m_ringTransform->ringRotation = display.getCurrentRing().getCurrentObjectAngle();
  return std::move(m_prev);
}

void RotateLeftRightMenuState::constructUi(ui::Ui& /*ui*/, engine::world::World& /*world*/, MenuDisplay& /*display*/)
{
}
} // namespace menu
