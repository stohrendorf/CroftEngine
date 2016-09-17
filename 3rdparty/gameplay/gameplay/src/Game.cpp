#include "Base.h"
#include "Game.h"
#include "Platform.h"
#include "RenderState.h"
#include "FileSystem.h"
#include "FrameBuffer.h"
#include "Scene.h"

/** @script{ignore} */
GLenum __gl_error_code = GL_NO_ERROR;


namespace gameplay
{
    static Game* __gameInstance = nullptr;
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
        , _properties(nullptr)
        , _animationController(nullptr)
        , _aiController(nullptr)
        , _timeEvents(nullptr)
    {
        GP_ASSERT(__gameInstance == nullptr);

        __gameInstance = this;
        _timeEvents = new std::priority_queue<TimeEvent, std::vector<TimeEvent>, std::less<TimeEvent>>();
    }


    Game::~Game()
    {
        // Do not call any virtual functions from the destructor.
        // Finalization is done from outside this class.
        SAFE_DELETE(_timeEvents);
#ifdef GP_USE_MEM_LEAK_DETECTION
    Ref::printLeaks();
    printMemoryLeaks();
#endif

        __gameInstance = nullptr;
    }


    Game* Game::getInstance()
    {
        GP_ASSERT(__gameInstance);
        return __gameInstance;
    }


    void Game::initialize()
    {
        // stub
    }


    void Game::finalize()
    {
        // stub
    }


    void Game::update(const std::chrono::microseconds& /*elapsedTime*/)
    {
        // stub
    }


    bool Game::drawScene(const std::shared_ptr<Node>& node)
    {

        auto dr = node->getDrawable();
        if(dr != nullptr)
        {
            dr->draw();
        }

        return true;
    }

    void Game::render(const std::chrono::microseconds& /*elapsedTime*/)
    {
        clear(CLEAR_COLOR_DEPTH, Vector4::zero(), 1, 0);
        Scene::getScene()->visit(this, &Game::drawScene);
    }


    std::chrono::microseconds Game::getAbsoluteTime()
    {
        return Platform::getAbsoluteTime();
    }


    std::chrono::microseconds Game::getGameTime()
    {
        return Platform::getAbsoluteTime() - _pausedTimeTotal;
    }


    void Game::setVsync(bool enable)
    {
        Platform::setVsync(enable);
    }


    bool Game::isVsync()
    {
        return Platform::isVsync();
    }


    int Game::run()
    {
        if( _state != UNINITIALIZED )
            return -1;

        loadConfig();

        _width = Platform::getDisplayWidth();
        _height = Platform::getDisplayHeight();

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

        _animationController = new AnimationController();
        _animationController->initialize();

        _aiController = new AIController();
        _aiController->initialize();

        _state = RUNNING;

        return true;
    }


    void Game::shutdown()
    {
        // Call user finalization.
        if( _state != UNINITIALIZED )
        {
            GP_ASSERT(_animationController);
            GP_ASSERT(_aiController);

            Platform::signalShutdown();

            // Call user finalize
            finalize();

            _animationController->finalize();
            SAFE_DELETE(_animationController);

            _aiController->finalize();
            SAFE_DELETE(_aiController);

            FrameBuffer::finalize();
            RenderState::finalize();

            SAFE_DELETE(_properties);

            _state = UNINITIALIZED;
        }
    }


