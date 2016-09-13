#include "Base.h"
#include "Model.h"
#include "MeshPart.h"
#include "Technique.h"
#include "Pass.h"
#include "Node.h"


namespace gameplay
{
    Model::Model()
        : Drawable()
        , _mesh(nullptr)
        , _material(nullptr)
        , _partCount(0)
        , _partMaterials()
        , _skin(nullptr)
    {
    }


    Model::Model(const std::shared_ptr<Mesh>& mesh)
        : Drawable()
        , _mesh(mesh)
        , _material(nullptr)
        , _partCount(0)
        , _partMaterials()
        , _skin(nullptr)
    {
        GP_ASSERT(mesh);
        _partCount = mesh->getPartCount();
    }


    Model::~Model() = default;


    const std::shared_ptr<Mesh>& Model::getMesh() const
    {
        return _mesh;
    }


    unsigned int Model::getMeshPartCount() const
    {
        GP_ASSERT(_mesh);
        return _mesh->getPartCount();
    }


    std::shared_ptr<Material> Model::getMaterial(int partIndex)
    {
        GP_ASSERT(partIndex == -1 || partIndex >= 0);

        std::shared_ptr<Material> m = nullptr;

        if( partIndex < 0 )
            return _material;
        if( partIndex >= (int)_partCount )
            return nullptr;

        // Look up explicitly specified part material.
        if( !_partMaterials.empty() )
        {
            m = _partMaterials[partIndex];
        }
        if( m == nullptr )
        {
            // Return the shared material.
            m = _material;
        }

        return m;
    }


    void Model::setMaterial(const std::shared_ptr<Material>& material, int partIndex)
    {
        GP_ASSERT(partIndex == -1 || (partIndex >= 0 && partIndex < (int)getMeshPartCount()));

        std::shared_ptr<Material> oldMaterial = nullptr;

        if( partIndex == -1 )
        {
            oldMaterial = _material;

            // Set new shared material.
            if( material )
            {
                _material = material;
            }
        }
        else if( partIndex >= 0 && partIndex < (int)getMeshPartCount() )
        {
            // Ensure mesh part count is up-to-date.
            validatePartCount();

            // Release existing part material and part binding.
            if( !_partMaterials.empty() )
            {
                oldMaterial = _partMaterials[partIndex];
            }
            else
            {
                // Allocate part arrays for the first time.
                if( _partMaterials.empty() )
                {
                    _partMaterials.resize(_partCount);
                }
            }

            // Set new part material.
            if( material )
            {
                _partMaterials[partIndex] = material;
            }
        }

        // Release existing material and binding.
        if( oldMaterial )
        {
            for( unsigned int i = 0, tCount = oldMaterial->getTechniqueCount(); i < tCount; ++i )
            {
                auto t = oldMaterial->getTechniqueByIndex(i);
                GP_ASSERT(t);
                for( unsigned int j = 0, pCount = t->getPassCount(); j < pCount; ++j )
                {
                    GP_ASSERT(t->getPassByIndex(j));
                    t->getPassByIndex(j)->setVertexAttributeBinding(nullptr);
                }
            }
        }

        if( material )
        {
            // Hookup vertex attribute bindings for all passes in the new material.
            for( unsigned int i = 0, tCount = material->getTechniqueCount(); i < tCount; ++i )
            {
                auto t = material->getTechniqueByIndex(i);
                GP_ASSERT(t);
                for( unsigned int j = 0, pCount = t->getPassCount(); j < pCount; ++j )
                {
                    auto p = t->getPassByIndex(j);
                    GP_ASSERT(p);
                    auto b = VertexAttributeBinding::create(_mesh, p->getEffect());
                    p->setVertexAttributeBinding(b);
                }
            }
            // Apply node binding for the new material.
            if( _node )
            {
                setMaterialNodeBinding(material);
            }
        }
    }


    std::shared_ptr<Material> Model::setMaterial(const char* vshPath, const char* fshPath, const char* defines, int partIndex)
    {
        // Try to create a Material with the given parameters.
        auto material = Material::create(vshPath, fshPath, defines);
        if( material == nullptr )
        {
            GP_ERROR("Failed to create material for model.");
            return nullptr;
        }

        // Assign the material to us.
        setMaterial(material, partIndex);

        return material;
    }


