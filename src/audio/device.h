#pragma once

#include "filterhandle.h"

#include <alc.h>
#include <gsl.h>

namespace audio
{

class Device : public boost::noncopyable
{
public:
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
