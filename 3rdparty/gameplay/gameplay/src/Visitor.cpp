#include "Visitor.h"

#include "Node.h"
#include "RenderContext.h"

#include "gl/debuggroup.h"

namespace gameplay
{
void Visitor::visit(Node& node)
{
    gl::DebugGroup debugGroup{node.getId()};

    m_context.setCurrentNode( &node );
    node.accept( *this );
    m_context.setCurrentNode( nullptr );
}
}
