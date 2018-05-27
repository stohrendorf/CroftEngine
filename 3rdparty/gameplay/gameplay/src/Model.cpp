#include "Base.h"
#include "Model.h"
#include "MeshPart.h"
#include "Node.h"

namespace gameplay
{
void Model::draw(RenderContext& context)
{
    for( const std::shared_ptr<Mesh>& mesh : m_meshes )
    {
        for( const std::shared_ptr<MeshPart>& part : mesh->getParts() )
        {
            part->draw( context );
        }
    }
}
}
