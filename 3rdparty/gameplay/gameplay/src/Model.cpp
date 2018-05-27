#include "Base.h"
#include "Model.h"
#include "MeshPart.h"
#include "Node.h"

namespace gameplay
{
void Model::draw(RenderContext& context)
{
    for( const auto& mesh : m_meshes )
    {
        BOOST_ASSERT( mesh );

        for( const auto& part : mesh->getParts() )
        {
            BOOST_ASSERT( part );

            part->draw( context );
        }
    }
}
}
