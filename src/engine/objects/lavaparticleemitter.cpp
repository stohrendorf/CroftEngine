#include "lavaparticleemitter.h"

#include "engine/location.h"
#include "engine/objectmanager.h"
#include "engine/particle.h"
#include "engine/soundeffects_tr1.h"
#include "engine/world/room.h"
#include "engine/world/world.h"
#include "objectstate.h"
#include "render/scene/node.h"

#include <gsl-lite/gsl-lite.hpp>

namespace engine::objects
{
void LavaParticleEmitter::update()
{
  const auto particle = gsl_lite::make_shared<LavaParticle>(m_state.location, getWorld());
  setParent(particle, m_state.location.room->node);
  getWorld().getObjectManager().registerParticle(particle);

  playSoundEffect(TR1SoundEffect::ChoppyWater);
}
} // namespace engine::objects