#pragma once

#include "glfw.h"

#include <filesystem>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <vector>

namespace gl
{
class Window final
{
public:
  explicit Window(const std::vector<std::filesystem::path>& logoPaths,
                  const glm::ivec2& windowSize,
                  bool borderlessFullscreen);
  ~Window();

  void updateWindowSize();

  [[nodiscard]] bool windowShouldClose() const
  {
    glfwPollEvents();

    return glfwWindowShouldClose(m_window) == GLFW_TRUE;
  }

  void swapBuffers() const;

  [[nodiscard]] gsl::not_null<GLFWwindow*> getWindow() const
  {
    return gsl::not_null{m_window};
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

  [[nodiscard]] bool hasFocus() const;

private:
  GLFWwindow* m_window = nullptr;
  glm::ivec2 m_windowPos{0};
  glm::ivec2 m_windowSize{0};
  glm::ivec2 m_viewport{0};
  bool m_isFullscreen = false;
};
} // namespace gl
