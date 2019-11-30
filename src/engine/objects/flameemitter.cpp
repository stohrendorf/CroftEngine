#include "flameemitter.h"

#include "engine/particle.h"
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
    getEngine().getParticles().emplace_back(m_flame);
  }
  else if(m_flame != nullptr)
  {
    removeParticle();
    getEngine().getAudioEngine().stopSound(TR1SoundId::Burning, m_flame.get());
  }
}

void FlameEmitter::removeParticle()
{
  if(m_flame == nullptr)
    return;

  const auto it = std::find_if(getEngine().getParticles().begin(),
                               getEngine().getParticles().end(),
                               [f = m_flame](const auto& p) { return f == p.get(); });
  if(it != getEngine().getParticles().end())
    getEngine().getParticles().erase(it);

  setParent(m_flame, nullptr);
  m_flame.reset();
}
} // namespace engine
