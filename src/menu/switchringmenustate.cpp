#include "switchringmenustate.h"

#include "core/vec.h"
#include "inflateringmenustate.h"
#include "menudisplay.h"
#include "menuring.h"
#include "menuringtransform.h"
#include "menustate.h"
#include "util.h"

namespace menu
{
std::unique_ptr<MenuState>
  SwitchRingMenuState::onFrame(ui::Ui& /*ui*/, engine::world::World& /*world*/, MenuDisplay& display)
{
  if(m_duration != Duration)
  {
    m_duration += 1_rframe;
    m_ringTransform->radius -= m_radiusSpeed;
    m_ringTransform->cameraPos.Z = MenuRingTransform::CameraZPosOffset + m_ringTransform->radius;
    m_ringTransform->ringRotation -= 180_deg / Duration * 1_rframe;
    m_ringTransform->cameraRotX = exactScale(m_targetCameraRotX, m_duration, Duration);
    return nullptr;
  }

  display.currentRingIndex = m_next;
  m_ringTransform->cameraRotX = m_targetCameraRotX;
  m_ringTransform->cameraPos.Y *= -1;

  return create<InflateRingMenuState>(false);
}

void SwitchRingMenuState::handleObject(ui::Ui& /*ui*/,
                                       engine::world::World& world,
                                       MenuDisplay& display,
                                       MenuObject& object)
{
  if(&object == &display.getCurrentRing().getSelectedObject())
  {
    idleRotation(world, object, false);
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

void SwitchRingMenuState::begin(engine::world::World& /*world*/)
{
  m_radiusSpeed = m_ringTransform->radius / Duration * 1_rframe;
  m_targetCameraRotX = m_down ? -MenuRingTransform::CameraSwitchRingXRot : MenuRingTransform::CameraSwitchRingXRot;
}
} // namespace menu
