#include "Visitor.h"

#include "Node.h"
#include "render/gl/debuggroup.h"
#include "rendercontext.h"

namespace render
{
namespace scene
{
void Visitor::visit(Node& node)
{
    gl::DebugGroup debugGroup{node.getId()};

    m_context.setCurrentNode(&node);
    node.accept(*this);
    m_context.setCurrentNode(nullptr);
}
} // namespace scene
} // namespace render
