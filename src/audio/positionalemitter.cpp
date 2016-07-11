#include "positionalemitter.h"

#include "alext.h"

#include <boost/throw_exception.hpp>
#include <boost/assert.hpp>

namespace audio
{
PositionalEmitter::PositionalEmitter(const std::shared_ptr<BufferHandle>& bufferHandle, const std::shared_ptr<FilterHandle>& underwaterFilter)
    : m_bufferHandle(bufferHandle)
    , m_underwaterFilter(underwaterFilter)
{
    m_handle.setBuffer(bufferHandle);

    alSourcef(m_handle.get(), AL_MIN_GAIN, 0.0);
    DEBUG_CHECK_AL_ERROR();
    alSourcef(m_handle.get(), AL_MAX_GAIN, 1.0);
    DEBUG_CHECK_AL_ERROR();

    alSourcef(m_handle.get(), AL_ROOM_ROLLOFF_FACTOR, 1.0);
    DEBUG_CHECK_AL_ERROR();
    alSourcei(m_handle.get(), AL_AUXILIARY_SEND_FILTER_GAIN_AUTO, AL_TRUE);
    DEBUG_CHECK_AL_ERROR();
    alSourcei(m_handle.get(), AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO, AL_TRUE);
    DEBUG_CHECK_AL_ERROR();
    alSourcef(m_handle.get(), AL_AIR_ABSORPTION_FACTOR, 0.1f);
    DEBUG_CHECK_AL_ERROR();

    alSourcei(m_handle.get(), AL_BUFFER, m_bufferHandle->get());
    DEBUG_CHECK_AL_ERROR();
}

bool PositionalEmitter::isActive() const
{
    return m_active;
}

bool PositionalEmitter::isLooping() const
{
    ALint looping;
    alGetSourcei(m_handle.get(), AL_LOOPING, &looping);
    DEBUG_CHECK_AL_ERROR();
    return looping != AL_FALSE;
}

bool PositionalEmitter::isPlaying() const
{
    ALenum state = AL_STOPPED;
    alGetSourcei(m_handle.get(), AL_SOURCE_STATE, &state);
    DEBUG_CHECK_AL_ERROR();

    // Paused state and existing file pointers also counts as playing.
    return state == AL_PLAYING || state == AL_PAUSED;
}

void PositionalEmitter::play(const core::ExactTRCoordinates& coords, bool underwater)
{
    alSourcei(m_handle.get(), AL_SOURCE_RELATIVE, AL_FALSE);
    DEBUG_CHECK_AL_ERROR();
    setPosition(coords.toIrrlicht());

    setUnderwater(underwater);

    alSourcePlay(m_handle.get());
    DEBUG_CHECK_AL_ERROR();
    m_active = true;
}

void PositionalEmitter::pause()
{
    alSourcePause(m_handle.get());
    DEBUG_CHECK_AL_ERROR();
}

void PositionalEmitter::stop()
{
    alSourceStop(m_handle.get());
    DEBUG_CHECK_AL_ERROR();
}

void PositionalEmitter::setLooping(bool is_looping)
{
    alSourcei(m_handle.get(), AL_LOOPING, is_looping ? AL_TRUE : AL_FALSE);
    DEBUG_CHECK_AL_ERROR();
}

void PositionalEmitter::setGain(ALfloat gain_value)
{
    alSourcef(m_handle.get(), AL_GAIN, irr::core::clamp(gain_value, 0.0f, 1.0f));
    DEBUG_CHECK_AL_ERROR();
}

void PositionalEmitter::setPitch(ALfloat pitch_value)
{
    // Clamp pitch value according to specs
    alSourcef(m_handle.get(), AL_PITCH, irr::core::clamp(pitch_value, 0.5f, 2.0f));
    DEBUG_CHECK_AL_ERROR();
}

void PositionalEmitter::setRange(ALfloat range_value)
{
    // Source will become fully audible on 1/6 of overall position.
    alSourcef(m_handle.get(), AL_REFERENCE_DISTANCE, range_value / 6.0f);
    DEBUG_CHECK_AL_ERROR();
    alSourcef(m_handle.get(), AL_MAX_DISTANCE, range_value);
    DEBUG_CHECK_AL_ERROR();
}

void PositionalEmitter::setPosition(const irr::core::vector3df& position)
{
    alSourcefv(m_handle.get(), AL_POSITION, &position.X);
    DEBUG_CHECK_AL_ERROR();
}

void PositionalEmitter::setUnderwater(bool underwater)
{
    // Water low-pass filter is applied when source's is_water flag is set.
    // Note that it is applied directly to channel, i. e. all sources that
    // are underwater will damp, despite of global reverb setting.

    if(underwater)
    {
        alSourcei(m_handle.get(), AL_DIRECT_FILTER, m_underwaterFilter->get());
        DEBUG_CHECK_AL_ERROR();
    }
    else
    {
        alSourcei(m_handle.get(), AL_DIRECT_FILTER, AL_FILTER_NULL);
        DEBUG_CHECK_AL_ERROR();
    }
}

} // namespace audio
