#include "lavaparticleemitter.h"

#include "engine/particle.h"
#include "engine/soundeffects_tr1.h"
#include "engine/world/world.h"

namespace engine::objects
{
void LavaParticleEmitter::update()
{
  auto particle = std::make_shared<LavaParticle>(m_state.location, getWorld());
  setParent(particle, m_state.location.room->node);
  getWorld().getObjectManager().registerParticle(particle);

  playSoundEffect(TR1SoundEffect::ChoppyWater);
}
} // namespace engine::objects
