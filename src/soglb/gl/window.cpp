#include "window.h"

#include "cimgwrapper.h"
#include "glassert.h"
#include "glew_init.h"

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
    : m_windowPos{0, 0}
    , m_windowSize{resolution}
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
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
#ifndef NDEBUG
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

  m_window = glfwCreateWindow(resolution.x, resolution.y, "EdisonEngine", nullptr, nullptr);

  if(m_window == nullptr)
  {
    const char* message;
    glfwGetError(&message);
    BOOST_LOG_TRIVIAL(fatal) << "Failed to create window: " << message;
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create window"));
  }

  if(fullscreen)
    setFullscreen();

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
  GL_ASSERT(::gl::api::viewport(0, 0, m_viewport.x, m_viewport.y));
  return true;
}

void Window::swapBuffers() const
{
  glfwSwapBuffers(m_window);
}

void Window::setFullscreen()
{
  if(m_isFullscreen)
    return;

  const auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
  Expects(mode != nullptr);
  Expects(glfwGetError(nullptr) == GLFW_NO_ERROR);

  glfwGetWindowPos(m_window, &m_windowPos.x, &m_windowPos.y);
  Expects(glfwGetError(nullptr) == GLFW_NO_ERROR);
  glfwGetWindowSize(m_window, &m_windowSize.x, &m_windowSize.y);
  Expects(glfwGetError(nullptr) == GLFW_NO_ERROR);

  glfwSetWindowMonitor(m_window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
  Expects(glfwGetError(nullptr) == GLFW_NO_ERROR);
  m_isFullscreen = true;
}

void Window::setWindowed()
{
  if(!m_isFullscreen)
    return;

  glfwSetWindowMonitor(m_window, nullptr, m_windowPos.x, m_windowPos.y, m_windowSize.x, m_windowSize.y, GLFW_DONT_CARE);
  Expects(glfwGetError(nullptr) == GLFW_NO_ERROR);
  m_isFullscreen = false;
}
} // namespace gl
