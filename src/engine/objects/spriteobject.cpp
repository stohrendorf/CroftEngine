#include "spriteobject.h"

#include "engine/lighting.h"
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
SpriteObject::SpriteObject(const std::string& name,
                           const gsl::not_null<world::World*>& world,
                           const gsl::not_null<const world::Room*>& room,
                           const loader::file::Item& item,
                           const bool hasUpdateFunction,
                           const gsl::not_null<const world::Sprite*>& sprite)
    : Object{world, room, item, hasUpdateFunction}
    , m_objectNode{std::make_shared<render::scene::Node>(name)}
    , m_displayNode{std::make_shared<render::scene::Node>(name + "-display")}
    , m_sprite{sprite}
    , m_brightness{toBrightness(item.shade)}
{
  createModel();
  addChild(room->node, m_objectNode);
  addChild(m_objectNode, m_displayNode);
  applyTransform();
}

SpriteObject::SpriteObject(const std::string& name, const gsl::not_null<world::World*>& world, const Location& location)
    : Object{world, location}
    , m_objectNode{std::make_shared<render::scene::Node>(name)}
    , m_displayNode{std::make_shared<render::scene::Node>(name + "-display")}
{
  addChild(m_objectNode, m_displayNode);
}

void SpriteObject::createModel()
{
  Expects(m_sprite != nullptr);

  m_displayNode->setRenderable(m_sprite->yBoundMesh);
  m_displayNode->bind("u_lightAmbient",
                      [brightness = m_brightness](
                        const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                      { uniform.set(brightness.get()); });
  m_displayNode->bind(
    "b_lights",
    [emptyBuffer = std::make_shared<gl::ShaderStorageBuffer<engine::ShaderLight>>("lights-buffer-empty")](
      const render::scene::Node&, const render::scene::Mesh& /*mesh*/, gl::ShaderStorageBlock& shaderStorageBlock)
    { shaderStorageBlock.bind(*emptyBuffer); });

  if(m_sprite->render1.y > 0)
  {
    m_displayNode->setLocalMatrix(glm::translate(glm::mat4{1.0f}, glm::vec3{0.0f, m_sprite->render1.y, 0.0f}));
  }
}

void SpriteObject::serialize(const serialization::Serializer<world::World>& ser)
{
  Object::serialize(ser);
  auto tmp = getNode()->getName();
  ser(S_NV("@name", tmp),
      S_NV_VECTOR_ELEMENT("sprite", ser.context.getSprites(), m_sprite),
      S_NV("brightness", m_brightness));
  if(ser.loading)
  {
    createModel();
    m_objectNode->setVisible(m_state.triggerState != TriggerState::Invisible);
  }
}
} // namespace engine::objects
