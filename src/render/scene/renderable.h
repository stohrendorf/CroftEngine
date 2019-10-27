#pragma once

#include "render/gl/renderstate.h"

namespace render
{
namespace scene
{
class RenderContext;

class Renderable
{
public:
  explicit Renderable() = default;

  Renderable(const Renderable&) = delete;

  Renderable(Renderable&&) = delete;

  Renderable& operator=(Renderable&&) = delete;

  Renderable& operator=(const Renderable&) = delete;

  virtual ~Renderable() = default;

  virtual void render(RenderContext& context) = 0;

  render::gl::RenderState& getRenderState()
  {
    return m_renderState;
  }

private:
  render::gl::RenderState m_renderState;
};
} // namespace scene
} // namespace render
