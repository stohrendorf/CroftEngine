#pragma once

#include "core/vec.h"
#include "loader/file/datatypes.h"
#include "util/helpers.h"

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
    float _pad[2];

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

  void updateDynamic(const core::Shade& shade,
                     const core::RoomBoundPosition& pos,
                     const std::vector<loader::file::Room>& rooms)
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

    std::set<gsl::not_null<const loader::file::Room*>> testRooms;
    testRooms.emplace(pos.room);
    std::transform(pos.room->portals.begin(),
                   pos.room->portals.end(),
                   std::inserter(testRooms, testRooms.end()),
                   [&rooms](const auto& portal) { return &rooms.at(portal.adjoining_room.get()); });

    std::set<gsl::not_null<const loader::file::Room*>> testRooms2;
    for(const auto& room : testRooms)
    {
      testRooms2.emplace(room);
      std::transform(room->portals.begin(),
                     room->portals.end(),
                     std::inserter(testRooms2, testRooms2.end()),
                     [&rooms](const auto& portal) { return &rooms.at(portal.adjoining_room.get()); });
    }

    for(const auto& room : testRooms2)
    {
      // http://www-f9.ijs.si/~matevz/docs/PovRay/pov274.htm
      // 1 / ( 1 + (d/fade_distance) ^ fade_power );
      // assuming fade_power = 1, multiply numerator and denominator with fade_distance (identity transform):
      // fade_distance / ( fade_distance + d )
      std::transform(
        room->lights.begin(), room->lights.end(), std::back_inserter(lights), [](const loader::file::Light& light) {
          return Light{
            glm::vec4{light.position.toRenderSystem(), 0.0f}, light.getBrightness(), light.fadeDistance.get<float>()};
        });
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
