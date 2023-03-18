#include "waterfallmist.h"

#include "core/genericvec.h"
#include "core/magic.h"
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

#include <memory>

namespace engine::objects
{
void WaterfallMist::update()
{
  if(!m_state.activationState.isFullyActivated())
    return;

  const auto d = m_state.location.position - getWorld().getObjectManager().getLara().m_state.location.position;
  if(abs(d.X) > 20_sectors || abs(d.Y) > 20_sectors || abs(d.Z) > 20_sectors)
    return;

  auto particle = gsl::make_shared<SplashParticle>(m_state.location, getWorld(), true);
  setParent(particle, m_state.location.room->node);
  getWorld().getObjectManager().registerParticle(particle);
}
} // namespace engine::objects
