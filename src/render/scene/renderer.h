#pragma once

#include "render/gl/pixel.h"
#include "render/gl/renderstate.h"

#include <chrono>

namespace render::scene
{
class Scene;

class RenderContext;

class Renderer final
{
public:
  Renderer(const Renderer&) = delete;

  Renderer(Renderer&&) = delete;

  Renderer& operator=(const Renderer&) = delete;

  Renderer& operator=(Renderer&&) = delete;

  explicit Renderer();

  ~Renderer();

  [[nodiscard]] std::chrono::high_resolution_clock::time_point getGameTime() const
  {
    return std::chrono::high_resolution_clock::now() - m_constructionTime.time_since_epoch();
  }

  void render();

  [[nodiscard]] float getFrameRate() const
  {
    return m_frameRate;
  }

  void clear(::gl::core::Bitfield<::gl::ClearBufferMask> flags, const gl::SRGBA8& clearColor, float clearDepth);

  void clear(::gl::core::Bitfield<::gl::ClearBufferMask> flags,
             uint8_t red,
             uint8_t green,
             uint8_t blue,
             uint8_t alpha,
             float clearDepth)
  {
    clear(flags, gl::SRGBA8{red, green, blue, alpha}, clearDepth);
  }

  [[nodiscard]] const std::shared_ptr<Scene>& getScene() const
  {
    return m_scene;
  }

private:
  const std::chrono::high_resolution_clock::time_point m_constructionTime{std::chrono::high_resolution_clock::now()};

  std::chrono::high_resolution_clock::time_point m_frameLastFPS{}; // The last time the frame count was updated.
  uint_fast32_t m_frameCount = 0;                                  // The current frame count.
  float m_frameRate = 0;                                           // The current frame rate.
  gl::SRGBA8 m_clearColor; // The clear color value last used for clearing the color buffer.
  float m_clearDepth = 1;  // The clear depth value last used for clearing the depth buffer.

  std::shared_ptr<Scene> m_scene;
};
} // namespace render::scene
