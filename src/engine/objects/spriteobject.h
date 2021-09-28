#pragma once

#include "core/boundingbox.h"
#include "core/units.h"
#include "object.h"
#include "render/scene/node.h"

#include <boost/throw_exception.hpp>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <stdexcept>
#include <string>

namespace engine
{
struct Location;
}

namespace loader::file
{
struct Item;
}

namespace engine::world
{
class World;
struct Sprite;
struct Room;
} // namespace engine::world

namespace engine::objects
{
class SpriteObject : public Object
{
private:
  gsl::not_null<std::shared_ptr<render::scene::Node>> m_objectNode;
  gsl::not_null<std::shared_ptr<render::scene::Node>> m_displayNode;
  const world::Sprite* m_sprite = nullptr;
  core::Brightness m_brightness{0.5f};

  void createModel();

protected:
  SpriteObject(const std::string& name, const gsl::not_null<world::World*>& world, const Location& location);

public:
  SpriteObject(const std::string& name,
               const gsl::not_null<world::World*>& world,
               const gsl::not_null<const world::Room*>& room,
               const loader::file::Item& item,
               bool hasUpdateFunction,
               const gsl::not_null<const world::Sprite*>& sprite);

  SpriteObject(const SpriteObject&) = delete;
  SpriteObject(SpriteObject&&) = delete;
  SpriteObject& operator=(const SpriteObject&) = delete;
  SpriteObject& operator=(SpriteObject&&) = delete;

  ~SpriteObject() override
  {
    setParent(m_objectNode, nullptr);
  }

  bool triggerSwitch(const core::Frame&) override
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("triggerSwitch called on sprite"));
  }

  std::shared_ptr<render::scene::Node> getNode() const override
  {
    return m_objectNode;
  }

  void update() override
  {
  }

  const world::Sprite& getSprite() const
  {
    Expects(m_sprite != nullptr);
    return *m_sprite;
  }

  core::BoundingBox getBoundingBox() const override
  {
    return core::BoundingBox{};
  }

  void updateLighting() override
  {
  }

  void serialize(const serialization::Serializer<world::World>& ser) override;
};

#define SPRITEOBJECT_DEFAULT_CONSTRUCTORS(CLASS, HAS_UPDATE_FUNCTION)                                 \
  CLASS(const std::string& name, const gsl::not_null<world::World*>& world, const Location& location) \
      : SpriteObject{name, world, location}                                                           \
  {                                                                                                   \
  }                                                                                                   \
                                                                                                      \
  CLASS(const std::string& name,                                                                      \
        const gsl::not_null<world::World*>& world,                                                    \
        const gsl::not_null<const world::Room*>& room,                                                \
        const loader::file::Item& item,                                                               \
        const gsl::not_null<const world::Sprite*>& sprite)                                            \
      : SpriteObject{name, world, room, item, HAS_UPDATE_FUNCTION, sprite}                            \
  {                                                                                                   \
  }

} // namespace engine::objects
