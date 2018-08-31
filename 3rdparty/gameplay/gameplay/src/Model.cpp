#include "Base.h"
#include "Model.h"
#include "MeshPart.h"
#include "Node.h"

namespace gameplay
{
void Model::draw(RenderContext& context)
{
    context.pushState( m_renderState );
    for( const std::shared_ptr<Mesh>& mesh : m_meshes )
    {
        mesh->draw(context);
    }
    context.popState();
}
}
