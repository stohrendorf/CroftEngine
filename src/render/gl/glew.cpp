#include "glew.h"

#include "glassert.h"
#include "renderstate.h"

#include <gl/glew.h>

using namespace render::gl;

namespace
{
inline gsl::czstring glDebugSourceToString(const ::gl::DebugSource src)
{
  switch(src)
  {
  case ::gl::DebugSource::DebugSourceApi: return "API";
  case ::gl::DebugSource::DebugSourceApplication: return "Application";
  case ::gl::DebugSource::DebugSourceOther: return "Other";
  case ::gl::DebugSource::DebugSourceShaderCompiler: return "Shader Compiler";
  case ::gl::DebugSource::DebugSourceThirdParty: return "Third Party";
  case ::gl::DebugSource::DebugSourceWindowSystem: return "Window System";
  case ::gl::DebugSource::DontCare: return "<don't care>";
  }
  return "<unknown>";
}

inline gsl::czstring glDebugTypeToString(const ::gl::DebugType type)
{
  switch(type)
  {
  case ::gl::DebugType::DebugTypeDeprecatedBehavior: return "Deprecated Behavior";
  case ::gl::DebugType::DebugTypeError: return "Error";
  case ::gl::DebugType::DebugTypeMarker: return "Marker";
  case ::gl::DebugType::DebugTypePerformance: return "Performance";
  case ::gl::DebugType::DebugTypePopGroup: return "Pop Group";
  case ::gl::DebugType::DebugTypePushGroup: return "Push Group";
  case ::gl::DebugType::DebugTypePortability: return "Portability";
  case ::gl::DebugType::DebugTypeOther: return "Other";
  case ::gl::DebugType::DebugTypeUndefinedBehavior: return "Undefined Behaviour";
  case ::gl::DebugType::DontCare: return "<don't care>";
  }
  return "<unknown>";
}

inline gsl::czstring glDebugSeverityToString(const ::gl::DebugSeverity severity)
{
  switch(severity)
  {
  case ::gl::DebugSeverity::DebugSeverityHigh: return "High";
  case ::gl::DebugSeverity::DebugSeverityLow: return "Low";
  case ::gl::DebugSeverity::DebugSeverityMedium: return "Medium";
  case ::gl::DebugSeverity::DebugSeverityNotification: return "Notification";
  case ::gl::DebugSeverity::DontCare: return "<don't care>";
  }
  return "<unknown>";
}

void SOGLB_API debugCallback(::gl::DebugSource source,
                             ::gl::DebugType type,
                             uint32_t id,
                             ::gl::DebugSeverity severity,
                             ::gl::core::SizeType length,
                             gsl::czstring message,
                             const void* userParam)
{
  if(source == ::gl::DebugSource::DebugSourceApplication)
    return;

  BOOST_LOG_TRIVIAL(debug) << "GLDebug #" << id << ", severity " << glDebugSeverityToString(severity) << ", type "
                           << glDebugTypeToString(type) << ", source " << glDebugSourceToString(source) << ": "
                           << message;
}
} // namespace

void render::gl::initializeGl()
{
  glewExperimental = GL_TRUE; // Let GLEW ignore "GL_INVALID_ENUM in glGetString(GL_EXTENSIONS)"
  const auto err = glewInit();
  if(err != GLEW_OK)
  {
    BOOST_LOG_TRIVIAL(error) << "glewInit: " << reinterpret_cast<gsl::czstring>(glewGetErrorString(err));
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to initialize GLEW"));
  }

  glGetError(); // clear the error flag

#ifndef NDEBUG
  GL_ASSERT(::gl::enable(::gl::EnableCap::DebugOutput));
  GL_ASSERT(::gl::enable(::gl::EnableCap::DebugOutputSynchronous));

  GL_ASSERT(::gl::debugMessageCallback(&debugCallback, nullptr));
#endif

  RenderState::initDefaults();

  GL_ASSERT(::gl::enable(::gl::EnableCap::FramebufferSrgb));
}
