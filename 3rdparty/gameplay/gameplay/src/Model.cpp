#include "Base.h"
#include "Model.h"
#include "MeshPart.h"

namespace gameplay
{
void Model::draw(RenderContext& context)
{
    context.pushState( m_renderState );
    for( const auto& mesh : m_meshes )
    {
        mesh->draw( context );
    }
    context.popState();
}
}
