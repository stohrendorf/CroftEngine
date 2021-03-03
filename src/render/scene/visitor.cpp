#include "visitor.h"

#include "renderable.h"
#include "rendercontext.h"

#include <gl/debuggroup.h>

namespace render::scene
{
void Visitor::visit(Node& node)
{
  if(!node.isVisible())
    return;
  if(const auto& vp = m_context.getViewProjection(); vp.has_value() && node.canBeCulled(vp.value()))
  {
    gl::DebugGroup debugGroup{node.getName() + " <culled>"};
    return;
  }

  gl::DebugGroup debugGroup{node.getName()};
  m_context.setCurrentNode(&node);
  if(auto r = node.getRenderable())
  {
    [[maybe_unused]] bool rendered = r->render(m_context);
    if constexpr(FlushAfterEachRender)
    {
      if(rendered)
      {
        GL_ASSERT(gl::api::finish());
      }
    }
  }
  node.accept(*this);
  m_context.setCurrentNode(nullptr);
}
} // namespace render::scene
