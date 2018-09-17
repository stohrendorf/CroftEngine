#include "Game.h"

#include "RenderContext.h"
#include "Scene.h"

#include "gl/debuggroup.h"

namespace
{
void glErrorCallback(int err, const char* msg)
{
    BOOST_LOG_TRIVIAL( error ) << "glfw Error " << err << ": " << msg;
}

inline const char* glDebugSourceToString(GLenum src)
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

inline const char* glDebugTypeToString(GLenum type)
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

inline const char* glDebugSeverityToString(GLenum severity)
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

void GLAPIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei /*length*/,
                              const GLchar* message, const void* /*userParam*/)
{
    if( source == GL_DEBUG_SOURCE_APPLICATION )
        return;

    BOOST_LOG_TRIVIAL( debug ) << "GLDebug #" << id << ", severity " << glDebugSeverityToString( severity ) << ", type "
                               << glDebugTypeToString( type ) << ", source " << glDebugSourceToString( source ) << ": "
                               << message;
}
}

namespace gameplay
{
Game::Game()
        : m_scene{std::make_shared<Scene>()}
{
    glfwSetErrorCallback( &glErrorCallback );

    if( glfwInit() != GL_TRUE )
    {
        BOOST_LOG_TRIVIAL( fatal ) << "Failed to initialize GLFW";
        BOOST_THROW_EXCEPTION( std::runtime_error( "Failed to initialize GLFW" ) );
    }

    atexit( &glfwTerminate );

    // Get the window configuration values
    int width = 1280, height = 800;
    bool fullscreen = false;

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
    m_window = glfwCreateWindow( width, height, "EdisonEngine", fullscreen ? glfwGetPrimaryMonitor() : nullptr,
                                 nullptr );
    if( m_window == nullptr )
    {
        BOOST_LOG_TRIVIAL( fatal ) << "Failed to create window";
        BOOST_THROW_EXCEPTION( std::runtime_error( "Failed to create window" ) );
    }

    glfwMakeContextCurrent( m_window );

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
}

Game::~Game() = default;

namespace
{
class RenderVisitor
        : public Visitor
{
public:
    explicit RenderVisitor(RenderContext& context)
            : Visitor{context}
    {
    }

    void visit(Node& node) override
    {
        if( !node.isVisible() )
        {
            return;
        }

        gl::DebugGroup debugGroup{node.getId()};

        getContext().setCurrentNode( &node );

        if( auto dr = node.getDrawable() )
        {
            dr->draw( getContext() );
        }

        Visitor::visit( node );
    }
};
}

void Game::setVsync(bool enable)
{
    m_vsync = enable;
    glfwSwapInterval( enable ? 1 : 0 );
}

bool Game::isVsync() const
{
    return m_vsync;
}

void Game::initialize()
{
    if( m_initialized )
    {
        return;
    }

    updateWindowSize();
    RenderState::initDefaults();

    glEnable( GL_FRAMEBUFFER_SRGB );
    gl::checkGlError();

    m_initialized = true;
}

bool Game::updateWindowSize()
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

void Game::render()
{
    // Graphics Rendering.
    clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, {0, 0, 0, 0}, 1 );

    RenderContext context{};
    RenderVisitor visitor{context};
    m_scene->accept( visitor );

    // Update FPS.
    ++m_frameCount;
    if( (getGameTime() - m_frameLastFPS) >= std::chrono::seconds( 1 ) )
    {
        m_frameRate = m_frameCount;
        m_frameCount = 0;
        m_frameLastFPS = getGameTime();
    }
}

void Game::swapBuffers()
{
    glfwSwapBuffers( m_window );
}

void Game::setViewport(const Dimension2<size_t>& viewport)
{
    m_viewport = viewport;
    GL_ASSERT( glViewport( 0, 0,
                           gsl::narrow<GLuint>( viewport.width ),
                           gsl::narrow<GLuint>( viewport.height ) ) );
}

void Game::clear(GLbitfield flags, const gl::RGBA8& clearColor, float clearDepth)
{
    GLbitfield bits = 0;
    if( flags & GL_COLOR_BUFFER_BIT )
    {
        if( clearColor != m_clearColor )
        {
            GL_ASSERT( glClearColor( clearColor.r / 255.0f, clearColor.g / 255.0f, clearColor.b / 255.0f,
                                     clearColor.a / 255.0f ) );
            m_clearColor = clearColor;
        }
        bits |= GL_COLOR_BUFFER_BIT;
    }

    if( flags & GL_DEPTH_BUFFER_BIT )
    {
        if( clearDepth != m_clearDepth )
        {
            GL_ASSERT( glClearDepth( clearDepth ) );
            m_clearDepth = clearDepth;
        }
        bits |= GL_DEPTH_BUFFER_BIT;

        // We need to explicitly call the static enableDepthWrite() method on StateBlock
        // to ensure depth writing is enabled before clearing the depth buffer (and to
        // update the global StateBlock render state to reflect this).
        RenderState::enableDepthWrite();
    }

    GL_ASSERT( glClear( bits ) );
}

void Game::clear(GLbitfield flags, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha, float clearDepth)
{
    clear( flags, gl::RGBA8{red, green, blue, alpha}, clearDepth );
}
}
