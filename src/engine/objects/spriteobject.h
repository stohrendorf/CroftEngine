#pragma once

#include "object.h"

namespace engine
{
class World;

namespace objects
{
class SpriteObject : public Object
{
private:
  std::shared_ptr<render::scene::Node> m_node;
  const loader::file::Sprite* m_sprite = nullptr;
  core::Brightness m_brightness{0.5f};
  gsl::not_null<std::shared_ptr<render::scene::Material>> m_material;

  void createModel();

protected:
  SpriteObject(const gsl::not_null<World*>& world,
               const core::RoomBoundPosition& position,
               std::string name,
               gsl::not_null<std::shared_ptr<render::scene::Material>> material);

public:
  SpriteObject(const gsl::not_null<World*>& world,
               std::string name,
               const gsl::not_null<const loader::file::Room*>& room,
               const loader::file::Item& item,
               bool hasUpdateFunction,
               const gsl::not_null<const loader::file::Sprite*>& sprite,
               gsl::not_null<std::shared_ptr<render::scene::Material>> material);

  SpriteObject(const SpriteObject&) = delete;
  SpriteObject(SpriteObject&&) = delete;
  SpriteObject& operator=(const SpriteObject&) = delete;
  SpriteObject& operator=(SpriteObject&&) = delete;

  ~SpriteObject() override
  {
    if(m_node != nullptr)
    {
      setParent(m_node, nullptr);
    }
  }

  bool triggerSwitch(core::Frame) override
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("triggerSwitch called on sprite"));
  }

  std::shared_ptr<render::scene::Node> getNode() const override
  {
    return m_node;
  }

  void update() override
  {
  }

  loader::file::BoundingBox getBoundingBox() const override
  {
    loader::file::BoundingBox bb;
    bb.minX = bb.maxX = m_state.position.position.X;
    bb.minY = bb.maxY = m_state.position.position.Y;
    bb.minZ = bb.maxZ = m_state.position.position.Z;
    return bb;
  }

  void serialize(const serialization::Serializer& ser) override;
};
} // namespace objects
} // namespace engine