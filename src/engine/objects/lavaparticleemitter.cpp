#include "lavaparticleemitter.h"

#include "engine/particle.h"
#include "laraobject.h"

namespace engine::objects
{
void LavaParticleEmitter::update()
{
  auto particle = std::make_shared<LavaParticle>(m_state.position, getEngine());
  setParent(particle, m_state.position.room->node);
  getEngine().getObjectManager().registerParticle(particle);

  playSoundEffect(TR1SoundId::ChoppyWater);
}

void LavaParticleEmitter::serialize(const serialization::Serializer& ser)
{
  ModelObject::serialize(ser);
  getSkeleton()->setRenderable(nullptr);
  getSkeleton()->removeAllChildren();
  getSkeleton()->clearParts();
}
} // namespace engine::objects
