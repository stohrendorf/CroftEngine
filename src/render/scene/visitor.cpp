#include "visitor.h"

#include "node.h"
#include "rendercontext.h"

#include <gl/debuggroup.h>

namespace render::scene
{
void Visitor::visit(Node& node)
{
  gl::DebugGroup debugGroup{node.getId()};

  m_context.setCurrentNode(&node);
  node.accept(*this);
  m_context.setCurrentNode(nullptr);
}
} // namespace render::scene
