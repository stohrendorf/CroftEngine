#include "utils.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <boost/log/trivial.hpp>
#include <boost/stacktrace/stacktrace.hpp>
#include <boost/throw_exception.hpp>
#include <gsl/gsl-lite.hpp>
#include <ios>
#include <stdexcept>
#include <string>

namespace audio::detail
{
void checkALError(const gsl::czstring code, const gsl::czstring func, const int line)
{
  Expects(code != nullptr);
  Expects(func != nullptr);

  const ALenum err = alGetError();
  if(err == AL_NO_ERROR)
    return;

  gsl::czstring errStr = "<unknown>";
  switch(err)
  {
  case AL_INVALID_NAME: errStr = "INVALID_NAME"; break;
  case AL_INVALID_ENUM: errStr = "INVALID_ENUM"; break;
  case AL_INVALID_OPERATION: errStr = "INVALID_OPERATION"; break;
  case AL_INVALID_VALUE: errStr = "INVALID_VALUE"; break;
  case AL_OUT_OF_MEMORY: errStr = "OUT_OF_MEMORY"; break;
  default:
    // silence compiler
    break;
  }

  BOOST_LOG_TRIVIAL(warning) << "OpenAL error 0x" << std::hex << err << std::dec << " (in " << func << ":" << line
                             << ") in statement '" << code << "': " << errStr;
  BOOST_LOG_TRIVIAL(warning) << "Stacktrace:\n" << boost::stacktrace::stacktrace();
  BOOST_THROW_EXCEPTION(std::runtime_error(std::string{"OpenAL error: "} + errStr));
}

void checkALCError(ALCdevice* device, const gsl::czstring code, const gsl::czstring func, const int line)
{
  Expects(code != nullptr);
  Expects(func != nullptr);

  const ALenum err = alcGetError(device);
  if(err == ALC_NO_ERROR)
    return;

  gsl::czstring errStr = "<unknown>";
  switch(err)
  {
  case ALC_INVALID_DEVICE: errStr = "INVALID_DEVICE"; break;
  case ALC_INVALID_CONTEXT: errStr = "INVALID_CONTEXT"; break;
  case ALC_INVALID_ENUM: errStr = "INVALID_ENUM"; break;
  case ALC_INVALID_VALUE: errStr = "INVALID_VALUE"; break;
  case ALC_OUT_OF_MEMORY: errStr = "OUT_OF_MEMORY"; break;
  default:
    // silence compiler
    break;
  }

  BOOST_LOG_TRIVIAL(warning) << "OpenAL error 0x" << std::hex << err << std::dec << " (in " << func << ":" << line
                             << ") in statement '" << code << "': " << errStr;
  BOOST_LOG_TRIVIAL(warning) << "Stacktrace:\n" << boost::stacktrace::stacktrace();
  BOOST_THROW_EXCEPTION(std::runtime_error(std::string{"OpenAL error: "} + errStr));
}
} // namespace audio::detail
