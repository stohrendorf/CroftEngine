#include "lavaparticleemitter.h"

#include "engine/particle.h"
#include "laraobject.h"

namespace engine::objects
{
void LavaParticleEmitter::update()
{
  auto particle = std::make_shared<LavaParticle>(m_state.position, getEngine());
  setParent(particle, m_state.position.room->node);
  getEngine().getParticles().emplace_back(particle);

  playSoundEffect(TR1SoundId::ChoppyWater);
}
} // namespace engine
