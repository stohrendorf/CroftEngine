#include "Game.h"

#include "RenderContext.h"
#include "Scene.h"

#include "gl/debuggroup.h"


void glErrorCallback(int err, const char* msg)
{
    BOOST_LOG_TRIVIAL(error) << "glfw Error " << err << ": " << msg;
}


inline const char* glDebugSourceToString(GLenum src)
{
    switch( src )
    {
        case GL_DEBUG_SOURCE_API: return "API";
        case GL_DEBUG_SOURCE_APPLICATION: return "Application";
        case GL_DEBUG_SOURCE_OTHER: return "Other";
        case GL_DEBUG_SOURCE_SHADER_COMPILER: return "Shader Compiler";
        case GL_DEBUG_SOURCE_THIRD_PARTY: return "Third Party";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "Window System";
        default: return "<unknown>";
    }
}


inline const char* glDebugTypeToString(GLenum type)
{
    switch( type )
    {
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "Deprecated Behavior";
        case GL_DEBUG_TYPE_ERROR: return "Error";
        case GL_DEBUG_TYPE_MARKER: return "Marker";
        case GL_DEBUG_TYPE_OTHER: return "Other";
        case GL_DEBUG_TYPE_PERFORMANCE: return "Performance";
        case GL_DEBUG_TYPE_POP_GROUP: return "Pop Group";
        case GL_DEBUG_TYPE_PUSH_GROUP: return "Push Group";
        case GL_DEBUG_TYPE_PORTABILITY: return "Portability";
        default: return "<unknown>";
    }
}


inline const char* glDebugSeverityToString(GLenum severity)
{
    switch( severity )
    {
        case GL_DEBUG_SEVERITY_HIGH: return "High";
        case GL_DEBUG_SEVERITY_LOW: return "Low";
        case GL_DEBUG_SEVERITY_MEDIUM: return "Medium";
        case GL_DEBUG_SEVERITY_NOTIFICATION: return "Notification";
        default: return "<unknown>";
    }
}


void GLAPIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei /*length*/, const GLchar* message, const void* /*userParam*/)
{
    if (source == GL_DEBUG_SOURCE_APPLICATION)
        return;

    BOOST_LOG_TRIVIAL(debug) << "GLDebug #" << id << ", severity " << glDebugSeverityToString(severity) << ", type " << glDebugTypeToString(type) << ", source " << glDebugSourceToString(source) << ": " << message;
}


