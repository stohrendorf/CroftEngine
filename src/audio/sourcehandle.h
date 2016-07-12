#pragma once

#include "bufferhandle.h"
#include "filterhandle.h"

#include <irrlicht.h>

namespace audio
{
    class SourceHandle : public boost::noncopyable
    {
        const ALuint m_handle;
        std::shared_ptr<BufferHandle> m_buffer;

        static ALuint createHandle()
        {
            ALuint handle;
            alGenSources(1, &handle);
            DEBUG_CHECK_AL_ERROR();

            Expects(alIsSource(handle));

            return handle;
        }

    public:
        explicit SourceHandle(ALuint h = createHandle())
            : m_handle(h)
        {
            Expects(alIsSource(h));
            set(AL_MAX_DISTANCE, 8 * 1024);
        }

        ~SourceHandle()
        {
            alSourceStop(m_handle);
            DEBUG_CHECK_AL_ERROR();
            alDeleteSources(1, &m_handle);
            DEBUG_CHECK_AL_ERROR();
        }

        ALuint get() const noexcept
        {
            return m_handle;
        }

        void setBuffer(const gsl::not_null<std::shared_ptr<BufferHandle>>& b)
        {
            m_buffer = b;
            alSourcei(m_handle, AL_BUFFER, b->get());
            DEBUG_CHECK_AL_ERROR();
        }

        void setDirectFilter(const std::shared_ptr<FilterHandle>& f)
        {
            alSourcei(m_handle, AL_DIRECT_FILTER, f ? f->get() : AL_FILTER_NULL);
            DEBUG_CHECK_AL_ERROR();
        }

        void set(ALenum e, ALint v)
        {
            alSourcei(m_handle, e, v);
            DEBUG_CHECK_AL_ERROR();
        }

        void set(ALenum e, const ALint* v)
        {
            alSourceiv(m_handle, e, v);
            DEBUG_CHECK_AL_ERROR();
        }

        void set(ALenum e, ALfloat v)
        {
            alSourcef(m_handle, e, v);
            DEBUG_CHECK_AL_ERROR();
        }

        void set(ALenum e, ALfloat a, ALfloat b, ALfloat c)
        {
            alSource3f(m_handle, e, a, b, c);
            DEBUG_CHECK_AL_ERROR();
        }

        void set(ALenum e, const ALfloat* v)
        {
            alSourcefv(m_handle, e, v);
            DEBUG_CHECK_AL_ERROR();
        }

        void play()
        {
            alSourcePlay(m_handle);
            DEBUG_CHECK_AL_ERROR();
        }

        void pause()
        {
            alSourcePause(m_handle);
            DEBUG_CHECK_AL_ERROR();
        }

        void stop()
        {
            alSourceStop(m_handle);
            DEBUG_CHECK_AL_ERROR();
        }

        bool isStopped() const
        {
            ALenum state = AL_STOPPED;
            alGetSourcei(m_handle, AL_SOURCE_STATE, &state);
            DEBUG_CHECK_AL_ERROR();

            return state == AL_STOPPED;
        }

        void setLooping(bool is_looping)
        {
            set(AL_LOOPING, is_looping ? AL_TRUE : AL_FALSE);
        }

        void setGain(ALfloat gain_value)
        {
            set(AL_GAIN, irr::core::clamp(gain_value, 0.0f, 1.0f));
        }

        void setPosition(const irr::core::vector3df& position)
        {
            set(AL_POSITION, position.X, position.Y, -position.Z);
        }

        void setPitch(ALfloat pitch_value)
        {
            // Clamp pitch value according to specs
            set(AL_PITCH, irr::core::clamp(pitch_value, 0.5f, 2.0f));
        }
    };
}
