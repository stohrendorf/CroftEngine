#include "earthquake.h"

#include "engine/engine.h"

namespace engine::objects
{
void Earthquake::update()
{
  if(!m_state.updateActivationTimeout())
    return;

  if(util::rand15() < 256)
  {
    getEngine().getCameraController().setBounce(-150_len);
    getEngine().getAudioEngine().playSound(TR1SoundId::RollingBall, nullptr);
  }
  else if(util::rand15() < 1024)
  {
    getEngine().getCameraController().setBounce(50_len);
    getEngine().getAudioEngine().playSound(TR1SoundId::TRexFootstep, nullptr);
  }
}
} // namespace engine::objects
