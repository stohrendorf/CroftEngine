#pragma once

#include "core/vec.h"
#include "render/scene/node.h"
#include "util/helpers.h"
#include "world/room.h"

#include <gl/buffer.h>
#include <set>

namespace engine
{
struct Lighting
{
  struct Light
  {
    glm::vec4 position{std::numeric_limits<float>::quiet_NaN()};
    float brightness = 0;
    float fadeDistance = 0;
    float _pad[2]{0.0f, 0.0f};

    bool operator==(const Light& rhs) const
    {
      return position == rhs.position && brightness == rhs.brightness && fadeDistance == rhs.fadeDistance;
    }

    bool operator!=(const Light& rhs) const
    {
      return !(*this == rhs);
    }
  };
  static_assert(sizeof(Light) == 32, "Invalid Light struct size");

  core::Brightness ambient{-1.0f};
  core::Brightness targetAmbient{};
  std::vector<Light> lights;
  std::vector<Light> bufferLights;

  gl::ShaderStorageBuffer<Light> m_buffer{"lights-buffer"};

  void updateDynamic(const core::Shade& shade, const core::RoomBoundPosition& pos)
  {
    if(shade.get() >= 0)
    {
      updateStatic(shade);
      return;
    }

    setAmbient(pos.room->ambientShade);

    lights.clear();
    if(pos.room->lights.empty())
    {
      m_buffer.setData(lights, gl::api::BufferUsageARB::StreamDraw);
      return;
    }

    std::set<gsl::not_null<const world::Room*>> testRooms;
    testRooms.emplace(pos.room);
    for(const auto& portal : pos.room->portals)
    {
      testRooms.emplace(portal.adjoiningRoom);
    }

    std::set<gsl::not_null<const world::Room*>> testRooms2;
    for(const auto& room : testRooms)
    {
      testRooms2.emplace(room);
      for(const auto& portal : room->portals)
      {
        testRooms2.emplace(portal.adjoiningRoom);
      }
    }

    for(const auto& room : testRooms2)
    {
      // http://www-f9.ijs.si/~matevz/docs/PovRay/pov274.htm
      // 1 / ( 1 + (d/fade_distance) ^ fade_power );
      // assuming fade_power = 1, multiply numerator and denominator with fade_distance (identity transform):
      // fade_distance / ( fade_distance + d )

      for(const auto& light : room->lights)
      {
        if(light.intensity.get() <= 0)
          continue;
        lights.emplace_back(Light{glm::vec4{light.position.toRenderSystem(), 0.0f},
                                  toBrightness(light.intensity).get(),
                                  light.fadeDistance.get<float>()});
      }
    }

    if(bufferLights != lights)
      m_buffer.setData(lights, gl::api::BufferUsageARB::DynamicDraw);
    bufferLights = lights;
  }

  void updateStatic(const core::Shade& shade)
  {
    lights.clear();
    setAmbient(shade);
    m_buffer.setData(lights, gl::api::BufferUsageARB::StaticDraw);
  }

  void setAmbient(const core::Shade& shade)
  {
    targetAmbient = toBrightness(shade);
    if(ambient.get() < 0)
      ambient = targetAmbient;
    else
      ambient += (targetAmbient - ambient) / 50.0f;
  }

  void bind(render::scene::Node& node) const
  {
    node.addUniformSetter("u_lightAmbient", [this](const render::scene::Node& /*node*/, gl::Uniform& uniform) {
      uniform.set(ambient.get());
    });

    node.addBufferBinder("b_lights", [this](const render::scene::Node&, gl::ShaderStorageBlock& shaderStorageBlock) {
      shaderStorageBlock.bind(m_buffer);
    });
  }
};
} // namespace engine
