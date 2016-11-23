#include "Base.h"
#include "Game.h"
#include "RenderState.h"
#include "FrameBuffer.h"
#include "Scene.h"

#include <boost/log/trivial.hpp>

/** @script{ignore} */
GLenum __gl_error_code = GL_NO_ERROR;


void glErrorCallback(int err, const char* msg)
{
    BOOST_LOG_TRIVIAL(error) << "glfw Error " << err << ": " << msg;
}


void GLAPIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei /*length*/, const GLchar* message, const void* /*userParam*/)
{
    BOOST_LOG_TRIVIAL(debug) << "GLDebug #" << id << ", severity " << severity << ", type " << type << ", source " << source << ": " << message;
}


namespace gameplay
{
    std::chrono::microseconds Game::_pausedTimeLast = std::chrono::microseconds::zero();
    std::chrono::microseconds Game::_pausedTimeTotal = std::chrono::microseconds::zero();


    Game::Game()
        : _initialized(false)
        , _state(UNINITIALIZED)
        , _pausedCount(0)
        , _frameLastFPS(0)
        , _frameCount(0)
        , _frameRate(0)
        , _width(0)
        , _height(0)
        , _clearDepth(1.0f)
        , _clearStencil(0)
        , _scene{std::make_shared<Scene>()}
    {
        glfwSetErrorCallback(&glErrorCallback);

        if( glfwInit() != GLFW_TRUE )
        {
            BOOST_LOG_TRIVIAL(fatal) << "Failed to initialize GLFW";
            BOOST_THROW_EXCEPTION(std::runtime_error("Failed to initialize GLFW"));
        }

        atexit(&glfwTerminate);

        // Get the window configuration values
        int width = 1280, height = 800, samples = 1;
        bool fullscreen = false;

        glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
        glfwWindowHint(GLFW_DEPTH_BITS, 24);
        glfwWindowHint(GLFW_STENCIL_BITS, 8);
        glfwWindowHint(GLFW_SAMPLES, samples);
        glfwWindowHint(GLFW_RED_BITS, 8);
        glfwWindowHint(GLFW_GREEN_BITS, 8);
        glfwWindowHint(GLFW_BLUE_BITS, 8);
        glfwWindowHint(GLFW_ALPHA_BITS, 8);
        glfwWindowHint(GLFW_DECORATED, fullscreen ? GLFW_FALSE : GLFW_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
#ifndef NDEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

        _multiSampling = samples > 0;

        // Create the windows
        _window = glfwCreateWindow(width, height, "EdisonEngine", fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);
        if(_window == nullptr)
        {
            BOOST_LOG_TRIVIAL(fatal) << "Failed to create window";
            BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create window"));
        }

        glfwMakeContextCurrent(_window);

        glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        // Use OpenGL 2.x with GLEW
        glewExperimental = GL_TRUE;
        const auto err = glewInit();
        if( err != GLEW_OK )
        {
            BOOST_LOG_TRIVIAL(error) << "glewInit: " << reinterpret_cast<const char*>(glewGetErrorString(err));
        }

#ifndef NDEBUG
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

        glDebugMessageCallback(&debugCallback, nullptr);
#endif
    }


    Game::~Game()
    {
        // Do not call any virtual functions from the destructor.
        // Finalization is done from outside this class.
#ifdef GP_USE_MEM_LEAK_DETECTION
    Ref::printLeaks();
    printMemoryLeaks();
#endif
    }


    bool Game::drawNode(RenderContext& context)
    {
        BOOST_ASSERT(context.getCurrentNode() != nullptr);
        auto dr = context.getCurrentNode()->getDrawable();
        if( dr != nullptr )
        {
            dr->draw(context);
        }

        return true;
    }


    void Game::render(bool wireframe)
    {
        clear(CLEAR_COLOR_DEPTH, {0,0,0,0}, 1, 0);

        RenderContext context{wireframe};
        _scene->visit(context, &Game::drawNode);
    }


    std::chrono::microseconds Game::getAbsoluteTime()
    {
        _timeAbsolute = std::chrono::high_resolution_clock::now() - _timeStart;

        return std::chrono::duration_cast<std::chrono::microseconds>(_timeAbsolute);
    }


    std::chrono::microseconds Game::getGameTime()
    {
        return getAbsoluteTime() - _pausedTimeTotal;
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
            return -1;

        glfwGetWindowSize(_window, &_width, &_height);

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
            return false;

        setViewport(Rectangle(0.0f, 0.0f, static_cast<float>(_width), static_cast<float>(_height)));
        RenderState::initialize();
        FrameBuffer::initialize();

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
            _pausedTimeLast = getAbsoluteTime();
        }

        ++_pausedCount;
    }


    void Game::resume()
    {
        if( _state == PAUSED )
        {
            --_pausedCount;

            if( _pausedCount == 0 )
            {
                _state = RUNNING;
                _pausedTimeTotal += getAbsoluteTime() - _pausedTimeLast;
            }
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
        ::exit(0);
    }


    void Game::frame()
    {
        if( !_initialized )
        {
            // Perform lazy first time initialization
            _initialized = true;
        }

        if( _state == Game::RUNNING )
        {
            // Graphics Rendering.
            render();

            // Update FPS.
            ++_frameCount;
            if( (Game::getGameTime() - _frameLastFPS) >= std::chrono::seconds(1) )
            {
                _frameRate = _frameCount;
                _frameCount = 0;
                _frameLastFPS = getGameTime();
            }
        }
        else if( _state == Game::PAUSED )
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
        glViewport(static_cast<GLuint>(viewport.x), static_cast<GLuint>(viewport.y), static_cast<GLuint>(viewport.width), static_cast<GLuint>(viewport.height));
    }


    void Game::clear(ClearFlags flags, const glm::vec4& clearColor, float clearDepth, int clearStencil)
    {
        GLbitfield bits = 0;
        if( flags & CLEAR_COLOR )
        {
            if( clearColor.x != _clearColor.x ||
                clearColor.y != _clearColor.y ||
                clearColor.z != _clearColor.z ||
                clearColor.w != _clearColor.w )
            {
                glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
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

        if( flags & CLEAR_STENCIL )
        {
            if( clearStencil != _clearStencil )
            {
                glClearStencil(clearStencil);
                _clearStencil = clearStencil;
            }
            bits |= GL_STENCIL_BUFFER_BIT;
        }
        glClear(bits);
    }


    void Game::clear(ClearFlags flags, float red, float green, float blue, float alpha, float clearDepth, int clearStencil)
    {
        clear(flags, glm::vec4(red, green, blue, alpha), clearDepth, clearStencil);
    }
}
