#include "VertexAttributeBinding.h"

#include "Mesh.h"
#include "ShaderProgram.h"

#include <boost/log/trivial.hpp>


namespace gameplay
{
    namespace
    {
        GLuint maxVertexAttribs = 0;
    }


    VertexAttributeBinding::VertexAttributeBinding(Mesh* mesh, const VertexFormat& vertexFormat, void* vertexPointer, const std::shared_ptr<ShaderProgram>& shaderProgram)
        : VertexArrayHandle()
    {
        BOOST_ASSERT(shaderProgram);

        // One-time initialization.
        if( maxVertexAttribs == 0 )
        {
            GLint temp;
            GL_ASSERT(glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &temp));

            maxVertexAttribs = temp;
            if( maxVertexAttribs <= 0 )
            {
                BOOST_LOG_TRIVIAL(error) << "The maximum number of vertex attributes supported by OpenGL on the current device is 0 or less.";
                BOOST_THROW_EXCEPTION(std::runtime_error("The maximum number of vertex attributes supported by OpenGL on the current device is 0 or less."));
            }
        }

        if( mesh != nullptr )
        {
            GL_ASSERT(glBindBuffer(GL_ARRAY_BUFFER, 0));
            GL_ASSERT(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

            // Use hardware VAOs.
            bind();

            // Bind the Mesh VBO so our glVertexAttribPointer calls use it.
            mesh->bind();
        }

        // Call setVertexAttribPointer for each vertex element.
        size_t offset = 0;
        for( size_t i = 0, count = vertexFormat.getElementCount(); i < count; ++i )
        {
            const VertexFormat::Element& e = vertexFormat.getElement(i);
            gameplay::VertexAttribute attrib = -1;
            void* pointer = vertexPointer ? static_cast<void*>(static_cast<uint8_t*>(vertexPointer) + offset) : reinterpret_cast<void*>(offset);

            // Constructor vertex attribute name expected in shader.
            switch( e.usage )
            {
                case VertexFormat::POSITION:
                    attrib = shaderProgram->getVertexAttribute(VERTEX_ATTRIBUTE_POSITION_NAME);
                    break;
                case VertexFormat::NORMAL:
                    attrib = shaderProgram->getVertexAttribute(VERTEX_ATTRIBUTE_NORMAL_NAME);
                    break;
                case VertexFormat::COLOR:
                    attrib = shaderProgram->getVertexAttribute(VERTEX_ATTRIBUTE_COLOR_NAME);
                    break;
                case VertexFormat::TANGENT:
                    attrib = shaderProgram->getVertexAttribute(VERTEX_ATTRIBUTE_TANGENT_NAME);
                    break;
                case VertexFormat::BINORMAL:
                    attrib = shaderProgram->getVertexAttribute(VERTEX_ATTRIBUTE_BINORMAL_NAME);
                    break;
                case VertexFormat::TEXCOORD:
                    attrib = shaderProgram->getVertexAttribute(VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME);
                    break;
                default:
                    // This happens whenever vertex data contains extra information (not an error).
                    attrib = -1;
                    break;
            }

            if( attrib != -1 )
            {
                setVertexAttribPointer(attrib, static_cast<GLint>(e.size), GL_FLOAT, GL_FALSE, static_cast<GLsizei>(vertexFormat.getVertexSize()), pointer);
            }
            else
            {
                BOOST_LOG_TRIVIAL(warning) << "Vertex element with usage '" << e.usage << "' does not correspond to an attribute in shader '" << shaderProgram->getId() << "'.";
            }
            offset += e.size * sizeof(float);
        }

        unbind();
    }


    VertexAttributeBinding::VertexAttributeBinding(Mesh* mesh, const std::shared_ptr<ShaderProgram>& shaderProgram)
        : VertexAttributeBinding{mesh, mesh->getVertexFormat(), nullptr, shaderProgram}
    {
    }


    VertexAttributeBinding::~VertexAttributeBinding() = default;


    void VertexAttributeBinding::setVertexAttribPointer(GLuint indx, GLint size, GLenum type, GLboolean normalize, GLsizei stride, void* pointer)
    {
        BOOST_ASSERT(indx < maxVertexAttribs);

        // Hardware mode.
        GL_ASSERT( glVertexAttribPointer(indx, size, type, normalize, stride, pointer) );
        GL_ASSERT( glEnableVertexAttribArray(indx) );
    }
}
