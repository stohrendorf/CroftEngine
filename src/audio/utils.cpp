#include <al.h>

#include <boost/log/trivial.hpp>

namespace audio
{
    bool checkALError(const char* func, int line)
    {
        ALenum err = alGetError();
        if( err != AL_NO_ERROR )
        {
            const char* errStr = "<unknown>";
            switch(err)
            {
                case AL_INVALID_NAME:
                    errStr = "INVALID_NAME";
                    break;
                case AL_INVALID_ENUM:
                    errStr = "INVALID_ENUM";
                    break;
                case AL_INVALID_OPERATION:
                    errStr = "INVALID_OPERATION";
                    break;
                case AL_INVALID_VALUE:
                    errStr = "INVALID_VALUE";
                    break;
                case AL_OUT_OF_MEMORY:
                    errStr = "OUT_OF_MEMORY";
                    break;
            }

            BOOST_LOG_TRIVIAL(warning) << "OpenAL error 0x" << std::hex << err << std::dec << " (in " << func << ":" << line << "): " << errStr;
            return true;
        }
        return false;
    }
} // namespace audio
