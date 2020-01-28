#pragma once

#include "glfw.h"

#include <glm/glm.hpp>

namespace gl
{
class Window final
{
public:
  explicit Window(bool fullscreen = false, const glm::ivec2& resolution = {1280, 800});

  [[nodiscard]] bool isVsync() const;

  void setVsync(bool enable);

  bool updateWindowSize();

  [[nodiscard]] bool windowShouldClose() const
  {
    glfwPollEvents();

    return glfwWindowShouldClose(m_window) == GLFW_TRUE;
  }

  void swapBuffers() const;

  [[nodiscard]] GLFWwindow* getWindow() const
  {
    return m_window;
  }

  void setViewport(const glm::ivec2& viewport);

  [[nodiscard]] float getAspectRatio() const
  {
    return static_cast<float>(m_viewport.x) / m_viewport.y;
  }

  [[nodiscard]] const glm::ivec2& getViewport() const
  {
    return m_viewport;
  }

private:
  GLFWwindow* m_window = nullptr;
  bool m_vsync = false;
  glm::ivec2 m_viewport{0};
};
} // namespace render::scene
