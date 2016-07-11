#pragma once

#include "alext.h"

#include "util/helpers.h"
#include "utils.h"

#include <gsl.h>

#include <AL/al.h>
#include <AL/alc.h>

namespace audio
{
struct BufferHandle
{
    DISABLE_COPY(BufferHandle);

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

struct FilterHandle
{
    DISABLE_COPY(FilterHandle);

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
    explicit FilterHandle(ALuint h)
        : m_handle(h)
    {
        Expects(alIsFilter(h));
    }

    explicit FilterHandle()
        : m_handle(createHandle())
    {
    }

    ~FilterHandle()
    {
        alDeleteFilters(1, &m_handle);
        DEBUG_CHECK_AL_ERROR();
    }

    ALuint get() const noexcept
    {
        return m_handle;
    }
};

struct SourceHandle
{
    DISABLE_COPY(SourceHandle);

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
    explicit SourceHandle(ALuint h)
        : m_handle(h)
    {
        Expects(alIsSource(h));
    }

    explicit SourceHandle()
        : m_handle(createHandle())
    {
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
};

struct Device
{
    DISABLE_COPY(Device);

    explicit Device();
    ~Device();

    const std::shared_ptr<FilterHandle>& getUnderwaterFilter() const
    {
        return m_underwaterFilter;
    }

private:
    ALCdevice* m_device = nullptr;
    ALCcontext* m_context = nullptr;
    std::shared_ptr<FilterHandle> m_underwaterFilter = nullptr;
};
}
