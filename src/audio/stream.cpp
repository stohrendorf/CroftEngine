#include "stream.h"

#include "device.h"

namespace audio
{
Stream::Stream(Device& device,
               std::unique_ptr<AbstractStreamSource>&& src,
               const size_t bufferSize,
               const size_t bufferCount)
        : m_stream{std::move( src )}
        , m_source{device.createSource().get()}
        , m_sampleBuffer( bufferSize * 2 )
{
    BOOST_LOG_TRIVIAL( trace ) << "Created AL stream with buffer size " << bufferSize
                               << " and " << bufferCount << " buffers";

    Expects( bufferSize > 0 );
    Expects( bufferCount >= 2 );

    m_buffers.reserve( bufferCount );
    for( size_t i = 0; i < bufferCount; ++i )
        m_buffers.emplace_back( std::make_shared<BufferHandle>() );

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

    ALint processed = src->getBuffersProcessed();
    Expects( processed >= 0 && static_cast<size_t>(processed) <= m_buffers.size() );

    while( processed-- > 0 )
    {
        const auto bufId = src->unqueueBuffer();

        bool foundBuffer = false;
        for( const auto& buffer : m_buffers )
        {
            if( bufId == buffer->get() )
            {
                fillBuffer( *buffer );
                src->queueBuffer( *buffer );
                foundBuffer = true;
                break;
            }
        }

        if( !foundBuffer )
            BOOST_LOG_TRIVIAL( warning ) << "Got unexpected buffer ID #" << bufId;
    }
}

void Stream::init()
{
    const auto src = m_source.lock();

    if( src == nullptr )
        return;

    fillBuffer( *m_buffers[0] );
    src->queueBuffer( *m_buffers[0] );

    fillBuffer( *m_buffers[1] );
    src->queueBuffer( *m_buffers[1] );
}

void Stream::fillBuffer(BufferHandle& buffer)
{
    const auto framesRead = m_stream->readStereo( m_sampleBuffer.data(), m_sampleBuffer.size() / 2, m_looping );
    buffer.fill( m_sampleBuffer.data(), framesRead * 2, 2, m_stream->getSampleRate() );
}
}
