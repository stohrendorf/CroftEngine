#include "spriteobject.h"

#include "engine/world/world.h"
#include "loader/file/item.h"
#include "loader/file/level/level.h"
#include "render/scene/mesh.h"
#include "serialization/quantity.h"
#include "serialization/serialization.h"
#include "serialization/vector_element.h"

#include <utility>

namespace engine::objects
{
SpriteObject::SpriteObject(const gsl::not_null<world::World*>& world,
                           std::string name,
                           const gsl::not_null<const world::Room*>& room,
                           const loader::file::Item& item,
                           const bool hasUpdateFunction,
                           const gsl::not_null<const world::Sprite*>& sprite)
    : Object{world, room, item, hasUpdateFunction}
    , m_node{std::make_shared<render::scene::Node>(std::move(name))}
    , m_sprite{sprite}
    , m_brightness{toBrightness(item.shade)}
{
  createModel();
  addChild(room->node, m_node);
  applyTransform();
}

SpriteObject::SpriteObject(const gsl::not_null<world::World*>& world,
                           const core::RoomBoundPosition& position,
                           std::string name)
    : Object{world, position}
    , m_node{std::make_shared<render::scene::Node>(std::move(name))}
{
}

void SpriteObject::createModel()
{
  Expects(m_sprite != nullptr);

  m_node->setRenderable(m_sprite->mesh);
  m_node->bind("u_lightAmbient",
               [brightness = m_brightness](
                 const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
               { uniform.set(brightness.get()); });
  m_node->bind(
    "b_lights",
    [emptyBuffer = std::make_shared<gl::ShaderStorageBuffer<engine::Lighting::Light>>("lights-buffer-empty")](
      const render::scene::Node&, const render::scene::Mesh& /*mesh*/, gl::ShaderStorageBlock& shaderStorageBlock)
    { shaderStorageBlock.bind(*emptyBuffer); });
}

void SpriteObject::serialize(const serialization::Serializer<world::World>& ser)
{
  Object::serialize(ser);
  auto tmp = getNode()->getName();
  ser(S_NV("@name", tmp), S_NVVE("sprite", ser.context.getSprites(), m_sprite), S_NV("brightness", m_brightness));
  if(ser.loading)
  {
    createModel();
  }
}
} // namespace engine::objects