namespace gameplay
{
    Game::Game()
        : _scene{std::make_shared<Scene>()}
    {
        glfwSetErrorCallback(&glErrorCallback);

        if( glfwInit() != GL_TRUE )
        {
            BOOST_LOG_TRIVIAL(fatal) << "Failed to initialize GLFW";
            BOOST_THROW_EXCEPTION(std::runtime_error("Failed to initialize GLFW"));
        }

        atexit(&glfwTerminate);

        // Get the window configuration values
        int width = 1280, height = 800;
        bool fullscreen = false;

        glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
        glfwWindowHint(GLFW_DEPTH_BITS, 24);
        glfwWindowHint(GLFW_SAMPLES, _multiSampling);
        glfwWindowHint(GLFW_RED_BITS, 8);
        glfwWindowHint(GLFW_GREEN_BITS, 8);
        glfwWindowHint(GLFW_BLUE_BITS, 8);
        glfwWindowHint(GLFW_ALPHA_BITS, 8);
        glfwWindowHint(GLFW_DECORATED, fullscreen ? GL_FALSE : GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#ifndef NDEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

        // Create the windows
        _window = glfwCreateWindow(width, height, "EdisonEngine", fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);
        if( _window == nullptr )
        {
            BOOST_LOG_TRIVIAL(fatal) << "Failed to create window";
            BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create window"));
        }

        glfwMakeContextCurrent(_window);

        glewExperimental = GL_TRUE; // Let GLEW ignore "GL_INVALID_ENUM in glGetString(GL_EXTENSIONS)"
        const auto err = glewInit();
        if( err != GLEW_OK )
        {
            BOOST_LOG_TRIVIAL(error) << "glewInit: " << reinterpret_cast<const char*>(glewGetErrorString(err));
        }

        glGetError(); // clear the error flag

#ifndef NDEBUG
        GL_ASSERT(glEnable(GL_DEBUG_OUTPUT));
        GL_ASSERT(glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS));

        GL_ASSERT(glDebugMessageCallback(&debugCallback, nullptr));
#endif

        glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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
                if( !node.isEnabled() )
                {
                    return;
                }

                gl::DebugGroup debugGroup{node.getId()};

                getContext().setCurrentNode(&node);

                if( auto dr = node.getDrawable() )
                {
                    dr->draw(getContext());
                }

                Visitor::visit(node);
            }
        };
    }


    void Game::render(bool wireframe)
    {
        clear(CLEAR_COLOR_DEPTH, {0, 0, 0, 0}, 1);

        RenderContext context{wireframe};
        RenderVisitor visitor{context};
        _scene->accept(visitor);
    }


    void Game::setVsync(bool enable)
    {
        _vsync = enable;
        glfwSwapInterval(enable ? 1 : 0);
    }


    bool Game::isVsync() const
    {
        return _vsync;
    }


    int Game::run()
    {
        if( _state != UNINITIALIZED )
        {
            return -1;
        }

        GL_ASSERT(glfwGetWindowSize(_window, &_width, &_height));

        // Start up game systems.
        if( !startup() )
        {
            shutdown();
            return -2;
        }

        return 0;
    }


    bool Game::startup()
    {
        if( _state != UNINITIALIZED )
        {
            return false;
        }

        setViewport(Rectangle{0.0f, 0.0f, static_cast<float>(_width), static_cast<float>(_height)});
        RenderState::initialize();

        _state = RUNNING;

        return true;
    }


    void Game::shutdown()
    {
        // Call user finalization.
        if( _state != UNINITIALIZED )
        {
            RenderState::finalize();

            _state = UNINITIALIZED;
        }
    }


    void Game::pause()
    {
        if( _state == RUNNING )
        {
            _state = PAUSED;
            _pauseStart = std::chrono::high_resolution_clock::now();
        }
    }


    void Game::resume()
    {
        if( _state == PAUSED )
        {
            _pausedTimeTotal += std::chrono::high_resolution_clock::now() - _pauseStart;
            _pauseStart = std::chrono::high_resolution_clock::now();
            _state = RUNNING;
        }
    }


    void Game::exit()
    {
        // Only perform a full/clean shutdown if GP_USE_MEM_LEAK_DETECTION is defined.
        // Every modern OS is able to handle reclaiming process memory hundreds of times
        // faster than it would take us to go through every pointer in the engine and
        // release them nicely. For large games, shutdown can end up taking long time,
        // so we'll just call ::exit(0) to force an instant shutdown.

        // End the process immediately without a full shutdown
        std::exit(0);
    }


    void Game::frame()
    {
        if( !_initialized )
        {
            // Perform lazy first time initialization
            _initialized = true;
        }

        if( _state == RUNNING )
        {
            // Graphics Rendering.
            render();

            // Update FPS.
            ++_frameCount;
            if( (getGameTime() - _frameLastFPS) >= std::chrono::seconds(1) )
            {
                _frameRate = _frameCount;
                _frameCount = 0;
                _frameLastFPS = getGameTime();
            }
        }
        else if( _state == PAUSED )
        {
            render();
        }
    }


    void Game::swapBuffers()
    {
        glfwSwapBuffers(_window);
    }


    void Game::setViewport(const Rectangle& viewport)
    {
        _viewport = viewport;
        GL_ASSERT(glViewport(static_cast<GLuint>(viewport.x), static_cast<GLuint>(viewport.y), static_cast<GLuint>(viewport.width),
            static_cast<GLuint>(viewport.height)));
    }


    void Game::clear(ClearFlags flags, const gl::RGBAF& clearColor, float clearDepth)
    {
        GLbitfield bits = 0;
        if( flags & CLEAR_COLOR )
        {
            if( clearColor != _clearColor )
            {
                glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
                _clearColor = clearColor;
            }
            bits |= GL_COLOR_BUFFER_BIT;
        }

        if( flags & CLEAR_DEPTH )
        {
            if( clearDepth != _clearDepth )
            {
                glClearDepth(clearDepth);
                _clearDepth = clearDepth;
            }
            bits |= GL_DEPTH_BUFFER_BIT;

            // We need to explicitly call the static enableDepthWrite() method on StateBlock
            // to ensure depth writing is enabled before clearing the depth buffer (and to
            // update the global StateBlock render state to reflect this).
            RenderState::StateBlock::enableDepthWrite();
        }

        glClear(bits);
    }


    void Game::clear(ClearFlags flags, float red, float green, float blue, float alpha, float clearDepth)
    {
        clear(flags, gl::RGBAF{red, green, blue, alpha}, clearDepth);
    }
}
