#include "device.h"


namespace audio
{
    Device::~Device()
    {
        m_shutdown = true;
        if( m_streamUpdater.joinable() )
        {
            m_streamUpdater.join();
        }

        if( m_context )
        {
            alcMakeContextCurrent(nullptr);
            DEBUG_CHECK_AL_ERROR();
            alcDestroyContext(m_context);
            DEBUG_CHECK_AL_ERROR();
        }

        if( m_device )
        {
            alcCloseDevice(m_device);
            DEBUG_CHECK_AL_ERROR();
        }
    }


    Device::Device()
    {
        alcGetError(nullptr); // clear any error

        static const ALCint paramList[] = {
            ALC_STEREO_SOURCES, 16,
            ALC_FREQUENCY, 44100,
            0 };

        if(alcIsExtensionPresent(nullptr, "ALC_ENUMERATION_EXT") == ALC_TRUE)
        {
            DEBUG_CHECK_AL_ERROR();

            BOOST_LOG_TRIVIAL(info) << "Probing OpenAL devices...";

            const char* devlist = alcGetString(nullptr, ALC_DEVICE_SPECIFIER);
            DEBUG_CHECK_AL_ERROR();

            if(devlist == nullptr)
            {
                BOOST_LOG_TRIVIAL(warning) << BOOST_CURRENT_FUNCTION << ": No audio devices";
                return;
            }

            while(*devlist)
            {
                BOOST_LOG_TRIVIAL(info) << "Probing device `" << devlist << "`";
                ALCdevice* dev = alcOpenDevice(devlist);
                DEBUG_CHECK_AL_ERROR();

                if(dev == nullptr)
                {
                    BOOST_LOG_TRIVIAL(info) << "Failed to open device";
                    continue;
                }

                if(alcIsExtensionPresent(dev, ALC_EXT_EFX_NAME) == ALC_TRUE)
                {
                    DEBUG_CHECK_AL_ERROR();
                    BOOST_LOG_TRIVIAL(info) << "Device supports EFX";
                    m_device = dev;
                    m_context = alcCreateContext(m_device, paramList);
                    DEBUG_CHECK_AL_ERROR();
                    // fails e.g. with Rapture3D, where EFX is supported
                    if(m_context != nullptr)
                    {
                        break;
                    }
                }
                DEBUG_CHECK_AL_ERROR();

                alcCloseDevice(dev);
                DEBUG_CHECK_AL_ERROR();
                devlist += strlen(devlist) + 1;
            }
        }
        else
        {
            DEBUG_CHECK_AL_ERROR();

            BOOST_LOG_TRIVIAL(info) << "Trying to use default OpenAL device";
            m_device = alcOpenDevice(nullptr);
            DEBUG_CHECK_AL_ERROR();

            if(m_device != nullptr)
            {
                m_context = alcCreateContext(m_device, paramList);
                DEBUG_CHECK_AL_ERROR();
            }
            else
            {
                BOOST_LOG_TRIVIAL(error) << "Failed to open default device";
            }
        }

        if( m_context == nullptr )
        {
            BOOST_LOG_TRIVIAL(warning) << BOOST_CURRENT_FUNCTION << ": Failed to create OpenAL context.";
            alcCloseDevice(m_device);
            DEBUG_CHECK_AL_ERROR();
            m_device = nullptr;
        }

        alcMakeContextCurrent(m_context);
        DEBUG_CHECK_AL_ERROR();
        alListenerf(AL_METERS_PER_UNIT, 1 / 512.0f);
        DEBUG_CHECK_AL_ERROR();
        alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
        DEBUG_CHECK_AL_ERROR();

        loadALExtFunctions(m_device);

        BOOST_LOG_TRIVIAL(info) << "Using OpenAL device " << alcGetString(m_device, ALC_DEVICE_SPECIFIER);

        m_underwaterFilter = std::make_shared<FilterHandle>();

        alFilteri(m_underwaterFilter->get(), AL_FILTER_TYPE, AL_FILTER_LOWPASS);
        DEBUG_CHECK_AL_ERROR();
        alFilterf(m_underwaterFilter->get(), AL_LOWPASS_GAIN, 0.7f); // Low frequencies gain.
        DEBUG_CHECK_AL_ERROR();
        alFilterf(m_underwaterFilter->get(), AL_LOWPASS_GAINHF, 0.1f); // High frequencies gain.
        DEBUG_CHECK_AL_ERROR();

        m_streamUpdater = std::thread{
            [this]()
            {
                while( !this->m_shutdown )
                {
                    this->updateStreams();
                    std::this_thread::yield();
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }
        };

        m_streamUpdater.detach();
    }
}
