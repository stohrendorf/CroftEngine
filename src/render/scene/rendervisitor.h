#pragma once

#include "node.h"
#include "visitor.h"

#include <gl/debuggroup.h>

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
    {
      return;
    }

    gl::DebugGroup debugGroup{node.getId()};

    getContext().setCurrentNode(&node);

    if(auto dr = node.getRenderable())
    {
      dr->render(getContext());
    }

    Visitor::visit(node);
  }
};
} // namespace render::scene
