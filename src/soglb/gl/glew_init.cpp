#include "glew_init.h"

#include "glassert.h"
#include "renderstate.h"

#include <GL/glew.h>
#include <boost/log/trivial.hpp>

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
  if(source == api::DebugSource::DebugSourceApplication)
    return;
  if(severity == api::DebugSeverity::DebugSeverityNotification)
    return;

  BOOST_LOG_TRIVIAL(debug) << "GLDebug #" << id << ", severity " << glDebugSeverityToString(severity) << ", type "
                           << glDebugTypeToString(type) << ", source " << glDebugSourceToString(source) << ": "
                           << message;
}
#endif
} // namespace

void gl::initializeGl()
{
  glewExperimental = GL_TRUE; // Let GLEW ignore "GL_INVALID_ENUM in glGetString(GL_EXTENSIONS)"
  const auto err = glewInit();
  if(err != GLEW_OK)
  {
    BOOST_LOG_TRIVIAL(error) << "glewInit: "
                             << reinterpret_cast<gsl::czstring>( // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
                                  glewGetErrorString(err));
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to initialize GLEW"));
  }

  BOOST_LOG_TRIVIAL(info) << "OpenGL version: "
                          << reinterpret_cast<const char*>(api::getString(api::StringName::Version));
  BOOST_LOG_TRIVIAL(info) << "GLSL version: "
                          << reinterpret_cast<const char*>(api::getString(api::StringName::ShadingLanguageVersion));
  BOOST_LOG_TRIVIAL(info) << "OpenGL vendor: "
                          << reinterpret_cast<const char*>(api::getString(api::StringName::Vendor));
  BOOST_LOG_TRIVIAL(info) << "OpenGL renderer: "
                          << reinterpret_cast<const char*>(api::getString(api::StringName::Renderer));
  glGetError(); // clear the error flag

  Expects(GLEW_ARB_bindless_texture);

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
  return GLEW_ARB_texture_filter_anisotropic == GL_TRUE || GLEW_EXT_texture_filter_anisotropic == GL_TRUE;
}

float gl::getMaxAnisotropyLevel()
{
  Expects(hasAnisotropicFilteringExtension());
  float value = 0;
  GL_ASSERT(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &value));
  return value;
}
