#include "glew.h"

#include "glassert.h"
#include "renderstate.h"

using namespace render::gl;

namespace
{
inline const char* glDebugSourceToString(const GLenum src)
{
    switch( src )
    {
        case GL_DEBUG_SOURCE_API:
            return "API";
        case GL_DEBUG_SOURCE_APPLICATION:
            return "Application";
        case GL_DEBUG_SOURCE_OTHER:
            return "Other";
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            return "Shader Compiler";
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            return "Third Party";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            return "Window System";
        default:
            return "<unknown>";
    }
}

inline const char* glDebugTypeToString(const GLenum type)
{
    switch( type )
    {
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            return "Deprecated Behavior";
        case GL_DEBUG_TYPE_ERROR:
            return "Error";
        case GL_DEBUG_TYPE_MARKER:
            return "Marker";
        case GL_DEBUG_TYPE_OTHER:
            return "Other";
        case GL_DEBUG_TYPE_PERFORMANCE:
            return "Performance";
        case GL_DEBUG_TYPE_POP_GROUP:
            return "Pop Group";
        case GL_DEBUG_TYPE_PUSH_GROUP:
            return "Push Group";
        case GL_DEBUG_TYPE_PORTABILITY:
            return "Portability";
        default:
            return "<unknown>";
    }
}

inline const char* glDebugSeverityToString(const GLenum severity)
{
    switch( severity )
    {
        case GL_DEBUG_SEVERITY_HIGH:
            return "High";
        case GL_DEBUG_SEVERITY_LOW:
            return "Low";
        case GL_DEBUG_SEVERITY_MEDIUM:
            return "Medium";
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            return "Notification";
        default:
            return "<unknown>";
    }
}

void GLAPIENTRY debugCallback(const GLenum source, const GLenum type, const GLuint id, const GLenum severity,
                              const GLsizei /*length*/, const GLchar* message, const void* /*userParam*/)
{
    if( source == GL_DEBUG_SOURCE_APPLICATION )
        return;

    BOOST_LOG_TRIVIAL( debug ) << "GLDebug #" << id << ", severity " << glDebugSeverityToString( severity ) << ", type "
                               << glDebugTypeToString( type ) << ", source " << glDebugSourceToString( source ) << ": "
                               << message;
}
}

void render::gl::initializeGl()
{
    glewExperimental = GL_TRUE; // Let GLEW ignore "GL_INVALID_ENUM in glGetString(GL_EXTENSIONS)"
    const auto err = glewInit();
    if( err != GLEW_OK )
    {
        BOOST_LOG_TRIVIAL( error ) << "glewInit: " << reinterpret_cast<const char*>(glewGetErrorString( err ));
        BOOST_THROW_EXCEPTION( std::runtime_error( "Failed to initialize GLEW" ) );
    }

    glGetError(); // clear the error flag

#ifndef NDEBUG
    GL_ASSERT( glEnable( GL_DEBUG_OUTPUT ) );
    GL_ASSERT( glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS ) );

    GL_ASSERT( glDebugMessageCallback( &debugCallback, nullptr ) );
#else
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
#endif

    RenderState::initDefaults();

    GL_ASSERT( glEnable( GL_FRAMEBUFFER_SRGB ) );
}
