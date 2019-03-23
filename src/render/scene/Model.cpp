#include "names.h"
#include "Model.h"
#include "MeshPart.h"

namespace render
{
namespace scene
{
void Model::draw(RenderContext& context)
{
    context.pushState( getRenderState() );
    for( const auto& mesh : m_meshes )
    {
        mesh->draw( context );
    }
    context.popState();
}
}
}
