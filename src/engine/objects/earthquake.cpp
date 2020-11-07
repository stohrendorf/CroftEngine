#include "earthquake.h"

#include "engine/audioengine.h"
#include "engine/engine.h"
#include "engine/presenter.h"

namespace engine::objects
{
void Earthquake::update()
{
  if(!m_state.updateActivationTimeout())
    return;

  if(util::rand15() < 256)
  {
    getEngine().getCameraController().setBounce(-150_len);
    getEngine().getPresenter().getAudioEngine().playSoundEffect(TR1SoundEffect::RollingBall, nullptr);
  }
  else if(util::rand15() < 1024)
  {
    getEngine().getCameraController().setBounce(50_len);
    getEngine().getPresenter().getAudioEngine().playSoundEffect(TR1SoundEffect::TRexFootstep, nullptr);
  }
}
} // namespace engine::objects
