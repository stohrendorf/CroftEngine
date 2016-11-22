#pragma once

#include "Rectangle.h"
#include "RenderState.h"

#include <GLFW/glfw3.h>


namespace gameplay
{
    class Scene;


    /**
     * Defines the base class your game will extend for game initialization, logic and platform delegates.
     *
     * This represents a running cross-platform game application and provides an abstraction
     * to most typical platform functionality and events.
     *
     * @see http://gameplay3d.github.io/GamePlay/docs/file-formats.html#wiki-Game_Config
     */
    class Game
    {
        friend class ShutdownListener;

    public:

        /**
         * The game states.
         */
        enum State
        {
            UNINITIALIZED,
            RUNNING,
            PAUSED
        };


        /**
         * Flags used when clearing the active frame buffer targets.
         */
        enum ClearFlags
        {
            CLEAR_COLOR = GL_COLOR_BUFFER_BIT,
            CLEAR_DEPTH = GL_DEPTH_BUFFER_BIT,
            CLEAR_STENCIL = GL_STENCIL_BUFFER_BIT,
            CLEAR_COLOR_DEPTH = CLEAR_COLOR | CLEAR_DEPTH,
            CLEAR_COLOR_STENCIL = CLEAR_COLOR | CLEAR_STENCIL,
            CLEAR_DEPTH_STENCIL = CLEAR_DEPTH | CLEAR_STENCIL,
            CLEAR_COLOR_DEPTH_STENCIL = CLEAR_COLOR | CLEAR_DEPTH | CLEAR_STENCIL
        };


        /**
         * Constructor.
         */
        explicit Game();

        /**
         * Destructor.
         */
        virtual ~Game();

        /**
         * Gets whether vertical sync is enabled for the game display.
         *
         * @return true if vsync is enabled; false if not.
         */
        bool isVsync() const;

        /**
         * Sets whether vertical sync is enabled for the game display.
         *
         * @param enable true if vsync is enabled; false if not.
         */
        void setVsync(bool enable);

        /**
         * Gets the total absolute running time (in milliseconds) since Game::run().
         *
         * @return The total absolute running time (in milliseconds).
         */
        std::chrono::microseconds getAbsoluteTime();

        /**
         * Gets the total game time (in milliseconds). This is the total accumulated game time (unpaused).
         *
         * You would typically use things in your game that you want to stop when the game is paused.
         * This includes things such as game physics and animation.
         *
         * @return The total game time (in milliseconds).
         */
        std::chrono::microseconds getGameTime();

        /**
         * Gets the game state.
         *
         * @return The current game state.
         */
        inline State getState() const;

        /**
         * Determines if the game has been initialized.
         *
         * @return true if the game initialization has completed, false otherwise.
         */
        inline bool isInitialized() const;

        /**
         * Called to initialize the game, and begin running the game.
         *
         * @return Zero for normal termination, or non-zero if an error occurred.
         */
        int run();

        /**
         * Pauses the game after being run.
         */
        void pause();

        /**
         * Resumes the game after being paused.
         */
        void resume();

        /**
         * Exits the game.
         */
        void exit();

        /**
         * Platform frame delegate.
         *
         * This is called every frame from the platform.
         * This in turn calls back on the user implemented game methods: update() then render()
         */
        void frame();

        /**
         * Gets the current frame rate.
         *
         * @return The current frame rate.
         */
        inline unsigned int getFrameRate() const;

        /**
         * Gets the game window width.
         *
         * @return The game window width.
         */
        inline unsigned int getWidth() const;

        /**
         * Gets the game window height.
         *
         * @return The game window height.
         */
        inline unsigned int getHeight() const;

        /**
         * Gets the aspect ratio of the window. (width / height)
         *
         * @return The aspect ratio of the window.
         */
        inline float getAspectRatio() const;

        /**
         * Gets the game current viewport.
         *
         * The default viewport is Rectangle(0, 0, Game::getWidth(), Game::getHeight()).
         */
        inline const Rectangle& getViewport() const;

        /**
         * Sets the game current viewport.
         *
         * The x, y, width and height of the viewport must all be positive.
         *
         * viewport The custom viewport to be set on the game.
         */
        void setViewport(const Rectangle& viewport);

        /**
         * Clears the specified resource buffers to the specified clear values.
         *
         * @param flags The flags indicating which buffers to be cleared.
         * @param clearColor The color value to clear to when the flags includes the color buffer.
         * @param clearDepth The depth value to clear to when the flags includes the color buffer.
         * @param clearStencil The stencil value to clear to when the flags includes the color buffer.
         */
        void clear(ClearFlags flags, const glm::vec4& clearColor, float clearDepth, int clearStencil);

