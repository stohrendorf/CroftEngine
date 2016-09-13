#pragma once

#include "FileSystem.h"
#include <GLFW/glfw3.h>


namespace gameplay
{
    class Game;


    /**
     * Defines a platform abstraction.
     *
     * This class has only a few public methods for creating a platform
     */
    class Platform
    {
    public:

        friend class Game;
        friend class ScreenDisplayer;
        friend class FileSystem;

        /**
         * Destructor.
         */
        ~Platform();

        /**
         * Creates a platform for the specified game which it will interact with.
         *
         * @param game The game to create a platform for.
         *
         * @return The created platform interface.
         * @script{ignore}
         */
        static Platform* create(Game* game);

        /**
         * Begins processing the platform messages.
         *
         * This method handles all OS window messages and drives the game loop.
         * It normally does not return until the application is closed.
         *
         * If a attachToWindow is passed to Platform::create the message pump will instead attach
         * to or allow the attachToWindow to drive the game loop on the platform.
         *
         * @return The platform message pump return code.
         */
        void enterMessagePump();

        /**
         * Swaps the frame buffer on the device.
         */
        static void swapBuffers();

        static GLFWwindow* getWindow();
        void boot();
        bool loop();
        void frame();

    private:

        /**
         * This method informs the platform that the game is shutting down
         * and anything platform specific should be shutdown as well or halted
         * This function is called automatically when the game shutdown function is called
         */
        static void signalShutdown();

        /**
         * Indicates whether a programmatic exit is allowed on this platform.
         * Some platforms (eg. iOS) do not allow apps to exit programmatically.
         *
         * @return whether a programmatic exit is allowed on this platform.
         */
        static bool canExit();

        /**
         * Gets the display width.
         *
         * @return The display width.
         */
        static unsigned int getDisplayWidth();

        /**
         * Gets the display height.
         *
         * @return The display height.
         */
        static unsigned int getDisplayHeight();

        /**
         * Gets the absolute platform time starting from when the message pump was started.
         *
         * @return The absolute platform time. (in milliseconds)
         */
        static std::chrono::microseconds getAbsoluteTime();

        /**
         * Sets the absolute platform time since the start of the message pump.
         *
         * @param time The time to set (in milliseconds).
         */
        static void setAbsoluteTime(const std::chrono::microseconds& time);

        /**
         * Gets whether vertical sync is enabled for the game display.
         *
         * @return true if vsync is enabled; false if not.
         */
        static bool isVsync();

        /**
         * Sets whether vertical sync is enable for the game display.
         *
         * @param enable true if vsync is enabled; false if not.
         */
        static void setVsync(bool enable);

        /**
         * Sleeps synchronously for the given amount of time (in milliseconds).
         *
         * @param time How long to sleep (in milliseconds).
         */
        static void sleep(const std::chrono::microseconds& time);

        /**
         * Set if multi-sampling is enabled on the platform.
         *
         * @param enabled true sets multi-sampling to be enabled, false to be disabled.
         */
        static void setMultiSampling(bool enabled);

        /**
         * Is multi-sampling mode enabled.
         */
        static bool isMultiSampling();

        /**
         * Constructor.
         */
        Platform(Game* game);

        /**
         * Constructor.
         */
        Platform(const Platform& copy);

    public:
        /**
         * Internal method used only from static code in various platform implementation.
         *
         * @script{ignore}
         */
        static void shutdownInternal();

    private:

        Game* _game; // The game this platform is interfacing with.
    };
}


#include "Game.h"
