#pragma once

#include <gl/renderstate.h>

namespace render::scene
{
class RenderContext;

class Renderable
{
public:
  explicit Renderable() = default;
  virtual ~Renderable() = default;

  Renderable(const Renderable&) = delete;
  Renderable(Renderable&&) = delete;
  Renderable& operator=(Renderable&&) = delete;
  Renderable& operator=(const Renderable&) = delete;

  virtual void render(RenderContext& context) = 0;

  gl::RenderState& getRenderState()
  {
    return m_renderState;
  }

private:
  gl::RenderState m_renderState;
};
} // namespace render::scene
