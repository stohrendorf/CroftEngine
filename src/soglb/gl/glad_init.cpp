#include "glad_init.h"

#include "api/gl.hpp"
#include "api/gl_api_provider.hpp"
#include "glassert.h"

#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <cstdint>
#include <cstdlib>
#include <gsl/gsl-lite.hpp>
#include <stdexcept>

using namespace gl;

namespace
{
#ifdef SOGLB_DEBUGGING
gsl::czstring glDebugSourceToString(const api::DebugSource src)
{
  switch(src)
  {
  case api::DebugSource::DebugSourceApi:
    return "API";
  case api::DebugSource::DebugSourceApplication:
    return "Application";
  case api::DebugSource::DebugSourceOther:
    return "Other";
  case api::DebugSource::DebugSourceShaderCompiler:
    return "Shader Compiler";
  case api::DebugSource::DebugSourceThirdParty:
    return "Third Party";
  case api::DebugSource::DebugSourceWindowSystem:
    return "Window System";
  case api::DebugSource::DontCare:
    return "<don't care>";
  }
  return "<unknown>";
}

gsl::czstring glDebugTypeToString(const api::DebugType type)
{
  switch(type)
  {
  case api::DebugType::DebugTypeDeprecatedBehavior:
    return "Deprecated Behavior";
  case api::DebugType::DebugTypeError:
    return "Error";
  case api::DebugType::DebugTypeMarker:
    return "Marker";
  case api::DebugType::DebugTypePerformance:
    return "Performance";
  case api::DebugType::DebugTypePopGroup:
    return "Pop Group";
  case api::DebugType::DebugTypePushGroup:
    return "Push Group";
  case api::DebugType::DebugTypePortability:
    return "Portability";
  case api::DebugType::DebugTypeOther:
    return "Other";
  case api::DebugType::DebugTypeUndefinedBehavior:
    return "Undefined Behaviour";
  case api::DebugType::DontCare:
    return "<don't care>";
  }
  return "<unknown>";
}

gsl::czstring glDebugSeverityToString(const api::DebugSeverity severity)
{
  switch(severity)
  {
  case api::DebugSeverity::DebugSeverityHigh:
    return "High";
  case api::DebugSeverity::DebugSeverityLow:
    return "Low";
  case api::DebugSeverity::DebugSeverityMedium:
    return "Medium";
  case api::DebugSeverity::DebugSeverityNotification:
    return "Notification";
  case api::DebugSeverity::DontCare:
    return "<don't care>";
  }
  return "<unknown>";
}

void SOGLB_API debugCallback(const api::DebugSource source,
                             const api::DebugType type,
                             const uint32_t id,
                             const api::DebugSeverity severity,
                             const api::core::SizeType /*length*/,
                             const gsl::czstring message,
                             const void* /*userParam*/)
{
  if(source == api::DebugSource::DebugSourceApplication
     && (type == api::DebugType::DebugTypePushGroup || type == api::DebugType::DebugTypePopGroup))
    return;

#  ifndef LOG_SPAM
  if(severity == api::DebugSeverity::DebugSeverityNotification && type == api::DebugType::DebugTypeOther
     && source == api::DebugSource::DebugSourceApi)
    return;
#  endif

  BOOST_LOG_TRIVIAL(debug) << "GLDebug #" << id << ", severity " << glDebugSeverityToString(severity) << ", type "
                           << glDebugTypeToString(type) << ", source " << glDebugSourceToString(source) << ": "
                           << message;
}

const char* amdDebugSeverityToString(GLenum severity)
{
  switch(severity)
  {
  case GL_DEBUG_SEVERITY_HIGH_AMD:
    return "high";
  case GL_DEBUG_SEVERITY_MEDIUM_AMD:
    return "medium";
  case GL_DEBUG_SEVERITY_LOW_AMD:
    return "low";
  default:
    return "<unknown>";
  }
}

const char* amdDebugCategoryToString(GLenum category)
{
  switch(category)
  {
  case GL_DEBUG_CATEGORY_API_ERROR_AMD:
    return "api error";
  case GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD:
    return "window system";
  case GL_DEBUG_CATEGORY_DEPRECATION_AMD:
    return "deprecation";
  case GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD:
    return "undefined behavior";
  case GL_DEBUG_CATEGORY_PERFORMANCE_AMD:
    return "performance";
  case GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD:
    return "shader compiler";
  case GL_DEBUG_CATEGORY_APPLICATION_AMD:
    return "application";
  case GL_DEBUG_CATEGORY_OTHER_AMD:
    return "other";
  default:
    return "<unknown>";
  }
}

void SOGLB_API amdDebugCallback(
  GLuint id, GLenum category, GLenum severity, GLsizei length, const GLchar* message, GLvoid* /*userParam*/)
{
  if(category == GL_DEBUG_CATEGORY_APPLICATION_AMD)
    return;

  Expects(length >= 0);

  BOOST_LOG_TRIVIAL(debug) << "AMD GLDebug #" << id << ", severity " << amdDebugSeverityToString(severity)
                           << ", category " << amdDebugCategoryToString(category) << ": "
                           << std::string_view(message, length);
}
#endif
} // namespace

