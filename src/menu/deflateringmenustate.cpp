#include "deflateringmenustate.h"

#include "core/vec.h"
#include "menudisplay.h"
#include "menuring.h"
#include "menuringtransform.h"
#include "menustate.h"
#include "util.h"

#include <utility>

namespace menu
{
void DeflateRingMenuState::handleObject(ui::Ui& /*ui*/,
                                        engine::world::World& world,
                                        MenuDisplay& display,
                                        MenuObject& object)
{
  if(&object == &display.getCurrentRing().getSelectedObject())
  {
    zeroRotation(object, 256_au);
  }
  else
  {
    idleRotation(world, object, false);
  }
}

std::unique_ptr<MenuState>
  DeflateRingMenuState::onFrame(ui::Ui& /*ui*/, engine::world::World& /*world*/, MenuDisplay& /*display*/)
{
  if(m_duration == 0_rframe)
    return std::move(m_next);

  m_duration -= 1_rframe;
  m_ringTransform->ringRotation -= 180_deg / Duration * 1_rframe;
  m_ringTransform->radius = exactScale(m_initialRadius, m_duration, Duration);
  m_ringTransform->cameraPos.Y += m_cameraSpeedY;
  m_ringTransform->cameraPos.Z = exactScale(m_targetPosZ, Duration - m_duration, Duration)
                                 + MenuRingTransform::CameraZPosOffset + m_ringTransform->radius;
  m_ringTransform->cameraRotX
    = m_initialCameraRotX + exactScale(m_targetRotX - m_initialCameraRotX, Duration - m_duration - 1_rframe, Duration);
  return nullptr;
}

DeflateRingMenuState::DeflateRingMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                           Direction direction,
                                           std::unique_ptr<MenuState> next)
    : MenuState{ringTransform}
    , m_next{std::move(next)}
    , m_targetPosY{direction == Direction::Down ? -MenuRingTransform::RingOffScreenCameraPosY
                   : direction == Direction::Up ? MenuRingTransform::RingOffScreenCameraPosY
                                                : 0_len}
    , m_targetPosZ{direction == Direction::Backpack ? MenuRingTransform::CameraBackpackZPos : 0_len}
    , m_targetRotX{direction == Direction::Backpack ? MenuRingTransform::CameraBackpackXRot : 0_deg}
{
}

void DeflateRingMenuState::begin(engine::world::World& /*world*/)
{
  // TODO fadeOutInventory(mode != InventoryMode::TitleMode);
  m_initialRadius = m_ringTransform->radius;
  m_initialCameraRotX = m_ringTransform->cameraRotX;
  m_cameraSpeedY = (m_targetPosY - m_ringTransform->cameraPos.Y) / Duration * 1_rframe;
}
} // namespace menu
