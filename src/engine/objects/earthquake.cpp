#include "earthquake.h"

#include "core/units.h"
#include "engine/audioengine.h"
#include "engine/cameracontroller.h"
#include "engine/soundeffects_tr1.h"
#include "engine/world/world.h"
#include "objectstate.h"
#include "qs/quantity.h"
#include "util/helpers.h"

namespace engine::objects
{
void Earthquake::update()
{
  if(!m_state.updateActivationTimeout())
    return;

  if(util::rand15() < 256)
  {
    getWorld().getCameraController().setBounce(-150_len);
    getWorld().getAudioEngine().playSoundEffect(TR1SoundEffect::Boulder, nullptr);
  }
  else if(util::rand15() < 1024)
  {
    getWorld().getCameraController().setBounce(50_len);
    getWorld().getAudioEngine().playSoundEffect(TR1SoundEffect::BigFloorImpact, nullptr);
  }
}
} // namespace engine::objects
