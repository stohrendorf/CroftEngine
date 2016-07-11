#include "positionalemitter.h"

#include <boost/throw_exception.hpp>

namespace audio
{
PositionalEmitter::PositionalEmitter(const std::shared_ptr<BufferHandle>& bufferHandle, const std::shared_ptr<FilterHandle>& underwaterFilter)
    : m_bufferHandle(bufferHandle)
    , m_underwaterFilter(underwaterFilter)
{
    m_source.setBuffer(bufferHandle);

    m_source.set(AL_MIN_GAIN, 0.0f);
    m_source.set(AL_MAX_GAIN, 1.0f);

    m_source.set(AL_ROOM_ROLLOFF_FACTOR, 1.0f);
    m_source.set(AL_AUXILIARY_SEND_FILTER_GAIN_AUTO, AL_TRUE);
    m_source.set(AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO, AL_TRUE);
    m_source.set(AL_AIR_ABSORPTION_FACTOR, 0.1f);
}

bool PositionalEmitter::isActive() const
{
    return m_active;
}

bool PositionalEmitter::isLooping() const
{
    ALint looping;
    alGetSourcei(m_source.get(), AL_LOOPING, &looping);
    DEBUG_CHECK_AL_ERROR();
    return looping != AL_FALSE;
}

bool PositionalEmitter::isPlaying() const
{
    ALenum state = AL_STOPPED;
    alGetSourcei(m_source.get(), AL_SOURCE_STATE, &state);
    DEBUG_CHECK_AL_ERROR();

    // Paused state and existing file pointers also counts as playing.
    return state == AL_PLAYING || state == AL_PAUSED;
}

void PositionalEmitter::play(const core::ExactTRCoordinates& coords, bool underwater)
{
    m_source.set(AL_SOURCE_RELATIVE, AL_FALSE);
    m_source.setPosition(coords.toIrrlicht());

    setUnderwater(underwater);

    m_source.play();
    DEBUG_CHECK_AL_ERROR();
    m_active = true;
}

void PositionalEmitter::setRange(ALfloat range_value)
{
    // Source will become fully audible on 1/6 of overall position.
    m_source.set(AL_REFERENCE_DISTANCE, range_value / 6.0f);
    m_source.set(AL_MAX_DISTANCE, range_value);
}

void PositionalEmitter::setUnderwater(bool underwater)
{
    // Water low-pass filter is applied when source's is_water flag is set.
    // Note that it is applied directly to channel, i. e. all sources that
    // are underwater will damp, despite of global reverb setting.

    m_source.setDirectFilter(underwater ? m_underwaterFilter : nullptr);
}
} // namespace audio
