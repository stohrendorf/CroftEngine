#include "waterfallmist.h"

#include "core/magic.h"
#include "core/vec.h"
#include "engine/floordata/floordata.h"
#include "engine/location.h"
#include "engine/objectmanager.h"
#include "engine/particle.h"
#include "engine/world/room.h"
#include "engine/world/world.h"
#include "laraobject.h"
#include "objectstate.h"
#include "qs/quantity.h"
#include "render/scene/node.h"

#include <gslu.h>
#include <memory>

namespace engine::objects
{
void WaterfallMist::update()
{
  if(!m_state.activationState.isFullyActivated())
    return;

  const auto d = m_state.location.position - getWorld().getObjectManager().getLara().m_state.location.position;
  if(abs(d.X) > 20 * core::SectorSize || abs(d.Y) > 20 * core::SectorSize || abs(d.Z) > 20 * core::SectorSize)
    return;

  auto particle = gslu::make_nn_shared<SplashParticle>(m_state.location, getWorld(), true);
  setParent(particle, m_state.location.room->node);
  getWorld().getObjectManager().registerParticle(particle);
}
} // namespace engine::objects
