#include "stream.h"

#include "device.h"

namespace audio
{
Stream::Stream(Device& device, std::unique_ptr<AbstractStreamSource>&& src, size_t bufferSize)
        : m_stream( std::move( src ) )
        , m_source{device.createSource().get()}
        , m_buffers{device.createBuffer(), device.createBuffer()}
        , m_sampleBuffer( bufferSize * 2 )
{
    init();

    m_source.lock()->play();
}

void Stream::update()
{
    if( m_source.expired() )
        return;

    const auto s = m_source.lock();

    ALint processed = 0;
    alGetSourcei( s->get(), AL_BUFFERS_PROCESSED, &processed );
    DEBUG_CHECK_AL_ERROR();
    if( processed == 0 )
        return;

    ALuint bufId;
    alSourceUnqueueBuffers( s->get(), 1, &bufId );
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
        BOOST_LOG_TRIVIAL( error ) << "Stream buffer torn, re-init";
        init();
        return;
    }

    alSourceQueueBuffers( s->get(), 1, &bufId );
    DEBUG_CHECK_AL_ERROR();
}

void Stream::init()
{
    if( m_source.expired() )
        return;

    const auto s = m_source.lock();

    fillBuffer( *m_buffers[0] );
    fillBuffer( *m_buffers[1] );

    auto tmp = m_buffers[0]->get();
    alSourceQueueBuffers( s->get(), 1, &tmp );
    DEBUG_CHECK_AL_ERROR();
    tmp = m_buffers[1]->get();
    alSourceQueueBuffers( s->get(), 1, &tmp );
    DEBUG_CHECK_AL_ERROR();
}

void Stream::fillBuffer(BufferHandle& buffer)
{
    const auto framesRead = m_stream->readStereo( m_sampleBuffer.data(), m_sampleBuffer.size() / 2 );
    buffer.fill( m_sampleBuffer.data(), framesRead * 2, 2, m_stream->getSampleRate() );
}
}
