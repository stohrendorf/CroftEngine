#include "lighting.h"

#include "engine/world/room.h"
#include "render/scene/node.h"

#include <gl/api/gl.hpp>
#include <gl/program.h>

namespace render::scene
{
class Mesh;
}

namespace engine
{
void Lighting::update(const core::Shade& shade, const world::Room& baseRoom)
{
  if(shade.get() >= 0)
  {
    fadeAmbient(shade);
    m_buffer = ShaderLight::getEmptyBuffer();
    return;
  }

  m_buffer = baseRoom.lightsBuffer;
  fadeAmbient(baseRoom.ambientShade);
}

void Lighting::bind(render::scene::Node& node) const
{
  node.bind("u_lightAmbient",
            [this](const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
            {
              uniform.set(ambient.get());
            });

  node.bind(
    "b_lights",
    [this](const render::scene::Node&, const render::scene::Mesh& /*mesh*/, gl::ShaderStorageBlock& shaderStorageBlock)
    {
      shaderStorageBlock.bind(*m_buffer);
    });
}
} // namespace engine
