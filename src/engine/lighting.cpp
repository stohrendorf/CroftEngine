#include "lighting.h"

#include "core/units.h"
#include "engine/objectmanager.h"
#include "engine/objects/laraobject.h"
#include "engine/world/room.h"
#include "engine/world/world.h"
#include "render/scene/node.h"

#include <gl/program.h>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <vector>

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

  m_buffer = gsl::not_null{baseRoom.lightsBuffer};
  fadeAmbient(baseRoom.ambientShade);
}

void Lighting::bind(render::scene::Node& node, const world::World& world) const
{
  node.bind("u_lightAmbient",
            [this](const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
            {
              uniform.set(ambient.get());
            });

  node.bind("b_dynLights",
            [&world, emptyLightsBuffer = ShaderLight::getEmptyBuffer()](
              const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::ShaderStorageBlock& block)
            {
              if(const auto lara = world.getObjectManager().getLaraPtr();
                 lara != nullptr && !lara->flashLightsBufferData.empty())
              {
                block.bindRange(*lara->flashLightsBuffer, 0, lara->flashLightsBufferData.size());
              }
              else
              {
                block.bind(*emptyLightsBuffer);
              }
            });

  node.bind(
    "b_lights",
    [this](const render::scene::Node*, const render::scene::Mesh& /*mesh*/, gl::ShaderStorageBlock& shaderStorageBlock)
    {
      shaderStorageBlock.bind(*m_buffer);
    });
}
} // namespace engine
