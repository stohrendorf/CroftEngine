#include "window.h"

#include "cimgwrapper.h"
#include "glassert.h"

#include <boost/log/trivial.hpp>
#include <gsl-lite.hpp>

namespace gl
{
namespace
{
void glErrorCallback(const int err, const gsl::czstring msg)
{
  BOOST_LOG_TRIVIAL(error) << "glfw Error " << err << ": " << msg;
}
} // namespace

Window::Window(const bool fullscreen, const glm::ivec2& resolution)
{
  glfwSetErrorCallback(&glErrorCallback);

  if(glfwInit() != GLFW_TRUE)
  {
    BOOST_LOG_TRIVIAL(fatal) << "Failed to initialize GLFW";
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to initialize GLFW"));
  }

  atexit(&glfwTerminate);

  glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
  glfwWindowHint(GLFW_DEPTH_BITS, 24);
  // glfwWindowHint( GLFW_SAMPLES, m_multiSampling );
  glfwWindowHint(GLFW_RED_BITS, 8);
  glfwWindowHint(GLFW_GREEN_BITS, 8);
  glfwWindowHint(GLFW_BLUE_BITS, 8);
  glfwWindowHint(GLFW_ALPHA_BITS, 8);
  glfwWindowHint(GLFW_DECORATED, fullscreen ? GLFW_FALSE : GLFW_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
#ifndef NDEBUG
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

  m_window = glfwCreateWindow(
    resolution.x, resolution.y, "EdisonEngine", fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);
  if(m_window == nullptr)
  {
    const char* message;
    glfwGetError(&message);
    BOOST_LOG_TRIVIAL(fatal) << "Failed to create window: " << message;
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create window"));
  }

  CImgWrapper imgWrapper{std::filesystem::path{"logo.png"}};
  imgWrapper.interleave();
  GLFWimage img;
  img.width = imgWrapper.width();
  img.height = imgWrapper.height();
  img.pixels = const_cast<unsigned char*>(imgWrapper.data()); // NOLINT(cppcoreguidelines-pro-type-const-cast)
  glfwSetWindowIcon(m_window, 1, &img);

  glfwMakeContextCurrent(m_window);

  initializeGl();

  updateWindowSize();

#ifdef NDEBUG
  glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
#endif
}

void Window::setVsync(const bool enable)
{
  m_vsync = enable;
  glfwSwapInterval(enable ? 1 : 0);
}

bool Window::isVsync() const
{
  return m_vsync;
}

bool Window::updateWindowSize()
{
  glm::ivec2 tmpSize;
  glfwGetFramebufferSize(m_window, &tmpSize.x, &tmpSize.y);

  if(tmpSize == m_viewport)
    return false;

  m_viewport = tmpSize;
  setViewport(m_viewport);
  return true;
}

void Window::swapBuffers() const
{
  glfwSwapBuffers(m_window);
}

void Window::setViewport(const glm::ivec2& viewport)
{
  m_viewport = viewport;
  GL_ASSERT(::gl::api::viewport(0, 0, viewport.x, viewport.y));
}
} // namespace gl
