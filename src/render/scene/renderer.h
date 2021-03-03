#pragma once

#include <chrono>
#include <gl/pixel.h>
#include <gl/renderstate.h>

namespace render::scene
{
class Camera;
class Node;
class RenderContext;

class Renderer final
{
public:
  Renderer(const Renderer&) = delete;
  Renderer(Renderer&&) = delete;
  Renderer& operator=(const Renderer&) = delete;
  Renderer& operator=(Renderer&&) = delete;

  explicit Renderer(gsl::not_null<std::shared_ptr<Camera>> camera);

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

  void clear(const gl::api::core::Bitfield<gl::api::ClearBufferMask>& flags,
             const gl::SRGBA8& clearColor,
             float clearDepth);

  void clear(const gl::api::core::Bitfield<gl::api::ClearBufferMask>& flags,
             uint8_t red,
             uint8_t green,
             uint8_t blue,
             uint8_t alpha,
             float clearDepth)
  {
    clear(flags, gl::SRGBA8{red, green, blue, alpha}, clearDepth);
  }

  [[nodiscard]] const auto& getRootNode() const
  {
    return m_rootNode;
  }

  [[nodiscard]] const auto& getCamera() const
  {
    return m_camera;
  }

  // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
  void resetRenderState()
  {
    static gl::RenderState state;
    state.apply(true);
  }

  void resetRootNode();

private:
  const std::chrono::high_resolution_clock::time_point m_constructionTime{std::chrono::high_resolution_clock::now()};

  std::chrono::high_resolution_clock::time_point m_frameLastFPS{}; // The last time the frame count was updated.
  uint_fast32_t m_frameCount = 0;                                  // The current frame count.
  float m_frameRate = 0;                                           // The current frame rate.
  gl::SRGBA8 m_clearColor; // The clear color value last used for clearing the color buffer.
  float m_clearDepth = 1;  // The clear depth value last used for clearing the depth buffer.

  std::shared_ptr<Node> m_rootNode;
  gsl::not_null<std::shared_ptr<Camera>> m_camera;
};
} // namespace render::scene
