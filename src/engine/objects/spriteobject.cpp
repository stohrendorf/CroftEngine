#include "spriteobject.h"

#include "loader/file/item.h"
#include "render/scene/sprite.h"
#include "serialization/quantity.h"
#include "serialization/sprite_ptr.h"

#include <utility>

namespace engine::objects
{
SpriteObject::SpriteObject(const gsl::not_null<World*>& world,
                           std::string name,
                           const gsl::not_null<const loader::file::Room*>& room,
                           const loader::file::Item& item,
                           const bool hasUpdateFunction,
                           const gsl::not_null<const loader::file::Sprite*>& sprite,
                           gsl::not_null<std::shared_ptr<render::scene::Material>> material)
    : Object{world, room, item, hasUpdateFunction}
    , m_node{std::make_shared<render::scene::Node>(std::move(name))}
    , m_sprite{sprite}
    , m_brightness{toBrightness(item.shade)}
    , m_material{std::move(material)}
{
  m_lighting.bind(*m_node);

  createModel();
  addChild(room->node, m_node);
  applyTransform();
  updateLighting();
}

SpriteObject::SpriteObject(const gsl::not_null<World*>& world,
                           const core::RoomBoundPosition& position,
                           std::string name,
                           gsl::not_null<std::shared_ptr<render::scene::Material>> material)
    : Object{world, position}
    , m_node{std::make_shared<render::scene::Node>(std::move(name))}
    , m_material{std::move(material)}
{
  m_lighting.bind(*m_node);
}

void SpriteObject::createModel()
{
  Expects(m_sprite != nullptr);

  const auto mesh = render::scene::createSpriteMesh(static_cast<float>(m_sprite->render0.x),
                                                    static_cast<float>(-m_sprite->render0.y),
                                                    static_cast<float>(m_sprite->render1.x),
                                                    static_cast<float>(-m_sprite->render1.y),
                                                    m_sprite->uv0.toGl(),
                                                    m_sprite->uv1.toGl(),
                                                    m_material,
                                                    m_sprite->texture_id.get_as<int32_t>());

  m_node->setRenderable(mesh);
  m_node->addUniformSetter("u_lightAmbient",
                           [brightness = m_brightness](const render::scene::Node& /*node*/, gl::Uniform& uniform) {
                             uniform.set(brightness.get());
                           });
  bindSpritePole(*m_node, render::scene::SpritePole::Y);
}

void SpriteObject::serialize(const serialization::Serializer& ser)
{
  Object::serialize(ser);
  auto tmp = getNode()->getName();
  ser(S_NV("@name", tmp), S_NV("sprite", m_sprite), S_NV("brightness", m_brightness));
  if(ser.loading)
  {
    createModel();
  }
}

gl::CImgWrapper SpriteObject::getCroppedImage() const
{
  Expects(m_sprite != nullptr);
  return m_sprite->image->cropped(m_sprite->uv0.toPx(m_sprite->image->width()).x,
                                  m_sprite->uv0.toPx(m_sprite->image->height()).y,
                                  m_sprite->uv1.toPx(m_sprite->image->width()).x,
                                  m_sprite->uv1.toPx(m_sprite->image->height()).y);
}
} // namespace engine::objects