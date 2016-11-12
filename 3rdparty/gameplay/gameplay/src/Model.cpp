#include "Base.h"
#include "Model.h"
#include "MeshPart.h"
#include "Node.h"
#include "MaterialParameter.h"

#include <boost/log/trivial.hpp>

namespace gameplay
{
    Model::Model()
        : Drawable()
        , _mesh(nullptr)
        , _skin(nullptr)
    {
    }


    Model::Model(const std::shared_ptr<Mesh>& mesh)
        : Drawable()
        , _mesh(mesh)
        , _skin(nullptr)
    {
        BOOST_ASSERT(mesh);
    }


    Model::~Model() = default;


    const std::shared_ptr<Mesh>& Model::getMesh() const
    {
        return _mesh;
    }


    std::shared_ptr<Material> Model::getMaterial(size_t partIndex) const
    {
        BOOST_ASSERT(partIndex >= 0);

        if( partIndex >= _mesh->getPartCount() )
            return nullptr;

        // Look up explicitly specified part material.
        return _mesh->getPart(partIndex)->_material;
    }


    void Model::setMaterial(const std::shared_ptr<Material>& material, size_t partIndex)
    {
        BOOST_ASSERT(partIndex < _mesh->getPartCount());
        BOOST_ASSERT(material != nullptr);

        _mesh->getPart(partIndex)->setMaterial(material);

        // Apply node binding for the new material.
        if( _node )
        {
            setMaterialNodeBinding(material);
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


    const std::unique_ptr<MeshSkin>& Model::getSkin() const
    {
        return _skin;
    }


    void Model::setSkin(std::unique_ptr<MeshSkin>&& skin)
    {
        if( _skin != skin )
        {
            _skin = std::move(skin);
            if( _skin )
                _skin->_model = std::static_pointer_cast<Model>( shared_from_this() );
        }
    }


    void Model::setNode(Node* node)
    {
        Drawable::setNode(node);

        // Re-bind node related material parameters
        if( !node )
            return;

        for(size_t i = 0; i < _mesh->getPartCount(); ++i)
            setMaterialNodeBinding(_mesh->getPart(i)->_material);
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


    static bool drawWireframe(const std::shared_ptr<MeshPart>& part)
    {
        size_t indexCount = part->getIndexCount();
        size_t indexSize = 0;
        switch( part->getIndexFormat() )
        {
            case Mesh::INDEX8:
                indexSize = 1;
                break;
            case Mesh::INDEX16:
                indexSize = 2;
                break;
            case Mesh::INDEX32:
                indexSize = 4;
                break;
            default:
                BOOST_LOG_TRIVIAL(error) << "Unsupported index format (" << part->getIndexFormat() << ").";
                return false;
        }

        switch( part->getPrimitiveType() )
        {
            case Mesh::TRIANGLES:
                for( size_t i = 0; i < indexCount; i += 3 )
                {
                    GL_ASSERT( glDrawElements(GL_LINE_LOOP, 3, part->getIndexFormat(), (reinterpret_cast<const GLvoid*>(i*indexSize))) );
                }
                return true;

            case Mesh::TRIANGLE_STRIP:
                for( size_t i = 2; i < indexCount; ++i )
                {
                    GL_ASSERT( glDrawElements(GL_LINE_LOOP, 3, part->getIndexFormat(), (reinterpret_cast<const GLvoid*>((i-2)*indexSize))) );
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

        const size_t partCount = _mesh->getPartCount();
        BOOST_ASSERT(partCount > 0);
        for(size_t i = 0; i < partCount; ++i )
        {
            auto part = _mesh->getPart(i);
            BOOST_ASSERT(part);

            // Get the material for this mesh part.
            auto material = getMaterial(i);
            if( !material )
                continue;

            material->setNodeBinding(getNode());

            material->bind(part->getVaBinding());

            //! @todo apply additional bindings here

            part->bind();
            if( !wireframe || !drawWireframe(part) )
            {
                GL_ASSERT( glDrawElements(part->getPrimitiveType(), part->getIndexCount(), part->getIndexFormat(), nullptr) );
            }
            material->unbind();
        }
        return partCount;
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    void Model::setMaterialNodeBinding(const std::shared_ptr<Material>& material)
    {
        BOOST_ASSERT(material);

        if( _node )
        {
            material->setNodeBinding(getNode());
        }
    }
}
