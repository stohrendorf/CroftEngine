#include "Base.h"
#include "VertexAttributeBinding.h"
#include "Mesh.h"
#include "Effect.h"

#include <boost/log/trivial.hpp>

namespace gameplay
{
    static GLuint __maxVertexAttribs = 0;


    VertexAttributeBinding::VertexAttributeBinding()
        : _handle(0)
    {
    }


    VertexAttributeBinding::~VertexAttributeBinding()
    {
        GL_ASSERT( glDeleteVertexArrays(1, &_handle) );
        _handle = 0;
    }


    std::shared_ptr<VertexAttributeBinding> VertexAttributeBinding::create(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<ShaderProgram>& shaderProgram)
    {
        BOOST_ASSERT(mesh);

        return create(mesh, mesh->getVertexFormat(), nullptr, shaderProgram);
    }


    std::shared_ptr<VertexAttributeBinding> VertexAttributeBinding::create(const VertexFormat& vertexFormat, void* vertexPointer, const std::shared_ptr<ShaderProgram>& shaderProgram)
    {
        return create(nullptr, vertexFormat, vertexPointer, shaderProgram);
    }


    std::shared_ptr<VertexAttributeBinding> VertexAttributeBinding::create(const std::shared_ptr<Mesh>& mesh, const VertexFormat& vertexFormat, void* vertexPointer, const std::shared_ptr<ShaderProgram>& shaderProgram)
    {
        BOOST_ASSERT(shaderProgram);

        // One-time initialization.
        if( __maxVertexAttribs == 0 )
        {
            GLint temp;
            GL_ASSERT( glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &temp) );

            __maxVertexAttribs = temp;
            if( __maxVertexAttribs <= 0 )
            {
                BOOST_LOG_TRIVIAL(error) << "The maximum number of vertex attributes supported by OpenGL on the current device is 0 or less.";
                return nullptr;
            }
        }

        // Create a new VertexAttributeBinding.
        auto binding = std::make_shared<VertexAttributeBinding>();

        if(mesh != nullptr)
        {
            GL_ASSERT(glBindBuffer(GL_ARRAY_BUFFER, 0));
            GL_ASSERT(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

            // Use hardware VAOs.
            GL_ASSERT(glGenVertexArrays(1, &binding->_handle));

            if(binding->_handle == 0)
            {
                BOOST_LOG_TRIVIAL(error) << "Failed to create VAO handle.";
                return nullptr;
            }

            GL_ASSERT(glBindVertexArray(binding->_handle));

            // Bind the Mesh VBO so our glVertexAttribPointer calls use it.
            GL_ASSERT(glBindBuffer(GL_ARRAY_BUFFER, mesh->getVertexBuffer()));
        }

        // Call setVertexAttribPointer for each vertex element.
        size_t offset = 0;
        for( size_t i = 0, count = vertexFormat.getElementCount(); i < count; ++i )
        {
            const VertexFormat::Element& e = vertexFormat.getElement(i);
            gameplay::VertexAttribute attrib = -1;
            void* pointer = vertexPointer ? static_cast<void*>(static_cast<unsigned char*>(vertexPointer) + offset) : reinterpret_cast<void*>(offset);

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
                case VertexFormat::BLENDWEIGHTS:
                    attrib = shaderProgram->getVertexAttribute(VERTEX_ATTRIBUTE_BLENDWEIGHTS_NAME);
                    break;
                case VertexFormat::BLENDINDICES:
                    attrib = shaderProgram->getVertexAttribute(VERTEX_ATTRIBUTE_BLENDINDICES_NAME);
                    break;
                case VertexFormat::TEXCOORD0:
                    if((attrib = shaderProgram->getVertexAttribute(VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME)) != -1)
                        break;

                case VertexFormat::TEXCOORD1:
                case VertexFormat::TEXCOORD2:
                case VertexFormat::TEXCOORD3:
                case VertexFormat::TEXCOORD4:
                case VertexFormat::TEXCOORD5:
                case VertexFormat::TEXCOORD6:
                case VertexFormat::TEXCOORD7:
                    break;

                default:
                    // This happens whenever vertex data contains extra information (not an error).
                    attrib = -1;
                    break;
            }

            if(attrib != -1)
            {
                if(e.usage != VertexFormat::BLENDINDICES)
                    binding->setVertexAttribPointer(attrib, static_cast<GLint>(e.size), GL_FLOAT, GL_FALSE, static_cast<GLsizei>(vertexFormat.getVertexSize()), pointer);
                else
                    binding->setVertexAttribPointer(attrib, static_cast<GLint>(e.size), GL_INT, GL_FALSE, static_cast<GLsizei>(vertexFormat.getVertexSize()), pointer);
            }
            else
            {
                BOOST_LOG_TRIVIAL(warning) << "Vertex element with usage '" << e.usage << "' does not correspond to an attribute in shader '" << shaderProgram->getId() << "'.";
            }
            offset += e.size * sizeof(float);
        }

        GL_ASSERT( glBindVertexArray(0) );

        return binding;
    }


    void VertexAttributeBinding::setVertexAttribPointer(GLuint indx, GLint size, GLenum type, GLboolean normalize, GLsizei stride, void* pointer)
    {
        BOOST_ASSERT(indx < __maxVertexAttribs);

        // Hardware mode.
        GL_ASSERT( glVertexAttribPointer(indx, size, type, normalize, stride, pointer) );
        GL_ASSERT( glEnableVertexAttribArray(indx) );
    }


    void VertexAttributeBinding::bind()
    {
        GL_ASSERT( glBindVertexArray(_handle) );
    }


    void VertexAttributeBinding::unbind()
    {
        GL_ASSERT( glBindVertexArray(0) );
    }
}
