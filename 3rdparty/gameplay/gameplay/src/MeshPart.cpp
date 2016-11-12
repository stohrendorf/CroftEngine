#include "Base.h"
#include "MeshPart.h"


#include <boost/log/trivial.hpp>
#include "Material.h"


namespace gameplay
{
    MeshPart::MeshPart(Mesh* mesh,
                       Mesh::PrimitiveType primitiveType,
                       Mesh::IndexFormat indexFormat,
                       size_t indexCount,
                       bool dynamic)
        : _mesh{mesh}
        , _primitiveType{primitiveType}
        , _indexFormat{indexFormat}
        , _indexCount{indexCount}
        , _dynamic{dynamic}
    {
        bind();

        size_t indexSize;
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
                BOOST_THROW_EXCEPTION(std::runtime_error("Unsupported index format"));
        }

        GL_ASSERT(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize * indexCount, nullptr, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW));
    }


    MeshPart::~MeshPart() = default;


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


    bool MeshPart::isDynamic() const
    {
        return _dynamic;
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    void MeshPart::setIndexData(const void* indexData, size_t indexStart, size_t indexCount)
    {
        bind();

        size_t indexSize;
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
        BOOST_ASSERT(_mesh != nullptr);

        _material = material;
        _vaBinding = std::make_shared<VertexAttributeBinding>(_mesh, material->getShaderProgram());
    }
}
