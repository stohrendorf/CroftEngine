#include "earthquake.h"

#include "engine/audioengine.h"
#include "engine/cameracontroller.h"
#include "engine/presenter.h"
#include "engine/world/world.h"

namespace engine::objects
{
void Earthquake::update()
{
  if(!m_state.updateActivationTimeout())
    return;

  if(util::rand15() < 256)
  {
    getWorld().getCameraController().setBounce(-150_len);
    getWorld().getAudioEngine().playSoundEffect(TR1SoundEffect::RollingBall, nullptr);
  }
  else if(util::rand15() < 1024)
  {
    getWorld().getCameraController().setBounce(50_len);
    getWorld().getAudioEngine().playSoundEffect(TR1SoundEffect::TRexFootstep, nullptr);
  }
}
} // namespace engine::objects
