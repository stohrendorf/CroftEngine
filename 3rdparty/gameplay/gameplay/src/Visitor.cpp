#include "Visitor.h"

#include "Node.h"
#include "RenderContext.h"

namespace gameplay
{
    void Visitor::visit(Node& node)
    {
        m_context.setCurrentNode( &node );
        node.accept( *this );
        m_context.setCurrentNode( nullptr );
    }
}
