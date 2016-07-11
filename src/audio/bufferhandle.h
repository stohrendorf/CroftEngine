#pragma once

#include "utils.h"

#include <al.h>
#include <boost/noncopyable.hpp>
#include <gsl.h>

namespace audio
{
    class BufferHandle : public boost::noncopyable
    {
        const ALuint m_handle;

        static ALuint createHandle()
        {
            ALuint handle;
            alGenBuffers(1, &handle);
            DEBUG_CHECK_AL_ERROR();

            Expects(alIsBuffer(handle));

            return handle;
        }

    public:
        explicit BufferHandle(ALuint h)
            : m_handle(h)
        {
            Expects(alIsBuffer(h));
        }

        explicit BufferHandle()
            : m_handle(createHandle())
        {
        }

        ~BufferHandle()
        {
            alDeleteBuffers(1, &m_handle);
            DEBUG_CHECK_AL_ERROR();
        }

        ALuint get() const noexcept
        {
            return m_handle;
        }
    };
}
