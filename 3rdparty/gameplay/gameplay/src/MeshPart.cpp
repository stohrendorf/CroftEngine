#include "MeshPart.h"

#include "Material.h"
#include "RenderContext.h"


namespace gameplay
{
    MeshPart::MeshPart(const std::shared_ptr<gl::VertexArray>& vao)
        : m_vao{vao}
    {
    }


    MeshPart::~MeshPart() = default;


    bool MeshPart::drawWireframe() const
    {
        BOOST_ASSERT(m_vao->getIndexBuffers().size() == 1 && m_vao->getIndexBuffers()[0] != nullptr);

        for (const auto& buffer : m_vao->getIndexBuffers())
        {
            size_t indexSize;
            switch (buffer->getStorageType())
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

            for (GLsizeiptr i = 0; i < buffer->getIndexCount(); i += 3)
            {
                GL_ASSERT(glDrawElements(GL_LINE_LOOP, 3, buffer->getStorageType(), reinterpret_cast<const GLvoid*>(i*indexSize)));
            }
        }
        return true;
    }


    void MeshPart::draw(RenderContext& context) const
    {
        if( !_material )
            return;

        BOOST_ASSERT(context.getCurrentNode() != nullptr);

        for (const auto& mps : _materialParameterSetters)
            mps(*_material);

        _material->bind(*context.getCurrentNode());

        BOOST_ASSERT(m_vao->getIndexBuffers().size() == 1 && m_vao->getIndexBuffers()[0] != nullptr);

        m_vao->bind();

        if( !context.isWireframe() || !drawWireframe() )
        {
            for (const auto& buffer : m_vao->getIndexBuffers())
            {
                buffer->draw(GL_TRIANGLES);
            }
        }

        m_vao->unbind();
    }
}
