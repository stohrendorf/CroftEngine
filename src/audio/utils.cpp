#include <SDL2/SDL.h>

#include <al.h>

#include <boost/log/trivial.hpp>

namespace audio
{
// ======== Audio source global methods ========

bool checkALError(const char* func, int line)
{
    ALenum err = alGetError();
    if(err != AL_NO_ERROR)
    {
        const char* errStr = alGetString(err);
        if(errStr == nullptr)
            errStr = "<unknown>";

        BOOST_LOG_TRIVIAL(warning) << "OpenAL error (in " << func << ":" << line << "): " << errStr;
        return true;
    }
    return false;
}
} // namespace audio
