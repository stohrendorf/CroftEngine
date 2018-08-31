#pragma once

#include "Rectangle.h"
#include "RenderState.h"

#include "gl/pixel.h"

#include <GLFW/glfw3.h>

#include <chrono>

namespace gameplay
{
class Scene;


class RenderContext;


class Game
{
public:
    Game(const Game&) = delete;

    Game(Game&&) = delete;

    explicit Game();

    virtual ~Game();

    bool isVsync() const;

    void setVsync(bool enable);

    std::chrono::high_resolution_clock::time_point getGameTime() const
    {
        return std::chrono::high_resolution_clock::now() - m_constructionTime.time_since_epoch();
    }

    void initialize();

    bool updateWindowSize();

    void render();

    unsigned int getFrameRate() const
    {
        return m_frameRate;
    }

    float getAspectRatio() const
    {
        return m_viewport.x / m_viewport.y;
    }

    const Point& getViewport() const
    {
        return m_viewport;
    }

    void setViewport(const Point& viewport);

    void clear(GLbitfield flags, const gl::RGBA8& clearColor, float clearDepth);

    void clear(GLbitfield flags, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha, float clearDepth);

    bool windowShouldClose()
    {
        glfwPollEvents();

        return glfwWindowShouldClose( m_window ) == GL_TRUE;
    }

    void swapBuffers();

    GLFWwindow* getWindow() const
    {
        return m_window;
    }

    const std::shared_ptr<Scene>& getScene() const
    {
        return m_scene;
    }

private:
    bool m_initialized = false; // If game has initialized yet.
    const std::chrono::high_resolution_clock::time_point m_constructionTime{
            std::chrono::high_resolution_clock::now()};

    std::chrono::high_resolution_clock::time_point m_frameLastFPS{}; // The last time the frame count was updated.
    unsigned int m_frameCount = 0; // The current frame count.
    unsigned int m_frameRate = 0; // The current frame rate.
    Point m_viewport; // the games's current viewport.
    gl::RGBA8 m_clearColor; // The clear color value last used for clearing the color buffer.
    float m_clearDepth = 1; // The clear depth value last used for clearing the depth buffer.

    bool m_vsync = WINDOW_VSYNC;

    GLFWwindow* m_window = nullptr;

    std::shared_ptr<Scene> m_scene;
};
}
