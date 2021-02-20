#include "resetitemtransformmenustate.h"

#include "engine/audioengine.h"
#include "engine/world.h"
#include "menudisplay.h"
#include "menuring.h"
#include "util.h"

namespace menu
{
void ResetItemTransformMenuState::handleObject(ui::Ui& ui,
                                               engine::World& world,
                                               MenuDisplay& display,
                                               MenuObject& object)
{
  if(&object == &display.getCurrentRing().getSelectedObject())
  {
    display.updateMenuObjectDescription(ui, world, object);
    object.baseRotationX = exactScale(object.selectedBaseRotationX, m_duration, Duration);
    object.rotationX = exactScale(object.selectedRotationX, m_duration, Duration);
    object.positionZ = exactScale(object.selectedPositionZ, m_duration, Duration);
  }
  else
  {
    zeroRotation(object, 256_au);
  }
}

std::unique_ptr<MenuState>
  ResetItemTransformMenuState::onFrame(ui::Ui& /*ui*/, engine::World& /*world*/, MenuDisplay& /*display*/)
{
  if(m_duration != 0_frame)
  {
    m_duration -= 1_frame;
    return nullptr;
  }

  return std::move(m_next);
}

void ResetItemTransformMenuState::begin(engine::World& world)
{
  world.getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuOptionEscape, nullptr);
}
} // namespace menu
