#include "visitor.h"

#include "node.h"
#include "rendercontext.h"

#include <gl/debuggroup.h>
#include <optional>
#include <string>

namespace render::scene
{
void Visitor::visit(const Node& node)
{
  if(!node.isVisible())
    return;
  if(const auto& vp = m_context.getViewProjection(); vp.has_value() && node.canBeCulled(vp.value()))
  {
    SOGLB_DEBUGGROUP(node.getName() + " <culled>");
    return;
  }

  m_context.pushState(node.getRenderState());
  node.accept(*this);
  m_context.popState();
}
} // namespace render::scene
