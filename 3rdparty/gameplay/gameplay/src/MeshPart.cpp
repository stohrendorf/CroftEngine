#include "Base.h"
#include "MeshPart.h"


#include <boost/log/trivial.hpp>
#include "Material.h"


namespace gameplay
{
    MeshPart::MeshPart()
        : _mesh()
        , _meshIndex(0)
        , _primitiveType(Mesh::TRIANGLES)
        , _indexFormat()
        , _indexCount(0)
        , _indexBuffer(0)
        , _dynamic(false)
    {
    }


    MeshPart::~MeshPart()
    {
        if( _indexBuffer )
        {
            glDeleteBuffers(1, &_indexBuffer);
        }
    }


    std::shared_ptr<MeshPart> MeshPart::create(const std::weak_ptr<Mesh>& mesh, size_t meshIndex, Mesh::PrimitiveType primitiveType,
                                               Mesh::IndexFormat indexFormat, size_t indexCount, bool dynamic)
    {
        // Create a VBO for our index buffer.
        VertexBufferHandle vbo;
        GL_ASSERT( glGenBuffers(1, &vbo) );
        GL_ASSERT( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo) );

        size_t indexSize = 0;
        switch( indexFormat )
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
                BOOST_LOG_TRIVIAL(error) << "Unsupported index format (" << indexFormat << ").";
                glDeleteBuffers(1, &vbo);
                return nullptr;
        }

        GL_ASSERT( glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize * indexCount, nullptr, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW) );

        auto part = std::make_shared<MeshPart>();
        part->_mesh = mesh;
        part->_meshIndex = meshIndex;
        part->_primitiveType = primitiveType;
        part->_indexFormat = indexFormat;
        part->_indexCount = indexCount;
        part->_indexBuffer = vbo;
        part->_dynamic = dynamic;

        return part;
    }


    size_t MeshPart::getMeshIndex() const
    {
        return _meshIndex;
    }


    Mesh::PrimitiveType MeshPart::getPrimitiveType() const
    {
        return _primitiveType;
    }


    size_t MeshPart::getIndexCount() const
    {
        return _indexCount;
    }


    Mesh::IndexFormat MeshPart::getIndexFormat() const
    {
        return _indexFormat;
    }


    IndexBufferHandle MeshPart::getIndexBuffer() const
    {
        return _indexBuffer;
    }


    bool MeshPart::isDynamic() const
    {
        return _dynamic;
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    void MeshPart::setIndexData(const void* indexData, size_t indexStart, size_t indexCount)
    {
        GL_ASSERT( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer) );

        size_t indexSize = 0;
        switch( _indexFormat )
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
                BOOST_LOG_TRIVIAL(error) << "Unsupported index format (" << _indexFormat << ").";
                return;
        }

        if( indexStart == 0 && indexCount == 0 )
        {
            GL_ASSERT( glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize * _indexCount, indexData, _dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW) );
        }
        else
        {
            if( indexCount == 0 )
            {
                indexCount = _indexCount - indexStart;
            }

            GL_ASSERT( glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indexStart * indexSize, indexCount * indexSize, indexData) );
        }
    }

    void MeshPart::setMaterial(const std::shared_ptr<Material>& material)
    {
        BOOST_ASSERT(!_mesh.expired());

        _material = material;

        // Hookup vertex attribute bindings for all passes in the new material.
        auto t = material->getTechnique();
        BOOST_ASSERT(t);
        auto p = t->getPass();
        BOOST_ASSERT(p);

        _vaBinding = VertexAttributeBinding::create(_mesh.lock(), p->getShaderProgram());
    }

}
