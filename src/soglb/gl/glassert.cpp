#include "glassert.h"

#include "api/gl.hpp"

#ifdef SOGLB_DEBUGGING
#  include "api/gl_api_provider.hpp"

#  include <boost/assert.hpp>
#  include <boost/log/trivial.hpp>
#  include <boost/stacktrace.hpp>

void gl::checkGlError(gsl::czstring code)
{
  const auto error = api::getError();
  if(error == api::ErrorCode::NoError)
    return;

  gsl::czstring errStr;
  switch(error)
  {
  case api::ErrorCode::InvalidEnum: errStr = "invalid enum"; break;
  case api::ErrorCode::InvalidOperation: errStr = "invalid operation"; break;
  case api::ErrorCode::InvalidValue: errStr = "invalid value"; break;
  case api::ErrorCode::OutOfMemory: errStr = "out of memory"; break;
  case api::ErrorCode::StackOverflow: errStr = "stack overflow"; break;
  case api::ErrorCode::StackUnderflow: errStr = "stack underflow"; break;
  case api::ErrorCode::InvalidFramebufferOperation: errStr = "invalid framebuffer operation"; break;
  default: errStr = "<unknown error>";
  }

  BOOST_LOG_TRIVIAL(error) << "OpenGL error " << static_cast<api::core::EnumType>(error) << " after evaluation of '"
                           << code << "': " << errStr;
  BOOST_LOG_TRIVIAL(error) << "Stacktrace:\n" << boost::stacktrace::stacktrace();
  BOOST_THROW_EXCEPTION(std::runtime_error("OpenAL error, see logs"));
}
#endif
