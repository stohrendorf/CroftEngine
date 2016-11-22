#include "Base.h"
#include "Model.h"
#include "MeshPart.h"
#include "Node.h"
#include "MaterialParameter.h"

#include <boost/log/trivial.hpp>

namespace gameplay
{
    Model::Model(const std::shared_ptr<Mesh>& mesh)
        : Drawable()
        , _mesh(mesh)
    {
        BOOST_ASSERT(mesh);
    }


    Model::~Model() = default;


    const std::shared_ptr<Mesh>& Model::getMesh() const
    {
        return _mesh;
    }


    void Model::setMaterial(const std::shared_ptr<Material>& material, size_t partIndex)
    {
        BOOST_ASSERT(partIndex < _mesh->getPartCount());
        BOOST_ASSERT(material != nullptr);

        _mesh->getPart(partIndex)->setMaterial(material);

        // Apply node binding for the new material.
        if( _node )
        {
            bindNodeTo(material);
        }
    }


    std::shared_ptr<Material> Model::setMaterial(const std::string& vshPath, const std::string& fshPath, const std::vector<std::string>& defines, size_t partIndex)
    {
        // Try to create a Material with the given parameters.
        auto material = std::make_shared<Material>(vshPath, fshPath, defines);
        if( material == nullptr )
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to create material for model.";
            return nullptr;
        }

        // Assign the material to us.
        setMaterial(material, partIndex);

        return material;
    }


    void Model::setNode(Node* node)
    {
        Drawable::setNode(node);

        // Re-bind node related material parameters
        if( !node )
            return;

        for(size_t i = 0; i < _mesh->getPartCount(); ++i)
            bindNodeTo(_mesh->getPart(i)->_material);
    }


    static bool drawWireframe(const std::shared_ptr<Mesh>& mesh)
    {
        switch( mesh->getPrimitiveType() )
        {
            case Mesh::TRIANGLES:
            {
                size_t vertexCount = mesh->getVertexCount();
                for(size_t i = 0; i < vertexCount; i += 3 )
                {
                    GL_ASSERT( glDrawArrays(GL_LINE_LOOP, i, 3) );
                }
            }
                return true;

            case Mesh::TRIANGLE_STRIP:
            {
                size_t vertexCount = mesh->getVertexCount();
                for(size_t i = 2; i < vertexCount; ++i )
                {
                    GL_ASSERT( glDrawArrays(GL_LINE_LOOP, i-2, 3) );
                }
            }
                return true;

            default:
                // not supported
                return false;
        }
    }


    size_t Model::draw(bool wireframe)
    {
        BOOST_ASSERT(_mesh);

        for(const auto& part : _mesh->_parts)
        {
            BOOST_ASSERT(part);

            part->draw(wireframe, getNode());
        }

        return _mesh->getPartCount();
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    void Model::bindNodeTo(const std::shared_ptr<Material>& material)
    {
        BOOST_ASSERT(material);

        if( _node )
        {
            material->bindToNode(getNode());
        }
    }
}
