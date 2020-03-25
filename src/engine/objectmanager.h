#pragma once
#include "items_tr1.h"

#include <boost/throw_exception.hpp>
#include <gsl-lite.hpp>
#include <map>
#include <set>
#include <vector>

namespace serialization
{
class Serializer;
}

namespace loader::file
{
struct Item;
}

namespace engine
{
namespace objects
{
class Object;
class LaraObject;
} // namespace objects

class Engine;
class Particle;

using ObjectId = uint16_t;

class ObjectManager
{
  std::set<objects::Object*> m_scheduledDeletions;
  ObjectId m_objectCounter = -1;
  std::map<ObjectId, gsl::not_null<std::shared_ptr<objects::Object>>> m_objects;
  std::set<gsl::not_null<std::shared_ptr<objects::Object>>> m_dynamicObjects;
  std::vector<gsl::not_null<std::shared_ptr<Particle>>> m_particles;
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

  [[nodiscard]] const auto& getParticles() const
  {
    return m_particles;
  }

  objects::LaraObject& getLara()
  {
    Expects(m_lara != nullptr);
    return *m_lara;
  }

  [[nodiscard]] const auto& getLara() const
  {
    Expects(m_lara != nullptr);
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

  void registerDynamicObject(const gsl::not_null<std::shared_ptr<objects::Object>>& object)
  {
    m_dynamicObjects.emplace(object);
  }

  void registerParticle(const gsl::not_null<std::shared_ptr<Particle>>& particle)
  {
    m_particles.emplace_back(particle);
  }

  void eraseParticle(const std::shared_ptr<Particle>& particle);

  void applyScheduledDeletions();
  void registerObject(const gsl::not_null<std::shared_ptr<objects::Object>>& object);
  std::shared_ptr<objects::Object> find(const objects::Object* object) const;
  void createObjects(Engine& engine, std::vector<loader::file::Item>& items);
  [[nodiscard]] std::shared_ptr<objects::Object> getObject(ObjectId id) const;
  void update(Engine& engine, bool godMode);

  void serialize(const serialization::Serializer& ser);
};
} // namespace engine