#pragma once

#include "Dimension.h"

#include "render/gl/renderstate.h"
#include "render/gl/pixel.h"

#include <chrono>

namespace render
{
namespace scene
{
class Scene;


class RenderContext;


class Game final
{
public:
    Game(const Game&) = delete;

    Game(Game&&) = delete;

    Game& operator=(const Game&) = delete;

    Game& operator=(Game&&) = delete;

    explicit Game();

    virtual ~Game();

    std::chrono::high_resolution_clock::time_point getGameTime() const
    {
        return std::chrono::high_resolution_clock::now() - m_constructionTime.time_since_epoch();
    }

    void render();

    float getFrameRate() const
    {
        return m_frameRate;
    }

    void clear(GLbitfield flags, const gl::RGBA8& clearColor, float clearDepth);

    void clear(GLbitfield flags, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha, float clearDepth);

    const std::shared_ptr<Scene>& getScene() const
    {
        return m_scene;
    }

private:
    const std::chrono::high_resolution_clock::time_point m_constructionTime{std::chrono::high_resolution_clock::now()};

    std::chrono::high_resolution_clock::time_point m_frameLastFPS{}; // The last time the frame count was updated.
    uint_fast32_t m_frameCount = 0; // The current frame count.
    float m_frameRate = 0; // The current frame rate.
    gl::RGBA8 m_clearColor; // The clear color value last used for clearing the color buffer.
    float m_clearDepth = 1; // The clear depth value last used for clearing the depth buffer.

    std::shared_ptr<Scene> m_scene;
};
}
}
