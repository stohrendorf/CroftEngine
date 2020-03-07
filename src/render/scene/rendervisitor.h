#pragma once

#include "node.h"
#include "visitor.h"

#include <gl/debuggroup.h>

// #define RENDER_SYNCHRONOUS

namespace render::scene
{
class RenderVisitor : public Visitor
{
public:
  explicit RenderVisitor(RenderContext& context)
      : Visitor{context}
  {
  }

  void visit(Node& node) override
  {
    if(!node.isVisible())
      return;
    if(getContext().getViewProjection().has_value() && node.canBeCulled(getContext().getViewProjection().value()))
    {
      gl::DebugGroup debugGroup{node.getId() + " <culled>"};
      return;
    }

    gl::DebugGroup debugGroup{node.getId()};

    getContext().setCurrentNode(&node);

    if(auto dr = node.getRenderable())
    {
      dr->render(getContext());
#ifdef RENDER_SYNCHRONOUS
      GL_ASSERT(gl::api::finish());
#endif
    }

    Visitor::visit(node);
  }
};
} // namespace render::scene