        /**
         * Clears the specified resource buffers to the specified clear values.
         *
         * @param flags The flags indicating which buffers to be cleared.
         * @param red The red channel.
         * @param green The green channel.
         * @param blue The blue channel.
         * @param alpha The alpha channel.
         * @param clearDepth The depth value to clear to when the flags includes the color buffer.
         * @param clearStencil The stencil value to clear to when the flags includes the color buffer.
         */
        void clear(ClearFlags flags, float red, float green, float blue, float alpha, float clearDepth, int clearStencil);

        /**
         * Sets whether multi-sampling is to be enabled/disabled. Default is disabled.
         *
         * @param enabled true sets multi-sampling to be enabled, false to be disabled.
         */
        inline void setMultiSampling(bool enabled);

        /**
         * Is multi-sampling enabled.
         *
         * @return true if multi-sampling is enabled.
         */
        inline bool isMultiSampling() const;

        bool loop()
        {
            glfwPollEvents();

            return glfwWindowShouldClose(_window) == GLFW_FALSE;
        }


        /**
        * Renders a single frame once and then swaps it to the display.
        * This calls the given script function, which should take no parameters and return nothing (void).
        *
        * This is useful for rendering splash screens.
        */
        void swapBuffers();


        GLFWwindow* getWindow() const
        {
            return _window;
        }


        const std::shared_ptr<Scene>& getScene() const
        {
            return _scene;
        }


    protected:

        /**
         * Render callback for handling rendering routines.
         *
         * Called just after update, once per frame when game is running.
         * Ideal for all rendering code.
         */
        virtual void render();

        /**
         * Renders a single frame once and then swaps it to the display.
         *
         * This is useful for rendering splash screens.
         */
        template<class T>
        void swapBuffers(T* instance, void (T::*method)(void*), void* cookie);

    private:

        /**
         * Constructor.
         *
         * @param copy The game to copy.
         */
        Game(const Game& copy);

        /**
         * Starts the game.
         */
        bool startup();

        /**
         * Shuts down the game.
         */
        void shutdown();

        bool _initialized; // If game has initialized yet.
        State _state; // The game state.
        unsigned int _pausedCount; // Number of times pause() has been called.
        static std::chrono::microseconds _pausedTimeLast; // The last time paused.
        static std::chrono::microseconds _pausedTimeTotal; // The total time paused.
        std::chrono::microseconds _frameLastFPS; // The last time the frame count was updated.
        unsigned int _frameCount; // The current frame count.
        unsigned int _frameRate; // The current frame rate.
        int _width; // The game's display width.
        int _height; // The game's display height.
        Rectangle _viewport; // the games's current viewport.
        glm::vec4 _clearColor; // The clear color value last used for clearing the color buffer.
        float _clearDepth; // The clear depth value last used for clearing the depth buffer.
        int _clearStencil; // The clear stencil value last used for clearing the stencil buffer.

        std::chrono::high_resolution_clock::time_point _timeStart;
        std::chrono::high_resolution_clock::duration _timeAbsolute;
        bool _vsync = WINDOW_VSYNC;
        bool _multiSampling = false;
        GLFWwindow* _window = nullptr;

        std::shared_ptr<Scene> _scene;

        friend class ScreenDisplayer;

        bool drawScene(const std::shared_ptr<Node>& node);
    };


    inline Game::State Game::getState() const
    {
        return _state;
    }


    inline bool Game::isInitialized() const
    {
        return _initialized;
    }


    inline unsigned int Game::getFrameRate() const
    {
        return _frameRate;
    }


    inline unsigned int Game::getWidth() const
    {
        return _width;
    }


    inline unsigned int Game::getHeight() const
    {
        return _height;
    }


    inline float Game::getAspectRatio() const
    {
        return static_cast<float>(_width) / static_cast<float>(_height);
    }


    inline const Rectangle& Game::getViewport() const
    {
        return _viewport;
    }


    template<class T>
    void Game::swapBuffers(T* instance, void (T::*method)(void*), void* cookie)
    {
        GP_ASSERT(instance);
        (instance ->* method)(cookie);
        swapBuffers();
    }


    inline void Game::setMultiSampling(bool enabled)
    {
        if( enabled == _multiSampling )
        {
            return;
        }

        //! @todo Really enable multisampling
        _multiSampling = enabled;
    }


    inline bool Game::isMultiSampling() const
    {
        return _multiSampling;
    }
}
