#pragma once

#include "utils.h"

#include "alext.h"

#include <boost/noncopyable.hpp>

namespace audio
{
    class FilterHandle final : public boost::noncopyable
    {
        const ALuint m_handle;

        static ALuint createHandle()
        {
            ALuint handle;
            alGenFilters(1, &handle);
            DEBUG_CHECK_AL_ERROR();

            Expects(alIsFilter(handle));

            return handle;
        }

    public:
        explicit FilterHandle()
            : m_handle(createHandle())
        {
        }

        virtual ~FilterHandle()
        {
            alDeleteFilters(1, &m_handle);
            DEBUG_CHECK_AL_ERROR();
        }

        ALuint get() const noexcept
        {
            return m_handle;
        }
    };
}
