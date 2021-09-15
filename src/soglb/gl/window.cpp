#include "window.h"

#include "cimgwrapper.h"
#include "glassert.h"
#include "glew_init.h"
#include "renderstate.h"

#include <boost/log/trivial.hpp>
#include <gsl/gsl-lite.hpp>

namespace gl
{
namespace
{
void glErrorCallback(const int err, const gsl::czstring msg)
{
  BOOST_LOG_TRIVIAL(error) << "glfw Error " << err << ": " << msg;
}
} // namespace

Window::Window(const std::filesystem::path& logoPath, const glm::ivec2& resolution)
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
  glfwWindowHint(GLFW_DEPTH_BITS, 32);
  glfwWindowHint(GLFW_RED_BITS, 8);
  glfwWindowHint(GLFW_GREEN_BITS, 8);
  glfwWindowHint(GLFW_BLUE_BITS, 8);
  glfwWindowHint(GLFW_ALPHA_BITS, 8);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
#ifdef SOGLB_DEBUGGING
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#else
  glfwWindowHint(GLFW_CONTEXT_NO_ERROR, GLFW_TRUE);
#endif

  glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
  m_window = glfwCreateWindow(resolution.x, resolution.y, "EdisonEngine", nullptr, nullptr);

  if(m_window == nullptr)
  {
    const char* message;
    glfwGetError(&message);
    BOOST_LOG_TRIVIAL(fatal) << "Failed to create window: " << message;
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create window"));
  }

  CImgWrapper imgWrapper{std::filesystem::path{logoPath}};
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

void Window::updateWindowSize()
{
  glm::ivec2 tmpSize;
  glfwGetFramebufferSize(m_window, &tmpSize.x, &tmpSize.y);

  if(tmpSize == m_viewport)
    return;

  m_viewport = tmpSize;
  RenderState::getWantedState().setViewport(m_viewport);
}

void Window::swapBuffers() const
{
  glfwSwapBuffers(m_window);
}

void Window::setFullscreen()
{
  if(m_isFullscreen)
    return;

  const auto monitor = glfwGetPrimaryMonitor();
  Expects(monitor != nullptr);
  Expects(glfwGetError(nullptr) == GLFW_NO_ERROR);
  const auto mode = glfwGetVideoMode(monitor);
  Expects(mode != nullptr);
  Expects(glfwGetError(nullptr) == GLFW_NO_ERROR);

  glfwGetWindowPos(m_window, &m_windowPos.x, &m_windowPos.y);
  Expects(glfwGetError(nullptr) == GLFW_NO_ERROR);
  glfwGetWindowSize(m_window, &m_windowSize.x, &m_windowSize.y);
  Expects(glfwGetError(nullptr) == GLFW_NO_ERROR);

  glm::ivec2 pos{0, 0};
  glfwGetMonitorPos(monitor, &pos.x, &pos.y);
  Expects(glfwGetError(nullptr) == GLFW_NO_ERROR);

  glfwSetWindowAttrib(m_window, GLFW_DECORATED, GLFW_FALSE);
  Expects(glfwGetError(nullptr) == GLFW_NO_ERROR);
  glfwSetWindowAttrib(m_window, GLFW_RESIZABLE, GLFW_FALSE);
  Expects(glfwGetError(nullptr) == GLFW_NO_ERROR);
  glfwSetWindowPos(m_window, pos.x, pos.y);
  Expects(glfwGetError(nullptr) == GLFW_NO_ERROR);
  glfwSetWindowSize(m_window, mode->width, mode->height);
  Expects(glfwGetError(nullptr) == GLFW_NO_ERROR);
  m_isFullscreen = true;
}

void Window::setWindowed()
{
  if(!m_isFullscreen)
    return;

  glfwSetWindowAttrib(m_window, GLFW_DECORATED, GLFW_TRUE);
  Expects(glfwGetError(nullptr) == GLFW_NO_ERROR);
  glfwSetWindowAttrib(m_window, GLFW_RESIZABLE, GLFW_TRUE);
  Expects(glfwGetError(nullptr) == GLFW_NO_ERROR);
  glfwSetWindowPos(m_window, m_windowPos.x, m_windowPos.y);
  Expects(glfwGetError(nullptr) == GLFW_NO_ERROR);
  glfwSetWindowSize(m_window, m_windowSize.x, m_windowSize.y);
  Expects(glfwGetError(nullptr) == GLFW_NO_ERROR);

  m_isFullscreen = false;
}

Window::~Window()
{
  glfwDestroyWindow(m_window);
}
} // namespace gl
