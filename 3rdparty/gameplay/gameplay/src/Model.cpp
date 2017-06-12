#include "Base.h"
#include "Model.h"
#include "MeshPart.h"
#include "Node.h"
#include "MaterialParameter.h"

#include <boost/log/trivial.hpp>


namespace gameplay
{
    Model::Model()
        : Drawable{}
        , _meshes{}
    {
    }


    Model::~Model() = default;


    void Model::draw(RenderContext& context)
    {
        for( const auto& mesh : _meshes )
        {
            BOOST_ASSERT(mesh);

            for( const auto& part : mesh->getParts() )
            {
                BOOST_ASSERT(part);

                part->draw(context);
            }
        }
    }
}
