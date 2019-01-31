#pragma once

#include "utils.h"

#include "alext.h"

#include <boost/log/trivial.hpp>

namespace audio
{
class FilterHandle final
{
    const ALuint m_handle;

    static ALuint createHandle()
    {
        ALuint handle;
        AL_ASSERT( alGenFilters( 1, &handle ) );

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
        AL_ASSERT( alDeleteFilters( 1, &m_handle ) );
    }

    ALuint get() const noexcept
    {
        return m_handle;
    }
};
}
