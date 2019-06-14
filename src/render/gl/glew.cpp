#include "glew.h"

#include "glassert.h"
#include "renderstate.h"

#include <glbinding/glbinding.h>
#include <GLFW/glfw3.h>

using namespace render::gl;

namespace
{
inline const char* glDebugSourceToString(const ::gl::GLenum src)
{
    switch( src )
    {
        case ::gl::GL_DEBUG_SOURCE_API:
            return "API";
        case ::gl::GL_DEBUG_SOURCE_APPLICATION:
            return "Application";
        case ::gl::GL_DEBUG_SOURCE_OTHER:
            return "Other";
        case ::gl::GL_DEBUG_SOURCE_SHADER_COMPILER:
            return "Shader Compiler";
        case ::gl::GL_DEBUG_SOURCE_THIRD_PARTY:
            return "Third Party";
        case ::gl::GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            return "Window System";
        default:
            return "<unknown>";
    }
}

inline const char* glDebugTypeToString(const ::gl::GLenum type)
{
    switch( type )
    {
        case ::gl::GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            return "Deprecated Behavior";
        case ::gl::GL_DEBUG_TYPE_ERROR:
            return "Error";
        case ::gl::GL_DEBUG_TYPE_MARKER:
            return "Marker";
        case ::gl::GL_DEBUG_TYPE_OTHER:
            return "Other";
        case ::gl::GL_DEBUG_TYPE_PERFORMANCE:
            return "Performance";
        case ::gl::GL_DEBUG_TYPE_POP_GROUP:
            return "Pop Group";
        case ::gl::GL_DEBUG_TYPE_PUSH_GROUP:
            return "Push Group";
        case ::gl::GL_DEBUG_TYPE_PORTABILITY:
            return "Portability";
        default:
            return "<unknown>";
    }
}

inline const char* glDebugSeverityToString(const ::gl::GLenum severity)
{
    switch( severity )
    {
        case ::gl::GL_DEBUG_SEVERITY_HIGH:
            return "High";
        case ::gl::GL_DEBUG_SEVERITY_LOW:
            return "Low";
        case ::gl::GL_DEBUG_SEVERITY_MEDIUM:
            return "Medium";
        case ::gl::GL_DEBUG_SEVERITY_NOTIFICATION:
            return "Notification";
        default:
            return "<unknown>";
    }
}

void
debugCallback(const ::gl::GLenum source, const ::gl::GLenum type, const ::gl::GLuint id, const ::gl::GLenum severity,
              const ::gl::GLsizei /*length*/, const ::gl::GLchar* message, const void* /*userParam*/)
{
    if( source == ::gl::GL_DEBUG_SOURCE_APPLICATION )
        return;

    BOOST_LOG_TRIVIAL( debug ) << "GLDebug #" << id << ", severity " << glDebugSeverityToString( severity ) << ", type "
                               << glDebugTypeToString( type ) << ", source " << glDebugSourceToString( source ) << ": "
                               << message;
}
}

void render::gl::initializeGl()
{
    glbinding::initialize( glfwGetProcAddress );

#ifndef NDEBUG
    GL_ASSERT( glEnable( ::gl::GL_DEBUG_OUTPUT ) );
    GL_ASSERT( glEnable( ::gl::GL_DEBUG_OUTPUT_SYNCHRONOUS ) );

    GL_ASSERT( glDebugMessageCallback( &debugCallback, nullptr ) );
#endif

    RenderState::initDefaults();

    GL_ASSERT( glEnable( ::gl::GL_FRAMEBUFFER_SRGB ) );
}
