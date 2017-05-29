#include "MeshPart.h"

#include "Material.h"
#include "RenderContext.h"


namespace gameplay
{
    MeshPart::MeshPart(const gsl::not_null<Mesh*>& mesh,
                       GLint indexFormat)
        : _mesh{mesh}
        , _indexFormat{indexFormat}
    {
    }


    MeshPart::~MeshPart() = default;


    void MeshPart::setMaterial(const std::shared_ptr<Material>& material)
    {
        BOOST_ASSERT(_mesh != nullptr);

        _material = material;
    }


    bool MeshPart::drawWireframe() const
    {
        size_t indexSize;
        switch( _indexFormat )
        {
            case gl::TypeTraits<uint8_t>::TypeId:
                indexSize = 1;
                break;
            case gl::TypeTraits<uint16_t>::TypeId:
                indexSize = 2;
                break;
            case gl::TypeTraits<uint32_t>::TypeId:
                indexSize = 4;
                break;
            default:
                BOOST_LOG_TRIVIAL(error) << "Unsupported index format";
                return false;
        }

        for( GLsizeiptr i = 0; i < getIndexCount(); i += 3 )
        {
            GL_ASSERT(glDrawElements(GL_LINE_LOOP, 3, static_cast<GLenum>(_indexFormat), reinterpret_cast<const GLvoid*>(i*indexSize)));
        }
        return true;
    }


    void MeshPart::draw(RenderContext& context) const
    {
        if( !_material )
            return;

        BOOST_ASSERT(context.getCurrentNode() != nullptr);

        for( const auto& mps : _materialParameterSetters )
            mps(*_material);

        _material->bind(*context.getCurrentNode());

        if( m_vao == nullptr )
        {
            m_vao = std::make_shared<gl::VertexArray>();
            m_vao->bind();
            bind();
            for( const auto& buffer : _mesh->getBuffers() )
                buffer.bind(_material->getShaderProgram()->getHandle());
            m_vao->unbind();
        }

        m_vao->bind();

        if( !context.isWireframe() || !drawWireframe() )
        {
            switch( _indexFormat )
            {
                case gl::TypeTraits<uint8_t>::TypeId:
                    IndexBuffer::draw<uint8_t>(GL_TRIANGLES);
                    break;
                case gl::TypeTraits<uint16_t>::TypeId:
                    IndexBuffer::draw<uint16_t>(GL_TRIANGLES);
                    break;
                case gl::TypeTraits<uint32_t>::TypeId:
                    IndexBuffer::draw<uint32_t>(GL_TRIANGLES);
                    break;
                default:
                    BOOST_LOG_TRIVIAL(error) << "Unsupported index format";
                    break;
            }
        }

        m_vao->unbind();
    }
}
