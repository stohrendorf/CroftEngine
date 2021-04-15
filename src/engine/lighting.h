#pragma once

#include "core/units.h"

#include <gl/buffer.h>
#include <glm/glm.hpp>
#include <limits>

namespace render::scene
{
class Node;
}

namespace engine::world
{
struct Room;
}

namespace engine
{
struct ShaderLight
{
  glm::vec4 position{std::numeric_limits<float>::quiet_NaN()};
  float brightness = 0;
  float fadeDistance = 0;
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
  [[maybe_unused]] float _pad[2]{0.0f, 0.0f};

  bool operator==(const ShaderLight& rhs) const
  {
    return position == rhs.position && brightness == rhs.brightness && fadeDistance == rhs.fadeDistance;
  }

  bool operator!=(const ShaderLight& rhs) const
  {
    return !(*this == rhs);
  }
};
static_assert(sizeof(ShaderLight) == 32, "Invalid Light struct size");

struct Lighting
{
  core::Brightness ambient{-1.0f};
  core::Brightness targetAmbient{};

  Lighting()
      : m_buffer{&m_emptyBuffer}
  {
  }

  void update(const core::Shade& shade, const world::Room& baseRoom);

  void bind(render::scene::Node& node) const;

private:
  void fadeAmbient(const core::Shade& shade)
  {
    targetAmbient = toBrightness(shade);
    if(ambient.get() < 0)
      ambient = targetAmbient;
    else
      ambient += (targetAmbient - ambient) / 50.0f;
  }

  gsl::not_null<const gl::ShaderStorageBuffer<ShaderLight>*> m_buffer;
  gl::ShaderStorageBuffer<ShaderLight> m_emptyBuffer{"empty-lights-buffer"};
};
} // namespace engine
