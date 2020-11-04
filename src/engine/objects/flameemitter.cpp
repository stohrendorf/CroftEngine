#include "flameemitter.h"

#include "engine/audioengine.h"
#include "engine/particle.h"
#include "engine/presenter.h"
#include "laraobject.h"

namespace engine::objects
{
void FlameEmitter::update()
{
  if(m_state.updateActivationTimeout())
  {
    if(m_flame != nullptr)
      return;

    m_flame = std::make_shared<FlameParticle>(m_state.position, getEngine());
    setParent(m_flame, m_state.position.room->node);
    getEngine().getObjectManager().registerParticle(m_flame);
  }
  else if(m_flame != nullptr)
  {
    removeParticle();
    getEngine().getPresenter().getAudioEngine().stopSound(TR1SoundId::Burning, m_flame.get());
  }
}

void FlameEmitter::removeParticle()
{
  if(m_flame == nullptr)
    return;

  getEngine().getObjectManager().eraseParticle(m_flame);
  m_flame.reset();
}
} // namespace engine::objects
