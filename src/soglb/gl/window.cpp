#include "window.h"

#include "cimgwrapper.h"
#include "glad_init.h"
#include "glassert.h"
#include "glfw.h"

#include <algorithm>
#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <cstdlib>
#include <glm/fwd.hpp>
#include <gsl/gsl-lite.hpp>
#include <iterator>
#include <map>
#include <mutex>
#include <stdexcept>
#include <unordered_map>

namespace gl
{
namespace
{
std::mutex focusStatesMutex;
auto& getFocusStates()
{
  static std::unordered_map<GLFWwindow*, bool> focusStates;
  return focusStates;
}

void glErrorCallback(const int err, const gsl::czstring msg)
{
  BOOST_LOG_TRIVIAL(error) << "glfw Error " << err << ": " << msg;
}

void windowFocusCallback(GLFWwindow* window, int focused)
{
  const std::lock_guard guard{focusStatesMutex};
  getFocusStates()[window] = focused == GLFW_TRUE;
}

void initGlfw()
{
  glfwSetErrorCallback(&glErrorCallback);

  if(glfwInit() != GLFW_TRUE)
  {
    BOOST_LOG_TRIVIAL(fatal) << "Failed to initialize GLFW";
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to initialize GLFW"));
  }
}

void terminateGlfw()
{
  glfwSetErrorCallback(nullptr);
  glfwTerminate();
}
} // namespace

Window::Window(const std::vector<std::filesystem::path>& logoPaths,
               const glm::ivec2& windowSize,
               bool borderlessFullscreen)
    : m_windowPos{0, 0}
    , m_windowSize{windowSize}
{
  initGlfw();

  glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
  glfwWindowHint(GLFW_DEPTH_BITS, 32);
  glfwWindowHint(GLFW_RED_BITS, 8);
  glfwWindowHint(GLFW_GREEN_BITS, 8);
  glfwWindowHint(GLFW_BLUE_BITS, 8);
  glfwWindowHint(GLFW_ALPHA_BITS, 8);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  if(borderlessFullscreen)
  {
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_CURSOR_DISABLED, GLFW_FALSE);
  }

#ifdef SOGLB_DEBUGGING
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#else
  glfwWindowHint(GLFW_CONTEXT_NO_ERROR, GLFW_TRUE);
#endif

  glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
  m_window = glfwCreateWindow(windowSize.x, windowSize.y, "CroftEngine", nullptr, nullptr);

  if(m_window == nullptr)
  {
    const char* message;
    glfwGetError(&message);
    BOOST_LOG_TRIVIAL(fatal) << "Failed to create window: " << message;
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create window"));
  }

  glfwSetWindowFocusCallback(m_window, windowFocusCallback);
  windowFocusCallback(m_window, GLFW_TRUE);

  {
    std::vector<CImgWrapper> imgWrappers;
    std::transform(logoPaths.begin(),
                   logoPaths.end(),
                   std::back_inserter(imgWrappers),
                   [](const std::filesystem::path& p)
                   {
                     auto tmp = CImgWrapper{p};
                     tmp.interleave();
                     return tmp;
                   });
    std::vector<GLFWimage> glfwImages;
    std::transform(imgWrappers.begin(),
                   imgWrappers.end(),
                   std::back_inserter(glfwImages),
                   [](const CImgWrapper& srcImg)
                   {
                     GLFWimage img;
                     img.width = srcImg.width();
                     img.height = srcImg.height();
                     img.pixels
                       = const_cast<unsigned char*>(srcImg.data()); // NOLINT(cppcoreguidelines-pro-type-const-cast)
                     return img;
                   });
    glfwSetWindowIcon(m_window, gsl::narrow<int>(glfwImages.size()), glfwImages.data());
  }

  glfwMakeContextCurrent(m_window);

  initializeGl(
    [](const char* procName) -> void*
    {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      return reinterpret_cast<void*>(glfwGetProcAddress(procName));
    });

  updateWindowSize();

#ifdef NDEBUG
  if(!borderlessFullscreen)
  {
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }
#endif
  glfwSwapInterval(1);
}

void Window::updateWindowSize()
{
  glm::ivec2 tmpSize;
  glfwGetFramebufferSize(m_window, &tmpSize.x, &tmpSize.y);

  if(tmpSize == m_viewport)
    return;

  m_viewport = tmpSize;
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
  gsl_Assert(monitor != nullptr);
  gsl_Assert(glfwGetError(nullptr) == GLFW_NO_ERROR);
  const auto mode = glfwGetVideoMode(monitor);
  gsl_Assert(mode != nullptr);
  gsl_Assert(glfwGetError(nullptr) == GLFW_NO_ERROR);

  glfwGetWindowPos(m_window, &m_windowPos.x, &m_windowPos.y);
  gsl_Assert(glfwGetError(nullptr) == GLFW_NO_ERROR);
  glfwGetWindowSize(m_window, &m_windowSize.x, &m_windowSize.y);
  gsl_Assert(glfwGetError(nullptr) == GLFW_NO_ERROR);

  glm::ivec2 pos{0, 0};
  glfwGetMonitorPos(monitor, &pos.x, &pos.y);
  gsl_Assert(glfwGetError(nullptr) == GLFW_NO_ERROR);

#ifdef WIN32
  glfwSetWindowAttrib(m_window, GLFW_DECORATED, GLFW_FALSE);
  gsl_Assert(glfwGetError(nullptr) == GLFW_NO_ERROR);
  glfwSetWindowAttrib(m_window, GLFW_RESIZABLE, GLFW_FALSE);
  gsl_Assert(glfwGetError(nullptr) == GLFW_NO_ERROR);
  glfwSetWindowPos(m_window, pos.x, pos.y);
  gsl_Assert(glfwGetError(nullptr) == GLFW_NO_ERROR);
  glfwSetWindowSize(m_window, mode->width, mode->height);
  gsl_Assert(glfwGetError(nullptr) == GLFW_NO_ERROR);
#else
  glfwSetWindowMonitor(m_window, monitor, pos.x, pos.y, mode->width, mode->height, 30);
  gsl_Assert(glfwGetError(nullptr) == GLFW_NO_ERROR);
#endif
  m_isFullscreen = true;
}

void Window::setWindowed()
{
  if(!m_isFullscreen)
    return;

  glfwSetWindowAttrib(m_window, GLFW_DECORATED, GLFW_TRUE);
  gsl_Assert(glfwGetError(nullptr) == GLFW_NO_ERROR);
  glfwSetWindowAttrib(m_window, GLFW_RESIZABLE, GLFW_TRUE);
  gsl_Assert(glfwGetError(nullptr) == GLFW_NO_ERROR);
  glfwSetWindowPos(m_window, m_windowPos.x, m_windowPos.y);
  gsl_Assert(glfwGetError(nullptr) == GLFW_NO_ERROR);
  glfwSetWindowSize(m_window, m_windowSize.x, m_windowSize.y);
  gsl_Assert(glfwGetError(nullptr) == GLFW_NO_ERROR);

  m_isFullscreen = false;
}

Window::~Window()
{
  glfwSetWindowFocusCallback(m_window, nullptr);
  glfwDestroyWindow(m_window);

  terminateGlfw();

  const std::lock_guard guard{focusStatesMutex};
  getFocusStates().erase(m_window);
}

bool Window::hasFocus() const
{
  const std::lock_guard guard{focusStatesMutex};
  auto it = getFocusStates().find(m_window);
  return it != getFocusStates().end() && it->second;
}
} // namespace gl
