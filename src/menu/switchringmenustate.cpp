#include "switchringmenustate.h"

#include "inflateringmenustate.h"
#include "menudisplay.h"
#include "menuring.h"
#include "menuringtransform.h"
#include "util.h"

namespace menu
{
std::unique_ptr<MenuState>
  SwitchRingMenuState::onFrame(gl::Image<gl::SRGBA8>& /*img*/, engine::World& /*world*/, MenuDisplay& display)
{
  if(m_duration != Duration)
  {
    m_duration += 1_frame;
    m_ringTransform->radius -= m_radiusSpeed;
    m_ringTransform->ringRotation -= 180_deg / Duration * 1_frame;
    m_ringTransform->cameraRotX = exactScale(m_targetCameraRotX, m_duration, Duration);
    return nullptr;
  }

  display.currentRingIndex = m_next;
  m_ringTransform->cameraRotX = m_targetCameraRotX;

  return create<InflateRingMenuState>();
}

void SwitchRingMenuState::handleObject(engine::World& world, MenuDisplay& display, MenuObject& object)
{
  if(&object == &display.getCurrentRing().getSelectedObject())
  {
    display.updateMenuObjectDescription(world, object);
    idleRotation(world, object);
  }
  else
  {
    zeroRotation(object, 256_au);
  }
}

SwitchRingMenuState::SwitchRingMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                         size_t next,
                                         bool down)
    : MenuState{ringTransform}
    , m_next{next}
    , m_down{down}
{
}

void SwitchRingMenuState::begin(engine::World& /*world*/)
{
  m_radiusSpeed = m_ringTransform->radius / Duration * 1_frame;
  m_targetCameraRotX = m_down ? -45_deg : 45_deg;
}
} // namespace menu
