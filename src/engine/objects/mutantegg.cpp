#include "mutantegg.h"

#include "aiagent.h"
#include "core/angle.h"
#include "core/genericvec.h"
#include "core/id.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/floordata/floordata.h"
#include "engine/items_tr1.h"
#include "engine/location.h"
#include "engine/objectmanager.h"
#include "engine/skeletalmodelnode.h"
#include "engine/world/room.h"
#include "engine/world/world.h"
#include "laraobject.h"
#include "loader/file/item.h"
#include "modelobject.h"
#include "mutant.h"
#include "object.h"
#include "objectstate.h"
#include "qs/quantity.h"
#include "render/scene/node.h"
#include "serialization/objectreference.h"
#include "serialization/serialization.h"
#include "shatter.h"

#include <bitset>
#include <boost/log/trivial.hpp>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iosfwd>
#include <map>

// IWYU pragma: no_forward_declare serialization::Serializer

namespace engine::objects
{
MutantEgg::MutantEgg(const std::string& name,
                     const gsl::not_null<world::World*>& world,
                     const gsl::not_null<const world::Room*>& room,
                     loader::file::Item item,
                     const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
    : ModelObject{name, world, room, item, true, animatedModel, false}
{
  m_state.activationState = floordata::ActivationState(uint16_t(item.activationState & ~0x3e00u));

  switch((item.activationState & 0x3e00u) >> 9u)
  {
  case 1:
    item.type = TR1ItemId::WalkingMutant1;
    if(world->findAnimatedModelForType(TR1ItemId::FlyingMutant) != nullptr)
      m_childObject = std::make_shared<WalkingMutant>(
        makeObjectName(item.type.get_as<TR1ItemId>(), 999999),
        world,
        room,
        item,
        gsl::not_null{world->findAnimatedModelForType(TR1ItemId::FlyingMutant).get()});
    break;
  case 2:
    item.type = TR1ItemId::CentaurMutant;
    if(world->findAnimatedModelForType(item.type) != nullptr)
      m_childObject = std::make_shared<CentaurMutant>(makeObjectName(item.type.get_as<TR1ItemId>(), 999999),
                                                      world,
                                                      room,
                                                      item,
                                                      gsl::not_null{world->findAnimatedModelForType(item.type).get()});
    break;
  case 4:
    item.type = TR1ItemId::TorsoBoss;
    if(world->findAnimatedModelForType(item.type) != nullptr)
      m_childObject = std::make_shared<TorsoBoss>(makeObjectName(item.type.get_as<TR1ItemId>(), 999999),
                                                  world,
                                                  room,
                                                  item,
                                                  gsl::not_null{world->findAnimatedModelForType(item.type).get()});
    break;
  case 8:
    item.type = TR1ItemId::WalkingMutant2;
    if(world->findAnimatedModelForType(TR1ItemId::FlyingMutant) != nullptr)
      m_childObject = std::make_shared<WalkingMutant>(
        makeObjectName(item.type.get_as<TR1ItemId>(), 999999),
        world,
        room,
        item,
        gsl::not_null{world->findAnimatedModelForType(TR1ItemId::FlyingMutant).get()});
    break;
  default:
    item.type = TR1ItemId::FlyingMutant;
    if(world->findAnimatedModelForType(TR1ItemId::FlyingMutant) != nullptr)
      m_childObject
        = std::make_shared<FlyingMutant>(makeObjectName(item.type.get_as<TR1ItemId>(), 999999),
                                         world,
                                         room,
                                         item,
                                         gsl::not_null{world->findAnimatedModelForType(TR1ItemId::FlyingMutant).get()});
    break;
  }

  if(m_childObject == nullptr)
  {
    BOOST_LOG_TRIVIAL(warning) << "Mutant egg does not have an object to hatch";
  }
  else
  {
    getWorld().getObjectManager().registerObject(gsl::not_null{m_childObject});
    m_childObject->m_state.triggerState = TriggerState::Invisible;
  }

  for(size_t i = 0; i < getSkeleton()->getBoneCount(); ++i)
  {
    getSkeleton()->setVisible(i, (0xff0001ffu >> i) & 1u);
  }
  getSkeleton()->rebuildMesh();
  getSkeleton()->getRenderState().setScissorTest(false);
}

void MutantEgg::update()
{
  if(m_state.goal_anim_state != 1_as)
  {
    if(m_state.activationState.isOneshot() || m_state.type == TR1ItemId::MutantEggBig
       || absMax(getWorld().getObjectManager().getLara().m_state.location.position - m_state.location.position)
            < 4096_len)
    {
      if(m_childObject != nullptr)
        BOOST_LOG_TRIVIAL(debug) << getSkeleton()->getName() << ": Hatching " << m_childObject->getNode()->getName();
      else
        BOOST_LOG_TRIVIAL(debug) << getSkeleton()->getName() << ": Hatching";
      m_state.goal_anim_state = 1_as;
      m_state.collidable = false;
      for(size_t i = 0; i < getSkeleton()->getBoneCount(); ++i)
        getSkeleton()->setVisible(i, i < 24);
      getSkeleton()->rebuildMesh();

      shatterModel(*this, 0xfffe00, 0_len);

      if(m_childObject != nullptr)
      {
        auto& childState = m_childObject->m_state;
        childState.location = m_state.location;
        childState.rotation.Y = m_state.rotation.Y;
        addChild(gsl::not_null{m_state.location.room->node}, gsl::not_null{m_childObject->getNode()});

        m_childObject->applyTransform();

        childState.touch_bits.reset();
        m_childObject->initCreatureInfo();
        m_childObject->activate();
        childState.triggerState = TriggerState::Active;
      }
    }
  }

  ModelObject::update();
}

void MutantEgg::collide(CollisionInfo& info)
{
  collideWithLara(info);
}

void MutantEgg::serialize(const serialization::Serializer<world::World>& ser) const
{
  ModelObject::serialize(ser);

  ser(S_NV("childObject", serialization::SerializingObjectReference{m_childObject}));
}

void MutantEgg::deserialize(const serialization::Deserializer<world::World>& ser)
{
  ModelObject::deserialize(ser);
  getSkeleton()->getRenderState().setScissorTest(false);

  ser << [this](const serialization::Deserializer<world::World>& ser)
  {
    ser(S_NV("childObject", serialization::DeserializingObjectReference{m_childObject}));
  };
}
} // namespace engine::objects
