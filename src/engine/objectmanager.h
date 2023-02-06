#pragma once

#include "particlecollection.h"
#include "serialization/serialization_fwd.h"

#include <cstdint>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>

namespace loader::file
{
struct Item;
}

namespace engine::world
{
class World;
}

namespace engine::objects
{
class Object;
class LaraObject;
} // namespace engine::objects

namespace engine
{
enum class TR1ItemId;
class Particle;

using ObjectId = uint16_t;

class ObjectManager
{
  std::set<objects::Object*> m_scheduledDeletions;
  ObjectId m_objectCounter = 0;
  std::map<ObjectId, gslu::nn_shared<objects::Object>> m_objects;
  std::list<gslu::nn_shared<objects::Object>> m_activeObjects;
  std::set<gslu::nn_shared<objects::Object>> m_dynamicObjects;
  ParticleCollection m_particles;
  std::shared_ptr<objects::LaraObject> m_lara = nullptr;

public:
  auto& getObjects()
  {
    return m_objects;
  }

  [[nodiscard]] const auto& getObjects() const
  {
    return m_objects;
  }

  [[nodiscard]] const auto& getDynamicObjects() const
  {
    return m_dynamicObjects;
  }

  objects::LaraObject& getLara()
  {
    gsl_Expects(m_lara != nullptr);
    return *m_lara;
  }

  [[nodiscard]] const auto& getLara() const
  {
    gsl_Expects(m_lara != nullptr);
    return *m_lara;
  }

  [[nodiscard]] const auto& getLaraPtr() const
  {
    return m_lara;
  }

  void scheduleDeletion(objects::Object* object)
  {
    m_scheduledDeletions.insert(object);
  }

  void registerDynamicObject(const gslu::nn_shared<objects::Object>& object)
  {
    m_dynamicObjects.emplace(object);
  }

  [[nodiscard]] auto getDynamicObjectCount() const
  {
    return m_dynamicObjects.size();
  }

  void registerParticle(const gslu::nn_shared<Particle>& particle)
  {
    m_particles.registerParticle(particle);
  }

  void registerParticle(gslu::nn_shared<Particle>&& particle)
  {
    m_particles.registerParticle(std::move(particle));
  }

  [[nodiscard]] const auto& getParticles() const
  {
    return m_particles;
  }

  void eraseParticle(const std::shared_ptr<Particle>& particle)
  {
    m_particles.eraseParticle(particle);
  }

  void applyScheduledDeletions();
  void registerObject(const gslu::nn_shared<objects::Object>& object);
  std::shared_ptr<objects::Object> find(const objects::Object* object, bool includeDynamicObjects = false) const;
  void createObjects(world::World& world, std::vector<loader::file::Item>& items);
  [[nodiscard]] std::shared_ptr<objects::Object> getObject(ObjectId id) const;
  [[nodiscard]] auto getObjectCounter() const
  {
    return m_objectCounter;
  }
  void update(world::World& world, bool godMode);

  void replaceItems(const TR1ItemId& oldId, const TR1ItemId& newId, const world::World& world);

  void serialize(const serialization::Serializer<world::World>& ser) const;
  void deserialize(const serialization::Deserializer<world::World>& ser);

  void activate(const engine::objects::Object* object);

  void deactivate(const engine::objects::Object* object);
};
} // namespace engine
