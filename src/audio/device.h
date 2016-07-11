#pragma once

#include "util/helpers.h"

#include <AL/al.h>
#include <AL/alc.h>

namespace audio
{
struct Device
{
    DISABLE_COPY(Device);

    explicit Device();
    ~Device();

    ALuint getUnderwaterFilter() const
    {
        return m_underwaterFilter;
    }

private:
    ALCdevice* m_device = nullptr;
    ALCcontext* m_context = nullptr;
    ALuint m_underwaterFilter;
};
}