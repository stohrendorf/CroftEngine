#pragma once

#include "filterhandle.h"
#include "sourcehandle.h"
#include "stream.h"

#include <alc.h>
#include <gsl.h>
#include <set>

namespace audio
{

class Device final : public boost::noncopyable
{
public:
    explicit Device();
    ~Device();

    void update()
    {
        removeStoppedSources();
        updateStreams();
    }

    const std::shared_ptr<FilterHandle>& getUnderwaterFilter() const
    {
        return m_underwaterFilter;
    }

    void registerSource(const gsl::not_null<std::shared_ptr<SourceHandle>>& src)
    {
        m_sources.insert(src);
    }

    void registerStream(const gsl::not_null<std::shared_ptr<Stream>>& stream)
    {
        m_streams.insert(stream);
    }

    void removeStream(const std::shared_ptr<Stream>& stream)
    {
        m_streams.erase(stream);
    }

    void removeStoppedSources()
    {
        std::set<std::shared_ptr<SourceHandle>> cleaned;
        std::copy_if(m_sources.begin(), m_sources.end(), std::inserter(cleaned, cleaned.end()), [](const std::shared_ptr<SourceHandle>& src)
        {
            return !src->isStopped();
        });

        m_sources = std::move(cleaned);
    }

    void updateStreams()
    {
        for(const auto& stream : m_streams)
            stream->update();
    }

    void setListenerTransform(const irr::core::vector3df& pos, const irr::core::vector3df& front, const irr::core::vector3df& up)
    {
        alListener3f(AL_POSITION, pos.X, pos.Y, -pos.Z);

        const ALfloat o[6] = {
            front.X, front.Y, -front.Z,
            up.X, up.Y, -up.Z
        };
        alListenerfv(AL_ORIENTATION, o);
    }

private:
    ALCdevice* m_device = nullptr;
    ALCcontext* m_context = nullptr;
    std::shared_ptr<FilterHandle> m_underwaterFilter = nullptr;
    std::set<std::shared_ptr<SourceHandle>> m_sources;
    std::set<std::shared_ptr<Stream>> m_streams;
};

}
