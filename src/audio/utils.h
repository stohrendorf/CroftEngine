#pragma once

#include <boost/current_function.hpp>

namespace audio
{
extern bool checkALError(const char* func, int line);    // AL-specific error handler.

#define CHECK_AL_ERROR() checkALError(BOOST_CURRENT_FUNCTION, __LINE__)

#ifndef NDEBUG
#define DEBUG_CHECK_AL_ERROR() CHECK_AL_ERROR()
#else
#define DEBUG_CHECK_AL_ERROR() false
#endif
} // namespace audio
