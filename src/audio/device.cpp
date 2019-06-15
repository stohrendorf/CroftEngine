#include "device.h"

namespace audio
{
Device::~Device()
{
    m_shutdown = true;
    m_streamUpdater.join();

    m_underwaterFilter.reset();

    m_sources.clear();
    m_streams.clear();

    if( m_context != nullptr )
    {
        alcMakeContextCurrent( nullptr );
        alcDestroyContext( m_context );
    }

    if( m_device != nullptr )
    {
        alcCloseDevice( m_device );
    }
}

Device::Device()
{
    alcGetError( nullptr ); // clear any error

    static const ALCint paramList[] = {
        ALC_STEREO_SOURCES, 16,
        ALC_FREQUENCY, 44100,
        0
    };

    if( alcIsExtensionPresent( nullptr, "ALC_ENUMERATE_ALL_EXT" ) != ALC_TRUE )
    {
        BOOST_LOG_TRIVIAL( info ) << "Probing OpenAL devices...";

        const char* deviceList = alcGetString( nullptr, ALC_ALL_DEVICES_SPECIFIER );
        Expects( deviceList != nullptr );

        if( deviceList == nullptr )
        {
            BOOST_LOG_TRIVIAL( warning ) << "No audio devices";
            return;
        }

        while( *deviceList != '\0' )
        {
            BOOST_LOG_TRIVIAL( info ) << "Probing device `" << deviceList << "`";
            m_device = alcOpenDevice( deviceList );
            deviceList += strlen( deviceList ) + 1;

            if( m_device == nullptr )
            {
                BOOST_LOG_TRIVIAL( info ) << "Failed to open device";
                continue;
            }

            if( alcIsExtensionPresent( m_device, ALC_EXT_EFX_NAME ) == ALC_TRUE )
            {
                BOOST_LOG_TRIVIAL( info ) << "Device supports EFX";
                m_context = alcCreateContext( m_device, paramList );
                // fails e.g. with Rapture3D, where EFX is supported
                if( m_context != nullptr )
                {
                    break;
                }
            }

            alcCloseDevice( m_device );
            m_device = nullptr;
        }
    }
    else
    {
        BOOST_LOG_TRIVIAL( info ) << "Trying to use default OpenAL device";
        m_device = alcOpenDevice( nullptr );

        if( m_device != nullptr )
        {
            m_context = alcCreateContext( m_device, paramList );
        }
        else
        {
            BOOST_LOG_TRIVIAL( error ) << "Failed to open default device";
        }
    }

    if( m_context == nullptr )
    {
        BOOST_LOG_TRIVIAL( error ) << "Failed to create OpenAL context.";
        if( m_device != nullptr )
            alcCloseDevice( m_device );
        BOOST_THROW_EXCEPTION( std::runtime_error( "Failed to create OpenAL context." ) );
    }

    alcMakeContextCurrent( m_context );
    AL_ASSERT( alListenerf( AL_METERS_PER_UNIT, 1 / 512.0f ) );
    AL_ASSERT( alDistanceModel( AL_LINEAR_DISTANCE_CLAMPED ) );

    loadALExtFunctions( m_device );

    BOOST_LOG_TRIVIAL( info ) << "Using OpenAL device " << alcGetString( m_device, ALC_DEVICE_SPECIFIER );

    m_underwaterFilter = std::make_shared<FilterHandle>();

    AL_ASSERT( alFilteri( m_underwaterFilter->get(), AL_FILTER_TYPE, AL_FILTER_LOWPASS ) );
    AL_ASSERT( alFilterf( m_underwaterFilter->get(), AL_LOWPASS_GAIN, 0.7f ) ); // Low frequencies gain.
    AL_ASSERT( alFilterf( m_underwaterFilter->get(), AL_LOWPASS_GAINHF, 0.1f ) ); // High frequencies gain.

    m_streamUpdater = std::thread{
        [this]() {
          while( !this->m_shutdown )
          {
              this->updateStreams();
              std::this_thread::yield();
              std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
          }
        }
    };
}
}
