#include "rotateleftrightmenustate.h"

#include "menudisplay.h"
#include "menuring.h"
#include "util.h"

namespace menu
{
void RotateLeftRightMenuState::handleObject(ui::Ui& /*ui*/,
                                            engine::world::World& /*world*/,
                                            MenuDisplay& /*display*/,
                                            MenuObject& object)
{
  zeroRotation(object, 512_au);
}

RotateLeftRightMenuState::RotateLeftRightMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                                   bool left,
                                                   const MenuRing& ring,
                                                   std::unique_ptr<MenuState>&& prev)
    : MenuState{ringTransform}
    , m_targetObject{ring.currentObject + (left ? -1 : 1)}
    , m_rotSpeed{(left ? -ring.getAnglePerItem() : ring.getAnglePerItem()) / Duration}
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

std::unique_ptr<MenuState>
  RotateLeftRightMenuState::onFrame(ui::Ui& /*ui*/, engine::world::World& /*world*/, MenuDisplay& display)
{
  m_ringTransform->ringRotation += m_rotSpeed * 1_frame;
  m_duration -= 1_frame;
  if(m_duration != 0_frame)
    return nullptr;

  display.getCurrentRing().currentObject = m_targetObject;
  m_ringTransform->ringRotation = display.getCurrentRing().getCurrentObjectAngle();
  return std::move(m_prev);
}
} // namespace menu
