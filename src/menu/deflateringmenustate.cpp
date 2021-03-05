#include "deflateringmenustate.h"

#include "menudisplay.h"
#include "menuring.h"
#include "util.h"

namespace menu
{
void DeflateRingMenuState::handleObject(ui::Ui& ui, engine::World& world, MenuDisplay& display, MenuObject& object)
{
  if(&object == &display.getCurrentRing().getSelectedObject())
  {
    display.updateMenuObjectDescription(ui, world, object);
    zeroRotation(object, 256_au);
  }
  else
  {
    idleRotation(world, object);
  }
}

std::unique_ptr<MenuState>
  DeflateRingMenuState::onFrame(ui::Ui& /*ui*/, engine::World& /*world*/, MenuDisplay& /*display*/)
{
  if(m_duration == 0_frame)
    return std::move(m_next);

  m_duration -= 1_frame;
  m_ringTransform->ringRotation -= 180_deg / Duration * 1_frame;
  m_ringTransform->radius = exactScale(m_initialRadius, m_duration, Duration);
  m_ringTransform->cameraPos.Y += m_cameraSpeedY;
  return nullptr;
}

DeflateRingMenuState::DeflateRingMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                           bool down,
                                           std::unique_ptr<MenuState> next)
    : MenuState{ringTransform}
    , m_next{std::move(next)}
    , m_target{down ? -1536_len : 1536_len}
{
}

void DeflateRingMenuState::begin(engine::World& /*world*/)
{
  // TODO fadeOutInventory(mode != InventoryMode::TitleMode);
  m_initialRadius = m_ringTransform->radius;
  m_cameraSpeedY = (m_target - m_ringTransform->cameraPos.Y) / Duration * 1_frame;
}
} // namespace menu