void gl::initializeGl(void* (*loadProc)(const char* name))
{
  Expects(loadProc != nullptr);
  if(gladLoadGLLoader(loadProc) == 0)
  {
    BOOST_LOG_TRIVIAL(error) << "gladLoadGL failed";
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to initialize glad"));
  }

  BOOST_LOG_TRIVIAL(info) << "OpenGL version: "
                          // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                          << reinterpret_cast<const char*>(GL_ASSERT_FN(api::getString(api::StringName::Version)));
  BOOST_LOG_TRIVIAL(info) << "GLSL version: "
                          // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                          << reinterpret_cast<const char*>(
                               GL_ASSERT_FN(api::getString(api::StringName::ShadingLanguageVersion)));
  BOOST_LOG_TRIVIAL(info) << "OpenGL vendor: "
                          // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                          << reinterpret_cast<const char*>(GL_ASSERT_FN(api::getString(api::StringName::Vendor)));
  BOOST_LOG_TRIVIAL(info) << "OpenGL renderer: "
                          // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                          << reinterpret_cast<const char*>(GL_ASSERT_FN(api::getString(api::StringName::Renderer)));
  {
    int32_t numExts = 0;
    GL_ASSERT(api::getIntegerv(api::GetPName::NumExtensions, &numExts));
    BOOST_LOG_TRIVIAL(info) << "OpenGL extensions: " << numExts;
    gsl_Assert(numExts >= 0);
    for(int32_t i = 0; i < numExts; ++i)
    {
      BOOST_LOG_TRIVIAL(info) << " - "
                              // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                              << reinterpret_cast<const char*>(
                                   GL_ASSERT_FN(api::getString(api::StringName::Extensions, i)));
    }
  }

  if(getenv("ENABLE_VULKAN_RENDERDOC_CAPTURE") != nullptr)
  {
    BOOST_LOG_TRIVIAL(info) << "RenderDoc detected";
  }

  gsl_Assert(GLAD_GL_ARB_bindless_texture);

#ifdef SOGLB_DEBUGGING
  GL_ASSERT(api::enable(api::EnableCap::DebugOutput));
  GL_ASSERT(api::enable(api::EnableCap::DebugOutputSynchronous));

  GL_ASSERT(api::debugMessageCallback(&debugCallback, nullptr));

  if(GLAD_GL_AMD_debug_output)
  {
    GL_ASSERT(glDebugMessageEnableAMD(GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true));
    GL_ASSERT(glDebugMessageCallbackAMD(amdDebugCallback, nullptr));
  }
#endif

  GL_ASSERT(api::enable(api::EnableCap::FramebufferSrgb));

  if(!hasAnisotropicFilteringExtension())
    BOOST_LOG_TRIVIAL(info) << "Anisotropic filtering is not supported on this platform";
  else
    BOOST_LOG_TRIVIAL(info) << "Anisotropic filtering is supported on this platform, max level "
                            << getMaxAnisotropyLevel();

  GL_ASSERT(api::enable(api::EnableCap::Multisample));
  GL_ASSERT(api::enable(api::EnableCap::SampleShading));
  GL_ASSERT(api::enable(api::EnableCap::Dither));
  GL_ASSERT(api::minSampleShading(1.0f));
}

bool gl::hasAnisotropicFilteringExtension()
{
  return GLAD_GL_ARB_texture_filter_anisotropic == GL_TRUE || GLAD_GL_EXT_texture_filter_anisotropic == GL_TRUE;
}

float gl::getMaxAnisotropyLevel()
{
  Expects(hasAnisotropicFilteringExtension());
  float value = 0;
  GL_ASSERT(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &value));
  return value;
}
