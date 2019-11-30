#pragma once

#include "dimension.h"
#include "glfw.h"

namespace render::scene
{
class Window final
{
public:
  explicit Window(bool fullscreen = false, const Dimension2<int>& resolution = {1280, 800});

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

  void setViewport(const Dimension2<size_t>& viewport);

  [[nodiscard]] float getAspectRatio() const
  {
    return static_cast<float>(m_viewport.width) / m_viewport.height;
  }

  [[nodiscard]] const Dimension2<size_t>& getViewport() const
  {
    return m_viewport;
  }

private:
  GLFWwindow* m_window = nullptr;
  bool m_vsync = false;
  Dimension2<size_t> m_viewport; // the games's current viewport.
};
} // namespace render::scene
