#include <al.h>

#include <boost/log/trivial.hpp>

namespace audio
{
    bool checkALError(const char* func, int line)
    {
        ALenum err = alGetError();
        if( err != AL_NO_ERROR )
        {
            const char* errStr = alGetString(err);
            if( errStr == nullptr )
            {
                errStr = "<unknown>";
            }

            BOOST_LOG_TRIVIAL(warning) << "OpenAL error 0x" << std::hex << err << std::dec << " (in " << func << ":" << line << "): " << errStr;
            return true;
        }
        return false;
    }
} // namespace audio
