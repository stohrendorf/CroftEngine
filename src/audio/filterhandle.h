#pragma once

#include "utils.h"

#include "alext.h"

namespace audio
{
class FilterHandle final
{
    const ALuint m_handle;

    static ALuint createHandle()
    {
        ALuint handle;
        alGenFilters( 1, &handle );
        DEBUG_CHECK_AL_ERROR();

        Expects( alIsFilter( handle ) );

        return handle;
    }

public:
    explicit FilterHandle()
            : m_handle{createHandle()}
    {
    }

    explicit FilterHandle(const FilterHandle&) = delete;

    explicit FilterHandle(FilterHandle&&) = delete;

    FilterHandle& operator=(const FilterHandle&) = delete;

    FilterHandle& operator=(FilterHandle&&) = delete;

    ~FilterHandle()
    {
        alDeleteFilters( 1, &m_handle );
        DEBUG_CHECK_AL_ERROR();
    }

    ALuint get() const noexcept
    {
        return m_handle;
    }
};
}
