#include "dart.h"

#include "core/angle.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/heightinfo.h"
#include "engine/location.h"
#include "engine/objectmanager.h"
#include "engine/particle.h"
#include "engine/raycast.h"
#include "engine/world/room.h"
#include "engine/world/world.h"
#include "laraobject.h"
#include "modelobject.h"
#include "objectstate.h"
#include "qs/quantity.h"
#include "render/scene/node.h"

#include <bitset>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <utility>

namespace engine::objects
{
void Dart::collide(CollisionInfo& info)
{
  collideWithLara(info);
}

void Dart::update()
{
  if(m_state.touch_bits != 0)
  {
    getWorld().hitLara(50_hp);

    auto fx = createBloodSplat(getWorld(), m_state.location, m_state.speed, m_state.rotation.Y);
    getWorld().getObjectManager().registerParticle(fx);
  }

  const auto oldLocation = m_state.location;

  ModelObject::update();
  const auto sector = m_state.location.updateRoom();
  setCurrentRoom(m_state.location.room);

  m_state.floor
    = HeightInfo::fromFloor(sector, m_state.location.position, getWorld().getObjectManager().getObjects()).y;

  if(m_state.location.position.Y < m_state.floor)
    return;

  kill();

  const auto ricochetPos
    = std::get<1>(raycastLineOfSight(oldLocation, m_state.location.position, getWorld().getObjectManager()));

  auto particle = gsl::make_shared<RicochetParticle>(ricochetPos, getWorld());
  setParent(particle, ricochetPos.room->node);
  particle->angle = m_state.rotation;
  particle->timePerSpriteFrame = 6;
  getWorld().getObjectManager().registerParticle(std::move(particle));
}
} // namespace engine::objects
