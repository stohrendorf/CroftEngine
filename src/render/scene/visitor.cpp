#include "visitor.h"

#include "camera.h"
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
    SOGLB_DEBUGGROUP(node.getName() + " <culled>");
    return;
  }

  node.accept(*this);
}
} // namespace render::scene
