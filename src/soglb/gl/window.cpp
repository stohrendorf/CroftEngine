#include "window.h"

#include "cimgwrapper.h"
#include "glad_init.h"
#include "glassert.h"

#include <algorithm>
#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <cmath>
#include <cstdlib>
#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <gsl/gsl-lite.hpp>
#include <iterator>
#include <stdexcept>

namespace gl
{
namespace
{
void glErrorCallback(const int err, const gsl::czstring msg)
{
  BOOST_LOG_TRIVIAL(error) << "glfw Error " << err << ": " << msg;
}
} // namespace

Window::Window(int targetFramerate, const std::vector<std::filesystem::path>& logoPaths, const glm::ivec2& windowSize)
    : m_windowPos{0, 0}
    , m_windowSize{windowSize}
{
  glfwSetErrorCallback(&glErrorCallback);

  if(glfwInit() != GLFW_TRUE)
  {
    BOOST_LOG_TRIVIAL(fatal) << "Failed to initialize GLFW";
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to initialize GLFW"));
  }

  atexit(&glfwTerminate);

  glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
  glfwWindowHint(GLFW_REFRESH_RATE, targetFramerate);
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
  m_window = glfwCreateWindow(windowSize.x, windowSize.y, "EdisonEngine", nullptr, nullptr);

  if(m_window == nullptr)
  {
    const char* message;
    glfwGetError(&message);
    BOOST_LOG_TRIVIAL(fatal) << "Failed to create window: " << message;
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create window"));
  }

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
  glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
#endif

  {
    glfwSwapInterval(1);
    glfwSwapBuffers(m_window);

    std::vector<std::chrono::system_clock::time_point> samples;
    static constexpr size_t NSamples = 20;
    samples.reserve(NSamples + 1);
    for(int i = 0; i <= NSamples; ++i)
    {
      glfwSwapBuffers(m_window);
      samples.emplace_back(std::chrono::system_clock::now());
    }

    std::vector<long> swapIntervals;
    swapIntervals.reserve(NSamples);
    for(int i = 0; i < NSamples; ++i)
    {
      const auto d = samples[i + 1] - samples[i];
      const auto fps = std::chrono::seconds{1} / d;
      const auto interval = std::lround(static_cast<float>(fps) / static_cast<float>(targetFramerate));
      BOOST_LOG_TRIVIAL(debug) << "sampled " << fps << " fps, swap interval " << interval;
      swapIntervals.emplace_back(interval);
    }
    std::sort(swapIntervals.begin(), swapIntervals.end());

    BOOST_LOG_TRIVIAL(debug) << "assuming " << targetFramerate * swapIntervals[NSamples / 2] << " fps monitor";
    glfwSwapInterval(std::max(1L, swapIntervals[NSamples / 2]));
  }
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
