#include "spriteobject.h"

#include "engine/world/room.h"
#include "engine/world/sprite.h"
#include "engine/world/world.h"
#include "loader/file/item.h"
#include "object.h"
#include "objectstate.h"
#include "qs/quantity.h"
#include "render/scene/mesh.h"
#include "serialization/quantity.h"
#include "serialization/serialization.h"
#include "serialization/vector_element.h"

#include <algorithm>
#include <exception>
#include <gl/buffer.h>
#include <gl/program.h>
#include <glm/fwd.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

namespace engine
{
struct ShaderLight;
}

namespace engine::objects
{
SpriteObject::SpriteObject(const std::string& name,
                           const gsl::not_null<world::World*>& world,
                           const gsl::not_null<const world::Room*>& room,
                           const loader::file::Item& item,
                           const bool hasUpdateFunction,
                           const gsl::not_null<const world::Sprite*>& sprite,
                           bool billboard)
    : Object{world, room, item, hasUpdateFunction}
    , m_objectNode{std::make_shared<render::scene::Node>(name)}
    , m_displayNode{std::make_shared<render::scene::Node>(name + "-display")}
    , m_sprite{sprite}
    , m_brightness{toBrightness(item.shade)}
    , m_billboard{billboard}
{
  createModel();
  addChild(gsl::not_null{room->node}, m_objectNode);
  addChild(m_objectNode, m_displayNode);
  applyTransform();
}

SpriteObject::SpriteObject(const std::string& name,
                           const gsl::not_null<world::World*>& world,
                           const Location& location,
                           bool billboard)
    : Object{world, location}
    , m_objectNode{std::make_shared<render::scene::Node>(name)}
    , m_displayNode{std::make_shared<render::scene::Node>(name + "-display")}
    , m_billboard{billboard}
{
  addChild(m_objectNode, m_displayNode);
}

void SpriteObject::createModel()
{
  Expects(m_sprite != nullptr);

  m_displayNode->setRenderable(m_billboard ? m_sprite->billboardMesh : m_sprite->yBoundMesh);
  m_displayNode->bind("u_lightAmbient",
                      [brightness = m_brightness](
                        const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                      {
                        uniform.set(brightness.get());
                      });
  m_displayNode->bind(
    "b_lights",
    [emptyBuffer = std::make_shared<gl::ShaderStorageBuffer<engine::ShaderLight>>("lights-buffer-empty")](
      const render::scene::Node*, const render::scene::Mesh& /*mesh*/, gl::ShaderStorageBlock& shaderStorageBlock)
    {
      shaderStorageBlock.bind(*emptyBuffer);
    });

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

void SpriteObject::replace(const TR1ItemId& itemId, const gsl::not_null<const world::Sprite*>& sprite)
{
  m_sprite = sprite;
  m_state.type = itemId;
  createModel();
}
} // namespace engine::objects
