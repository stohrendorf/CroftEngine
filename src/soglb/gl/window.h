#pragma once

#include "glfw.h"

#include <glm/glm.hpp>

namespace gl
{
class Window final
{
public:
  explicit Window(const glm::ivec2& resolution = {1280, 800});
  ~Window();

  [[nodiscard]] bool isVsync() const;

  void setVsync(bool enable);

  void updateWindowSize();

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

  [[nodiscard]] const glm::ivec2& getViewport() const
  {
    return m_viewport;
  }

  void setFullscreen();
  void setWindowed();

  void setFullscreen(bool value)
  {
    if(value)
      setFullscreen();
    else
      setWindowed();
  }

  [[nodiscard]] bool isMinimized() const noexcept
  {
    return m_viewport.x <= 0 || m_viewport.y <= 0;
  }

private:
  GLFWwindow* m_window = nullptr;
  bool m_vsync = false;
  glm::ivec2 m_windowPos{0};
  glm::ivec2 m_windowSize{0};
  glm::ivec2 m_viewport{0};
  bool m_isFullscreen = false;
};
} // namespace gl
