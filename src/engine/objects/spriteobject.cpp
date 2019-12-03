#include "spriteobject.h"

#include "loader/file/item.h"
#include "render/scene/sprite.h"
#include "serialization/sprite_ptr.h"

#include <utility>

namespace engine::objects
{
SpriteObject::SpriteObject(const gsl::not_null<Engine*>& engine,
                           std::string name,
                           const gsl::not_null<const loader::file::Room*>& room,
                           const loader::file::Item& item,
                           const bool hasUpdateFunction,
                           const gsl::not_null<const loader::file::Sprite*>& sprite,
                           gsl::not_null<std::shared_ptr<render::scene::Material>> material)
    : Object{engine, room, item, hasUpdateFunction}
    , m_node{std::make_shared<render::scene::Node>(std::move(name))}
    , m_sprite{sprite}
    , m_brightness{item.getBrightness()}
    , m_material{std::move(material)}
{
  createModel();

  addChild(room->node, m_node);
  applyTransform();
}

SpriteObject::SpriteObject(const gsl::not_null<Engine*>& engine,
                           const core::RoomBoundPosition& position,
                           std::string name,
                           gsl::not_null<std::shared_ptr<render::scene::Material>> material)
    : Object{engine, position}
    , m_node{std::make_shared<render::scene::Node>(std::move(name))}
    , m_material{std::move(material)}
{
}

void SpriteObject::createModel()
{
  Expects(m_sprite != nullptr);

  const auto model = std::make_shared<render::scene::Sprite>(m_sprite->x0,
                                                             -m_sprite->y0,
                                                             m_sprite->x1,
                                                             -m_sprite->y1,
                                                             m_sprite->t0,
                                                             m_sprite->t1,
                                                             m_material,
                                                             render::scene::Sprite::Axis::Y);

  m_node->setRenderable(model);
  m_node->addUniformSetter(
    "u_diffuseTexture",
    [texture = m_sprite->texture](const render::scene::Node& /*node*/, render::gl::ProgramUniform& uniform) {
      uniform.set(*texture);
    });
  m_node->addUniformSetter(
    "u_lightAmbient",
    [brightness = m_brightness](const render::scene::Node& /*node*/, render::gl::ProgramUniform& uniform) {
      uniform.set(brightness);
    });
}

void SpriteObject::serialize(const serialization::Serializer& ser)
{
  Object::serialize(ser);
  auto tmp = getNode()->getId();
  ser("@name", tmp, S_NVP(m_sprite), S_NVP(m_brightness));
  if(ser.loading)
  {
    createModel();
  }
}
} // namespace engine::objects