#include "window.h"

#include "render/gl/glassert.h"
#include "gsl-lite.hpp"

namespace render
{
namespace scene
{
namespace
{
void glErrorCallback(const int err, const char* msg)
{
    BOOST_LOG_TRIVIAL( error ) << "glfw Error " << err << ": " << msg;
}
}

Window::Window(bool fullscreen, const Dimension2<int>& resolution)
{
    glfwSetErrorCallback( &glErrorCallback );

    if( glfwInit() != GL_TRUE )
    {
        BOOST_LOG_TRIVIAL( fatal ) << "Failed to initialize GLFW";
        BOOST_THROW_EXCEPTION( std::runtime_error( "Failed to initialize GLFW" ) );
    }

    atexit( &glfwTerminate );

    glfwWindowHint( GLFW_DOUBLEBUFFER, GL_TRUE );
    glfwWindowHint( GLFW_DEPTH_BITS, 24 );
    // glfwWindowHint( GLFW_SAMPLES, m_multiSampling );
    glfwWindowHint( GLFW_RED_BITS, 8 );
    glfwWindowHint( GLFW_GREEN_BITS, 8 );
    glfwWindowHint( GLFW_BLUE_BITS, 8 );
    glfwWindowHint( GLFW_ALPHA_BITS, 8 );
    glfwWindowHint( GLFW_DECORATED, fullscreen ? GL_FALSE : GL_TRUE );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
#ifndef NDEBUG
    glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE );
#endif

    // Create the windows
    m_window = glfwCreateWindow( resolution.width, resolution.height, "EdisonEngine",
                                 fullscreen ? glfwGetPrimaryMonitor() : nullptr,
                                 nullptr );
    if( m_window == nullptr )
    {
        BOOST_LOG_TRIVIAL( fatal ) << "Failed to create window";
        BOOST_THROW_EXCEPTION( std::runtime_error( "Failed to create window" ) );
    }

    glfwMakeContextCurrent( m_window );

    render::gl::initializeGl();

    updateWindowSize();

#ifdef NDEBUG
    glfwSetInputMode( m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
#endif
}

void Window::setVsync(const bool enable)
{
    m_vsync = enable;
    glfwSwapInterval( enable ? 1 : 0 );
}

bool Window::isVsync() const
{
    return m_vsync;
}

bool Window::updateWindowSize()
{
    int tmpW, tmpH;
    GL_ASSERT( glfwGetFramebufferSize( m_window, &tmpW, &tmpH ) );

    if( tmpW == m_viewport.width && tmpH == m_viewport.height )
        return false;

    m_viewport.width = gsl::narrow<size_t>( tmpW );
    m_viewport.height = gsl::narrow<size_t>( tmpH );

    setViewport( m_viewport );
    return true;
}

void Window::swapBuffers() const
{
    glfwSwapBuffers( m_window );
}

void Window::setViewport(const Dimension2<size_t>& viewport)
{
    m_viewport = viewport;
    GL_ASSERT( glViewport( 0, 0,
                           gsl::narrow<GLuint>( viewport.width ),
                           gsl::narrow<GLuint>( viewport.height ) ) );
}
}
}
