#pragma once

#include "translucency.h"

#include <gl/api/soglb_core.hpp>
#include <gl/renderstate.h>

namespace render::scene
{
class RenderContext;
class Node;

class Renderable
{
public:
  explicit Renderable() = default;
  virtual ~Renderable() = default;

  Renderable(const Renderable&) = delete;
  Renderable(Renderable&&) = delete;
  Renderable& operator=(Renderable&&) = delete;
  Renderable& operator=(const Renderable&) = delete;

  virtual void render(const Node* node, RenderContext& context) = 0;
  virtual void render(const Node* node, RenderContext& context, gl::api::core::SizeType instanceCount) = 0;
  [[nodiscard]] virtual bool empty(Translucency translucencySelector) const = 0;

  gl::RenderState& getRenderState()
  {
    return m_renderState;
  }

  [[nodiscard]] bool empty() const
  {
    return empty(Translucency::NonOpaque) && empty(Translucency::Opaque);
  }

private:
  gl::RenderState m_renderState;
};
} // namespace render::scene
