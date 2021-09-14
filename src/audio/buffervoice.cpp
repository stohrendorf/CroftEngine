#include "buffervoice.h"

#include "bufferhandle.h"
#include "sourcehandle.h"
#include "utils.h"
#include "voice.h"

#include <AL/al.h>
#include <utility>

namespace audio
{
BufferVoice::BufferVoice(gsl::not_null<std::shared_ptr<BufferHandle>> buffer)
    : Voice{}
    , m_buffer{std::move(buffer)}
{
}

BufferVoice::~BufferVoice()
{
  stop();
  if(const auto& source = getSourceHandle(); source != nullptr)
    AL_ASSERT(alSourcei(*source, AL_BUFFER, AL_NONE));
}

void BufferVoice::associate(std::unique_ptr<SourceHandle>&& source)
{
  if(source != nullptr)
    AL_ASSERT(alSourcei(*source, AL_BUFFER, *m_buffer));
  Voice::associate(std::move(source));
}

Clock::duration BufferVoice::getDuration() const
{
  return m_buffer->getDuration();
}
} // namespace audio
