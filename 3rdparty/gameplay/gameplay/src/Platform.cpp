#include "Base.h"
#include "Platform.h"
#include "FileSystem.h"
#include "Game.h"
#include "Properties.h"

#include <boost/log/trivial.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

static std::chrono::high_resolution_clock::time_point __timeStart;
static std::chrono::high_resolution_clock::duration __timeAbsolute;
static bool __vsync = WINDOW_VSYNC;
static bool __multiSampling = false;
static GLFWwindow* __window;


namespace gameplay
{
    void glErrorCallback(int err, const char* msg)
    {
        BOOST_LOG_TRIVIAL(error) << "glfw Error " << err << ": " << msg;
    }

    Platform::Platform(Game* game)
        : _game(game)
    {
    }


    Platform::~Platform() = default;


    Platform* Platform::create(Game* game)
    {
        BOOST_ASSERT(game);

        glfwSetErrorCallback(&glErrorCallback);

        if( glfwInit() != GLFW_TRUE )
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to initialize GLFW";
            return nullptr;
        }

        atexit(&glfwTerminate);

        FileSystem::setResourcePath("./");
        Platform* platform = new Platform(game);

        // Get the window configuration values
        const char* title = nullptr;
        int __x = 0, __y = 0, __width = 1280, __height = 800, __samples = 0;
        bool fullscreen = false;
        if( game->getConfig() )
        {
            Properties* config = game->getConfig()->getNamespace("window", true);
            if( config )
            {
                // Read window title.
                title = config->getString("title");

                // Read window rect.
                int x = config->getInt("x");
                int y = config->getInt("y");
                int width = config->getInt("width");
                int height = config->getInt("height");
                int samples = config->getInt("samples");
                fullscreen = config->getBool("fullscreen");

                if( fullscreen && width == 0 && height == 0 )
                {
                    auto mode = glfwGetVideoMode(nullptr);
                    // Use the screen resolution if fullscreen is true but width and height were not set in the config
                    width = mode->width;
                    height = mode->height;
                }
                if( x != 0 )
                    __x = x;
                if( y != 0 )
                    __y = y;
                if( width != 0 )
                    __width = width;
                if( height != 0 )
                    __height = height;
                if( samples != 0 )
                    __samples = samples;
            }
        }

        glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
        glfwWindowHint(GLFW_DEPTH_BITS, 24);
        glfwWindowHint(GLFW_STENCIL_BITS, 8);
        glfwWindowHint(GLFW_SAMPLES, __samples);
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

        __multiSampling = __samples > 0;

        // Create the windows
        __window = glfwCreateWindow(__width, __height, "EdisonEngine", fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);
        glfwMakeContextCurrent(__window);

        // Use OpenGL 2.x with GLEW
        glewExperimental = GL_TRUE;
        const auto err = glewInit();
        if( err != GLEW_OK )
        {
            BOOST_LOG_TRIVIAL(error) << "glewInit: " << (const char*)glewGetErrorString(err);
            return nullptr;
        }

        return platform;
    }


    void Platform::boot()
    {
        BOOST_ASSERT(_game);

        // Get the initial time.
        __timeStart = std::chrono::high_resolution_clock::now();
        __timeAbsolute = std::chrono::high_resolution_clock::duration(0);

        // Run the game.
        _game->run();
    }

    bool Platform::loop()
    {
        glfwPollEvents();

        if(glfwWindowShouldClose(__window))
            return false;

        return true;
    }

    void Platform::frame()
    {
        if(_game)
        {
            // Game state will be uninitialized if game was closed through Game::exit()
            if(_game->getState() == Game::UNINITIALIZED)
                return;

            _game->frame();
        }

        glfwSwapBuffers(__window);
    }

    void Platform::enterMessagePump()
    {
        BOOST_ASSERT(_game);

        // Get the initial time.
        __timeStart = std::chrono::high_resolution_clock::now();
        __timeAbsolute = std::chrono::high_resolution_clock::duration(0);

        // Run the game.
        _game->run();

        // Message loop.
        while( !glfwWindowShouldClose(__window) )
        {
            if( _game )
            {
                // Game state will be uninitialized if game was closed through Game::exit()
                if( _game->getState() == Game::UNINITIALIZED )
                    break;

                _game->frame();
            }

            glfwSwapBuffers(__window);
        }

        _game->exit();
        glfwDestroyWindow(__window);
    }


    void Platform::signalShutdown()
    {
    }


    bool Platform::canExit()
    {
        return true;
    }


    unsigned int Platform::getDisplayWidth()
    {
        int w, h;
        glfwGetWindowSize(__window, &w, &h);
        return w;
    }


    unsigned int Platform::getDisplayHeight()
    {
        int w, h;
        glfwGetWindowSize(__window, &w, &h);
        return h;
    }


    std::chrono::microseconds Platform::getAbsoluteTime()
    {
        __timeAbsolute = std::chrono::high_resolution_clock::now() - __timeStart;

        return std::chrono::duration_cast<std::chrono::microseconds>(__timeAbsolute);
    }


    void Platform::setAbsoluteTime(const std::chrono::microseconds& time)
    {
        __timeAbsolute = time;
    }


    bool Platform::isVsync()
    {
        return __vsync;
    }


    void Platform::setVsync(bool enable)
    {
        __vsync = enable;
        glfwSwapInterval(enable ? 1 : 0);
    }


    void Platform::swapBuffers()
    {
        glfwSwapBuffers(__window);
    }


    void Platform::sleep(const std::chrono::microseconds& time)
    {
        std::this_thread::sleep_for(time);
    }


    void Platform::setMultiSampling(bool enabled)
    {
        if( enabled == __multiSampling )
        {
            return;
        }

        // TODO
        __multiSampling = enabled;
    }


    bool Platform::isMultiSampling()
    {
        return __multiSampling;
    }


    GLFWwindow* Platform::getWindow()
    {
        return __window;
    }
}
