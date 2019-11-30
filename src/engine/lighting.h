#pragma once

#include "core/vec.h"
#include "loader/file/datatypes.h"
#include "util/helpers.h"

namespace engine
{
struct Lighting
{
  struct alignas(16) Light
  {
    glm::vec3 position = glm::vec3{std::numeric_limits<float>::quiet_NaN()};
    float brightness = 0;
    float fadeDistance = 0;
  };
  static_assert(sizeof(Light) == 32, "Invalid Light struct size");

  float ambient = 0;
  std::vector<Light> lights;

  render::gl::ShaderStorageBuffer<Light> m_buffer;

  void updateDynamic(int16_t shade, const core::RoomBoundPosition& pos, const std::vector<loader::file::Room>& rooms)
  {
    if(shade >= 0)
    {
      updateStatic(shade);
      return;
    }

    ambient = pos.room->getAmbientBrightness();

    lights.clear();
    if(pos.room->lights.empty())
    {
      return;
    }

    std::vector<gsl::not_null<const loader::file::Room*>> testRooms;
    testRooms.emplace_back(pos.room);
    for(const auto& portal : pos.room->portals)
    {
      testRooms.emplace_back(&rooms.at(portal.adjoining_room.get()));
    }

    for(const auto& room : testRooms)
    {
      for(const auto& light : room->lights)
      {
        // http://www-f9.ijs.si/~matevz/docs/PovRay/pov274.htm
        // 1 / ( 1 + (d/fade_distance) ^ fade_power );
        // assuming fade_power = 1, multiply numerator and denominator with fade_distance (identity transform):
        // fade_distance / ( fade_distance + d )
        lights.emplace_back(
          Light{light.position.toRenderSystem(), light.getBrightness(), light.fadeDistance.get_as<float>()});
      }
    }

    m_buffer.setData(lights, gl::BufferUsageARB::DynamicDraw);
  }

  void updateStatic(int16_t shade)
  {
    lights.clear();
    ambient = 1.0f - shade / 8191.0f;
    m_buffer.setData(lights, gl::BufferUsageARB::DynamicDraw);
  }

  void bind(render::scene::Node& node) const
  {
    node.addUniformSetter(
      "u_lightAmbient",
      [this](const render::scene::Node& /*node*/, render::gl::ProgramUniform& uniform) { uniform.set(ambient); });

    node.addBufferBinder(
      "b_lights", [this](const render::scene::Node& node, render::gl::ProgramShaderStorageBlock& shaderStorageBlock) {
        shaderStorageBlock.bind(m_buffer);
      });
  }
};
} // namespace engine
