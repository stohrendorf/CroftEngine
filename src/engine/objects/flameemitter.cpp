#include "flameemitter.h"

#include "engine/audioengine.h"
#include "engine/location.h"
#include "engine/objectmanager.h"
#include "engine/particle.h"
#include "engine/soundeffects_tr1.h"
#include "engine/world/room.h"
#include "engine/world/world.h"
#include "objectstate.h"
#include "render/scene/node.h"

namespace engine::objects
{
void FlameEmitter::update()
{
  if(m_state.updateActivationTimeout())
  {
    if(m_flame != nullptr)
      return;

    m_flame = std::make_shared<FlameParticle>(m_state.location, getWorld());
    setParent(m_flame, m_state.location.room->node);
    getWorld().getObjectManager().registerParticle(m_flame);
  }
  else if(m_flame != nullptr)
  {
    removeParticle();
    getWorld().getAudioEngine().stopSoundEffect(TR1SoundEffect::Burning, m_flame.get());
  }
}

void FlameEmitter::removeParticle()
{
  if(m_flame == nullptr)
    return;

  getWorld().getObjectManager().eraseParticle(m_flame);
  m_flame.reset();
}
} // namespace engine::objects
