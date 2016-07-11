#pragma once

#include "bufferhandle.h"
#include "core/coordinates.h"
#include "sourcehandle.h"

namespace audio
{
class PositionalEmitter : public boost::noncopyable
{
public:
    explicit PositionalEmitter(const std::shared_ptr<BufferHandle>& bufferHandle, const std::shared_ptr<FilterHandle>& underwaterFilter);
    ~PositionalEmitter() = default;

    void play(const core::ExactTRCoordinates& coords, bool underwater);

    void setRange(ALfloat range_value);     // Set max. audible distance.
    void setUnderwater(bool underwater);                   // Apply low-pass underwater filter.

    bool isLooping() const;
    bool isPlaying() const;
    bool isActive() const;

private:
    SourceHandle m_source;
    bool m_active = false;

    const std::shared_ptr<BufferHandle> m_bufferHandle;
    const std::shared_ptr<FilterHandle> m_underwaterFilter;
};
} // namespace audio
