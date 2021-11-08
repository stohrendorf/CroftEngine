#pragma once

#include "core/id.h"
#include "core/units.h"
#include "engine/floordata/floordata.h"
#include "modelobject.h"
#include "objectstate.h"

namespace engine
{
struct CollisionInfo;
struct Location;
} // namespace engine

namespace engine::world
{
class World;
struct Room;
struct SkeletalModelType;
} // namespace engine::world

namespace loader::file
{
struct Item;
}

namespace engine::objects
{
class Switch : public ModelObject
{
public:
  MODELOBJECT_DEFAULT_CONSTRUCTORS(Switch, true, false)

  void collide(CollisionInfo& collisionInfo) override;

  void update() final
  {
    m_state.activationState.fullyActivate();
    if(!m_state.updateActivationTimeout())
    {
      // reset switch if a timeout is active
      m_state.goal_anim_state = 1_as;
      m_state.timer = 0_rframe;
    }

    ModelObject::update();
  }
};
} // namespace engine::objects
