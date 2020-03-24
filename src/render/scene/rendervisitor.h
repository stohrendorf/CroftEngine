#pragma once

#include "node.h"
#include "renderable.h"
#include "visitor.h"

#include <gl/debuggroup.h>

namespace render::scene
{
class RenderVisitor : public Visitor
{
public:
  static constexpr bool FlushAfterEachRender = false;
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
      gl::DebugGroup debugGroup{node.getName() + " <culled>"};
      return;
    }

    gl::DebugGroup debugGroup{node.getName()};

    getContext().setCurrentNode(&node);

    if(auto r = node.getRenderable())
    {
      [[maybe_unused]] bool rendered = r->render(getContext());
      if constexpr(FlushAfterEachRender)
      {
        if(rendered)
        {
          GL_ASSERT(gl::api::finish());
        }
      }
    }

    Visitor::visit(node);
  }
};
} // namespace render::scene
