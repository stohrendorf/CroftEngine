#include "glad_init.h"

#include "api/gl.hpp"
#include "api/gl_api_provider.hpp"
#include "glassert.h"

#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <cstdint>
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
  case api::DebugSource::DebugSourceApi: return "API";
  case api::DebugSource::DebugSourceApplication: return "Application";
  case api::DebugSource::DebugSourceOther: return "Other";
  case api::DebugSource::DebugSourceShaderCompiler: return "Shader Compiler";
  case api::DebugSource::DebugSourceThirdParty: return "Third Party";
  case api::DebugSource::DebugSourceWindowSystem: return "Window System";
  case api::DebugSource::DontCare: return "<don't care>";
  }
  return "<unknown>";
}

gsl::czstring glDebugTypeToString(const api::DebugType type)
{
  switch(type)
  {
  case api::DebugType::DebugTypeDeprecatedBehavior: return "Deprecated Behavior";
  case api::DebugType::DebugTypeError: return "Error";
  case api::DebugType::DebugTypeMarker: return "Marker";
  case api::DebugType::DebugTypePerformance: return "Performance";
  case api::DebugType::DebugTypePopGroup: return "Pop Group";
  case api::DebugType::DebugTypePushGroup: return "Push Group";
  case api::DebugType::DebugTypePortability: return "Portability";
  case api::DebugType::DebugTypeOther: return "Other";
  case api::DebugType::DebugTypeUndefinedBehavior: return "Undefined Behaviour";
  case api::DebugType::DontCare: return "<don't care>";
  }
  return "<unknown>";
}

gsl::czstring glDebugSeverityToString(const api::DebugSeverity severity)
{
  switch(severity)
  {
  case api::DebugSeverity::DebugSeverityHigh: return "High";
  case api::DebugSeverity::DebugSeverityLow: return "Low";
  case api::DebugSeverity::DebugSeverityMedium: return "Medium";
  case api::DebugSeverity::DebugSeverityNotification: return "Notification";
  case api::DebugSeverity::DontCare: return "<don't care>";
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

  BOOST_LOG_TRIVIAL(debug) << "GLDebug #" << id << ", severity " << glDebugSeverityToString(severity) << ", type "
                           << glDebugTypeToString(type) << ", source " << glDebugSourceToString(source) << ": "
                           << message;
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
                          << reinterpret_cast<const char*>(api::getString(api::StringName::Version));
  BOOST_LOG_TRIVIAL(info) << "GLSL version: "
                          // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                          << reinterpret_cast<const char*>(api::getString(api::StringName::ShadingLanguageVersion));
  BOOST_LOG_TRIVIAL(info) << "OpenGL vendor: "
                          // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                          << reinterpret_cast<const char*>(api::getString(api::StringName::Vendor));
  BOOST_LOG_TRIVIAL(info) << "OpenGL renderer: "
                          // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                          << reinterpret_cast<const char*>(api::getString(api::StringName::Renderer));
  glGetError(); // clear the error flag

  Expects(GLAD_GL_ARB_bindless_texture);

#ifdef SOGLB_DEBUGGING
  GL_ASSERT(api::enable(api::EnableCap::DebugOutput));
  GL_ASSERT(api::enable(api::EnableCap::DebugOutputSynchronous));

  GL_ASSERT(api::debugMessageCallback(&debugCallback, nullptr));
#endif

  GL_ASSERT(api::enable(api::EnableCap::FramebufferSrgb));

  if(!hasAnisotropicFilteringExtension())
    BOOST_LOG_TRIVIAL(info) << "Anisotropic filtering is not supported on this platform";
  else
    BOOST_LOG_TRIVIAL(info) << "Anisotropic filtering is supported on this platform, max level "
                            << getMaxAnisotropyLevel();
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
