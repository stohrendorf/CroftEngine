#include "Base.h"
#include "MeshPart.h"


#include <boost/log/trivial.hpp>
#include "Material.h"


namespace gameplay
{
    MeshPart::MeshPart() = default;


    MeshPart::~MeshPart() = default;


    std::shared_ptr<MeshPart> MeshPart::create(const std::weak_ptr<Mesh>& mesh,
                                               Mesh::PrimitiveType primitiveType,
                                               Mesh::IndexFormat indexFormat,
                                               size_t indexCount,
                                               bool dynamic)
    {
        auto part = std::make_shared<MeshPart>();
        part->_indexBuffer.bind();

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
                return nullptr;
        }

        GL_ASSERT( glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize * indexCount, nullptr, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW) );

        part->_mesh = mesh;
        part->_primitiveType = primitiveType;
        part->_indexFormat = indexFormat;
        part->_indexCount = indexCount;
        part->_dynamic = dynamic;

        return part;
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


    const IndexBufferHandle& MeshPart::getIndexBuffer() const
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
        _indexBuffer.bind();

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
        _vaBinding = VertexAttributeBinding::create(_mesh.lock(), material->getShaderProgram());
    }
}