    bool Model::hasMaterial(unsigned int partIndex) const
    {
        return (partIndex < _partCount && !_partMaterials.empty() && _partMaterials[partIndex]);
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
        if( node )
        {
            if( _material )
            {
                setMaterialNodeBinding(_material);
            }
            if( !_partMaterials.empty() )
            {
                for( size_t i = 0; i < _partCount; ++i )
                {
                    if( _partMaterials[i] )
                    {
                        setMaterialNodeBinding(_partMaterials[i]);
                    }
                }
            }
        }
    }


    static bool drawWireframe(const std::shared_ptr<Mesh>& mesh)
    {
        switch( mesh->getPrimitiveType() )
        {
            case Mesh::TRIANGLES:
            {
                unsigned int vertexCount = mesh->getVertexCount();
                for( unsigned int i = 0; i < vertexCount; i += 3 )
                {
                    GL_ASSERT( glDrawArrays(GL_LINE_LOOP, i, 3) );
                }
            }
                return true;

            case Mesh::TRIANGLE_STRIP:
            {
                unsigned int vertexCount = mesh->getVertexCount();
                for( unsigned int i = 2; i < vertexCount; ++i )
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


    static bool drawWireframe(MeshPart* part)
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
                GP_ERROR("Unsupported index format (%d).", part->getIndexFormat());
                return false;
        }

        switch( part->getPrimitiveType() )
        {
            case Mesh::TRIANGLES:
            {
                for( size_t i = 0; i < indexCount; i += 3 )
                {
                    GL_ASSERT( glDrawElements(GL_LINE_LOOP, 3, part->getIndexFormat(), (reinterpret_cast<const GLvoid*>(i*indexSize))) );
                }
            }
                return true;

            case Mesh::TRIANGLE_STRIP:
            {
                for( size_t i = 2; i < indexCount; ++i )
                {
                    GL_ASSERT( glDrawElements(GL_LINE_LOOP, 3, part->getIndexFormat(), (reinterpret_cast<const GLvoid*>((i-2)*indexSize))) );
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
        GP_ASSERT(_mesh);

        size_t partCount = _mesh->getPartCount();
        if( partCount == 0 )
        {
            // No mesh parts (index buffers).
            if( _material )
            {
                auto technique = _material->getTechnique();
                GP_ASSERT(technique);
                size_t passCount = technique->getPassCount();
                for(size_t i = 0; i < passCount; ++i )
                {
                    auto pass = technique->getPassByIndex(i);
                    GP_ASSERT(pass);
                    pass->bind();
                    GL_ASSERT( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0) );
                    if( !wireframe || !drawWireframe(_mesh) )
                    {
                        GL_ASSERT( glDrawArrays(_mesh->getPrimitiveType(), 0, _mesh->getVertexCount()) );
                    }
                    pass->unbind();
                }
            }
        }
        else
        {
            for(size_t i = 0; i < partCount; ++i )
            {
                MeshPart* part = _mesh->getPart(i);
                GP_ASSERT(part);

                // Get the material for this mesh part.
                auto material = getMaterial(i);
                if( material )
                {
                    auto technique = material->getTechnique();
                    GP_ASSERT(technique);
                    size_t passCount = technique->getPassCount();
                    for(size_t j = 0; j < passCount; ++j )
                    {
                        auto pass = technique->getPassByIndex(j);
                        GP_ASSERT(pass);
                        pass->bind();
                        GL_ASSERT( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, part->_indexBuffer) );
                        if( !wireframe || !drawWireframe(part) )
                        {
                            GL_ASSERT( glDrawElements(part->getPrimitiveType(), part->getIndexCount(), part->getIndexFormat(), nullptr) );
                        }
                        pass->unbind();
                    }
                }
            }
        }
        return partCount;
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    void Model::setMaterialNodeBinding(const std::shared_ptr<Material>& material)
    {
        GP_ASSERT(material);

        if( _node )
        {
            material->setNodeBinding(getNode());
        }
    }


    void Model::validatePartCount()
    {
        GP_ASSERT(_mesh);
        _partCount = _mesh->getPartCount();
        _partMaterials.resize(_partCount);
    }
}