    void Game::pause()
    {
        if( _state == RUNNING )
        {
            GP_ASSERT(_animationController);
            GP_ASSERT(_aiController);
            _state = PAUSED;
            _pausedTimeLast = Platform::getAbsoluteTime();
            _animationController->pause();
            _aiController->pause();
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
                GP_ASSERT(_animationController);
                GP_ASSERT(_aiController);
                _state = RUNNING;
                _pausedTimeTotal += Platform::getAbsoluteTime() - _pausedTimeLast;
                _animationController->resume();
                _aiController->resume();
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

#ifdef GP_USE_MEM_LEAK_DETECTION

        // Schedule a call to shutdown rather than calling it right away.
        // This handles the case of shutting down the script system from
        // within a script function (which can cause errors).
        static ShutdownListener listener;
        schedule(0, &listener);

#else

        // End the process immediately without a full shutdown
        ::exit(0);

#endif
    }


    void Game::frame()
    {
        if( !_initialized )
        {
            // Perform lazy first time initialization
            initialize();
            _initialized = true;
        }

        static std::chrono::microseconds lastFrameTime = Game::getGameTime();
        std::chrono::microseconds frameTime = getGameTime();

        // Fire time events to scheduled TimeListeners
        fireTimeEvents(frameTime);

        if( _state == Game::RUNNING )
        {
            GP_ASSERT(_animationController);
            GP_ASSERT(_aiController);

            // Update Time.
            std::chrono::microseconds elapsedTime = (frameTime - lastFrameTime);
            lastFrameTime = frameTime;

            // Update the scheduled and running animations.
            _animationController->update(elapsedTime);

            // Update AI.
            _aiController->update(elapsedTime);

            // Application Update.
            update(elapsedTime);

            // Graphics Rendering.
            render(elapsedTime);

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
            // Application Update.
            update(std::chrono::microseconds::zero());

            // Graphics Rendering.
            render(std::chrono::microseconds::zero());
        }
    }


    void Game::renderOnce()
    {
        Platform::swapBuffers();
    }


    void Game::updateOnce()
    {
        GP_ASSERT(_animationController);
        GP_ASSERT(_aiController);

        // Update Time.
        static std::chrono::microseconds lastFrameTime = getGameTime();
        std::chrono::microseconds frameTime = getGameTime();
        std::chrono::microseconds elapsedTime = (frameTime - lastFrameTime);
        lastFrameTime = frameTime;

        // Update the internal controllers.
        _animationController->update(elapsedTime);
        _aiController->update(elapsedTime);
    }


    void Game::setViewport(const Rectangle& viewport)
    {
        _viewport = viewport;
        glViewport(static_cast<GLuint>(viewport.x), static_cast<GLuint>(viewport.y), static_cast<GLuint>(viewport.width), static_cast<GLuint>(viewport.height));
    }


    void Game::clear(ClearFlags flags, const Vector4& clearColor, float clearDepth, int clearStencil)
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
                _clearColor.set(clearColor);
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
        clear(flags, Vector4(red, green, blue, alpha), clearDepth, clearStencil);
    }


    void Game::resizeEvent(unsigned int /*width*/, unsigned int /*height*/)
    {
        // stub
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    void Game::schedule(const std::chrono::microseconds& timeOffset, TimeListener* timeListener, void* cookie)
    {
        GP_ASSERT(_timeEvents);
        TimeEvent timeEvent(getGameTime() + timeOffset, timeListener, cookie);
        _timeEvents->push(timeEvent);
    }


    void Game::clearSchedule()
    {
        SAFE_DELETE(_timeEvents);
        _timeEvents = new std::priority_queue<TimeEvent, std::vector<TimeEvent>, std::less<TimeEvent>>();
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    void Game::fireTimeEvents(const std::chrono::microseconds& frameTime)
    {
        while( _timeEvents->size() > 0 )
        {
            const TimeEvent* timeEvent = &_timeEvents->top();
            if( timeEvent->time > frameTime )
            {
                break;
            }
            if( timeEvent->listener )
            {
                timeEvent->listener->timeEvent(frameTime - timeEvent->time, timeEvent->cookie);
            }
            _timeEvents->pop();
        }
    }


    Game::TimeEvent::TimeEvent(const std::chrono::microseconds& time, TimeListener* timeListener, void* cookie)
        : time(time)
        , listener(timeListener)
        , cookie(cookie)
    {
    }


    bool Game::TimeEvent::operator<(const TimeEvent& v) const
    {
        // The first element of std::priority_queue is the greatest.
        return time > v.time;
    }


    Properties* Game::getConfig() const
    {
        if( _properties == nullptr )
            const_cast<Game*>(this)->loadConfig();

        return _properties;
    }


    void Game::loadConfig()
    {
        if( _properties == nullptr )
        {
            // Try to load custom config from file.
            if( FileSystem::fileExists("game.config") )
            {
                _properties = Properties::create("game.config");

                // Load filesystem aliases.
                Properties* aliases = _properties->getNamespace("aliases", true);
                if( aliases )
                {
                    FileSystem::loadResourceAliases(aliases);
                }
            }
            else
            {
                // Create an empty config
                _properties = new Properties();
            }
        }
    }


    void Game::ShutdownListener::timeEvent(const std::chrono::microseconds& /*timeDiff*/, void* /*cookie*/)
    {
        Game::getInstance()->shutdown();
    }
}
