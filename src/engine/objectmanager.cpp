#include "objectmanager.h"

#include "core/id.h"
#include "core/magic.h"
#include "core/units.h"
#include "items_tr1.h"
#include "loader/file/item.h"
#include "objects/laraobject.h"
#include "objects/object.h"
#include "objects/objectfactory.h"
#include "objects/objectstate.h"
#include "objects/pickupobject.h"
#include "particlecollection.h"
#include "render/scene/node.h"
#include "serialization/map.h"
#include "serialization/not_null.h"
#include "serialization/objectreference.h" // IWYU pragma: keep
#include "serialization/serialization.h"
#include "serialization/vector.h"
#include "world/room.h"
#include "world/sprite.h"
#include "world/world.h"

#include <algorithm>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/throw_exception.hpp>
#include <cstdint>
#include <functional>
#include <gsl-lite/gsl-lite.hpp>
#include <gslu.h>
#include <limits>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <utility>
#include <vector>

namespace engine
{
void ObjectManager::createObjects(world::World& world, std::vector<loader::file::Item>& items)
{
  gsl_Expects(m_objectCounter == 0);
  m_objectCounter = gsl_lite::narrow<ObjectId>(items.size());

  m_lara = nullptr;
  for(const auto& idItem : items | boost::adaptors::indexed())
  {
    auto object = objects::createObject(world, idItem.value(), idItem.index());
    if(idItem.value().type == TR1ItemId::Lara)
    {
      if(m_lara != nullptr)
      {
        m_lara->deactivate();
        m_lara->getNode()->setVisible(false);
      }
      m_lara = std::dynamic_pointer_cast<objects::LaraObject>(object);
      gsl_Assert(m_lara != nullptr);
    }

    if(object == nullptr)
      continue;

    m_objects.emplace(gsl_lite::narrow<ObjectId>(idItem.index()), object);
    if(object->isActive())
    {
      object->activate();
    }
  }
}

void ObjectManager::applyScheduledDeletions()
{
  if(m_scheduledDeletions.empty())
    return;

  for(const auto& del : m_scheduledDeletions)
  {
    deactivate(del);

    if(auto it = std::ranges::find_if(m_dynamicObjects,
                                      [del](const std::shared_ptr<objects::Object>& i) noexcept
                                      {
                                        return i.get() == del;
                                      });
       it != m_dynamicObjects.end())
    {
      m_dynamicObjects.erase(it);
      continue;
    }

    if(auto it = std::ranges::find_if(m_objects,
                                      [del](const std::pair<uint16_t, gslu::nn_shared<objects::Object>>& i)
                                      {
                                        return i.second.get().get() == del;
                                      });
       it != m_objects.end())
    {
      m_objects.erase(it);
      continue;
    }
  }

  m_scheduledDeletions.clear();
}

void ObjectManager::registerObject(const gslu::nn_shared<objects::Object>& object)
{
  if(m_objectCounter == std::numeric_limits<ObjectId>::max())
    BOOST_THROW_EXCEPTION(std::runtime_error("Artificial object counter exceeded"));

  m_objects.emplace(m_objectCounter++, object);
}

std::shared_ptr<objects::Object> ObjectManager::find(const objects::Object* object,
                                                     const bool includeDynamicObjects) const
{
  if(object == nullptr)
    return nullptr;

  const auto it = std::ranges::find_if(m_objects,
                                       [object](const std::pair<uint16_t, gslu::nn_shared<objects::Object>>& x)
                                       {
                                         return x.second.get().get() == object;
                                       });

  if(it != m_objects.end())
    return it->second;

  if(includeDynamicObjects)
  {
    const auto it2 = std::ranges::find_if(m_dynamicObjects,
                                          [object](const auto& x)
                                          {
                                            return x.get().get() == object;
                                          });
    if(it2 != m_dynamicObjects.end())
      return *it2;
  }

  return nullptr;
}

std::shared_ptr<objects::Object> ObjectManager::getObject(const ObjectId id) const
{
  const auto it = m_objects.find(id);
  if(it == m_objects.end())
    return nullptr;

  return it->second.get();
}

void ObjectManager::update(world::World& world, const bool godMode)
{
  for(const auto& object : m_dynamicObjects)
  {
    object->getNode()->setVisible(object->m_state.triggerState != objects::TriggerState::Invisible);
    object->updateLighting();
  }

  for(const auto& object : m_objects | std::views::values)
  {
    object->getNode()->setVisible(object->m_state.triggerState != objects::TriggerState::Invisible);
    object->updateLighting();
  }

  // need to work on a copy because update() may modify the collection
  for(const auto activeObjects = m_activeObjects; const auto& object : activeObjects)
  {
    if(object.get() == m_lara) // Lara is special and needs to be updated last
      continue;
    object->update();
  }

  m_particles.update(world);
  for(auto& room : world.getRooms())
  {
    room.particles.update(world);
  }

  if(m_lara != nullptr)
  {
    if(godMode && !m_lara->isDead())
      m_lara->m_state.health = core::LaraHealth;
    m_lara->update();
    m_lara->updateLighting();
  }

  applyScheduledDeletions();
}

void ObjectManager::serialize(const serialization::Serializer<world::World>& ser) const
{
  ser(S_NV("objectCounter", m_objectCounter),
      S_NV("objects", m_objects),
      S_NV("lara", serialization::ObjectReference{std::cref(m_lara)}));

  std::vector<ObjectId> activeObjectIds;
  for(const auto& obj : m_activeObjects)
  {
    auto it = std::ranges::find_if(m_objects,
                                   [obj](const auto& i)
                                   {
                                     return i.second.get() == obj;
                                   });
    if(it != m_objects.end())
    {
      activeObjectIds.emplace_back(it->first);
    }
  }
  ser(S_NV("activeObjects", activeObjectIds));
}

void ObjectManager::deserialize(const serialization::Deserializer<world::World>& ser)
{
  ser(S_NV("objectCounter", m_objectCounter),
      S_NV("objects", m_objects),
      S_NV("lara", serialization::ObjectReference{std::ref(m_lara)}));

  std::vector<ObjectId> activeObjectIds;
  ser(S_NV("activeObjects", activeObjectIds));
  m_activeObjects.clear();
  for(const auto id : activeObjectIds)
  {
    m_activeObjects.emplace_back(m_objects.at(id));
  }
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void ObjectManager::replaceItems(const TR1ItemId& oldId, const TR1ItemId& newId, const world::World& world)
{
  for(const auto& obj : m_objects | std::views::values)
  {
    if(obj->m_state.type != oldId)
      continue;

    const gsl_lite::not_null pickup{std::dynamic_pointer_cast<objects::PickupObject>(obj.get())};
    const auto& spriteSequence = world.getWorldGeometry().findSpriteSequenceForType(newId);
    gsl_Assert(spriteSequence != nullptr && !spriteSequence->sprites.empty());
    pickup->replace(newId, gsl_lite::not_null{spriteSequence->sprites.data()});
  }
}

void ObjectManager::deactivate(const objects::Object* object)
{
  if(const auto it = std::ranges::find_if(m_activeObjects,
                                          [object](const auto& i)
                                          {
                                            return i.get().get() == object;
                                          });
     it != m_activeObjects.end())
  {
    m_activeObjects.erase(it);
  }
}

void ObjectManager::activate(const objects::Object* object)
{
  const auto ob = find(object, true);
  if(ob == nullptr)
  {
    return;
  }

  if(const auto it = std::ranges::find(m_activeObjects, gsl_lite::not_null{ob}); it == m_activeObjects.end())
  {
    m_activeObjects.emplace_front(ob);
  }
}
} // namespace engine