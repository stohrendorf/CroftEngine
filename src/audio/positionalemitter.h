#pragma once

#include "utils.h"

#include "util/helpers.h"
#include "core/coordinates.h"

#include <al.h>
#include "device.h"

namespace audio
{
class PositionalEmitter
{
    DISABLE_COPY(PositionalEmitter);
public:
    explicit PositionalEmitter(const std::shared_ptr<BufferHandle>& bufferHandle, const std::shared_ptr<FilterHandle>& underwaterFilter);
    ~PositionalEmitter() = default;

    void play(const core::ExactTRCoordinates& coords, bool underwater);
    void pause();
    void stop();

    void setLooping(bool is_looping);  // Set looping flag.
    void setPitch(ALfloat pitch_value);     // Set pitch shift.
    void setGain(ALfloat gain_value);       // Set gain (volume).
    void setRange(ALfloat range_value);     // Set max. audible distance.
    void setUnderwater(bool underwater);                   // Apply low-pass underwater filter.

    bool isLooping() const;
    bool isPlaying() const;
    bool isActive() const;

private:
    SourceHandle m_handle;
    bool m_active = false;

    const std::shared_ptr<BufferHandle> m_bufferHandle;
    const std::shared_ptr<FilterHandle> m_underwaterFilter;

    void setPosition(const irr::core::vector3df& position);   // Set source position.
};
} // namespace audio
