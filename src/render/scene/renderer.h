#pragma once

#include <chrono>
#include <cstdint>
#include <gl/api/gl.hpp>
#include <gl/pixel.h>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <memory>

namespace render::scene
{
class Camera;
class Node;

class Renderer final
{
public:
  Renderer(const Renderer&) = delete;
  Renderer(Renderer&&) = delete;
  Renderer& operator=(const Renderer&) = delete;
  Renderer& operator=(Renderer&&) = delete;

  explicit Renderer(gslu::nn_shared<Camera> camera);

  ~Renderer();

  [[nodiscard]] std::chrono::high_resolution_clock::time_point getGameTime() const
  {
    return std::chrono::high_resolution_clock::now() - m_constructionTime.time_since_epoch();
  }

  void render();

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

  void resetRootNode();

  [[nodiscard]] auto isAlphaClipRendering() const
  {
    return m_isAlphaClipRendering;
  }

  void setAlphaClipRendering(bool value)
  {
    m_isAlphaClipRendering = value;
  }

private:
  const std::chrono::high_resolution_clock::time_point m_constructionTime{std::chrono::high_resolution_clock::now()};

  gl::SRGBA8 m_clearColor;
  float m_clearDepth = 1;

  gslu::nn_shared<Node> m_rootNode;
  gslu::nn_shared<Camera> m_camera;

  bool m_isAlphaClipRendering = false;
};
} // namespace render::scene
