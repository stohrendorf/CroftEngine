#include "stream.h"

#include "device.h"

namespace audio
{
Stream::Stream(Device& device, std::unique_ptr<AbstractStreamSource>&& src, const size_t bufferSize)
        : m_stream{std::move( src )}
        , m_source{device.createSource().get()}
        , m_buffers{device.createBuffer(), device.createBuffer()}
        , m_sampleBuffer( bufferSize * 2 )
{
    init();

    m_source.lock()->play();
}

void Stream::update()
{
    const auto src = m_source.lock();

    if( src == nullptr || src->isPaused() )
        return;

    if( src->isStopped() && !m_looping )
        return;

    ALint processed = 0;
    alGetSourcei( src->get(), AL_BUFFERS_PROCESSED, &processed );
    DEBUG_CHECK_AL_ERROR();
    if( processed == 0 )
        return;

    ALuint bufId;
    alSourceUnqueueBuffers( src->get(), 1, &bufId );
    DEBUG_CHECK_AL_ERROR();

    if( bufId == m_buffers[0]->get() )
    {
        fillBuffer( *m_buffers[0] );
    }
    else if( bufId == m_buffers[1]->get() )
    {
        fillBuffer( *m_buffers[1] );
    }
    else
    {
        BOOST_LOG_TRIVIAL( warning ) << "Got unexpected buffer ID";
        return;
    }

    alSourceQueueBuffers( src->get(), 1, &bufId );
    DEBUG_CHECK_AL_ERROR();
}

void Stream::init()
{
    const auto src = m_source.lock();

    if( src == nullptr )
        return;

    fillBuffer( *m_buffers[0] );
    fillBuffer( *m_buffers[1] );

    auto buf = m_buffers[0]->get();
    alSourceQueueBuffers( src->get(), 1, &buf );
    DEBUG_CHECK_AL_ERROR();
    buf = m_buffers[1]->get();
    alSourceQueueBuffers( src->get(), 1, &buf );
    DEBUG_CHECK_AL_ERROR();
}

void Stream::fillBuffer(BufferHandle& buffer)
{
    const auto framesRead = m_stream->readStereo( m_sampleBuffer.data(), m_sampleBuffer.size() / 2, m_looping );
    buffer.fill( m_sampleBuffer.data(), framesRead * 2, 2, m_stream->getSampleRate() );
}
}
