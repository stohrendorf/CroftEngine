#include "audioengine.h"

#include "tracks_tr1.h"
#include "laranode.h"
#include "engine.h"
#include "script/reflection.h"

#include <boost/format.hpp>
#include <boost/filesystem/convenience.hpp>

namespace engine
{
void AudioEngine::triggerCdTrack(TR1TrackId trackId,
                                 const floordata::ActivationState& activationRequest,
                                 const floordata::SequenceCondition triggerType)
{
    if( trackId >= TR1TrackId::Sentinel )
        return;

    if( trackId < TR1TrackId::LaraTalk2 )
    {
        // 1..27
        triggerNormalCdTrack( trackId, activationRequest, triggerType );
    }
    else if( trackId == TR1TrackId::LaraTalk2 )
    {
        // 28
        if( m_cdTrackActivationStates[trackId].isOneshot()
            && m_engine.getLara().getCurrentAnimState() == LaraStateId::JumpUp )
        {
            trackId = TR1TrackId::LaraTalk3;
        }
        triggerNormalCdTrack( trackId, activationRequest, triggerType );
    }
    else if( trackId < TR1TrackId::LaraTalk15 )
    {
        // 29..40
        if( trackId != TR1TrackId::LaraTalk11 )
            triggerNormalCdTrack( trackId, activationRequest, triggerType );
    }
    else if( trackId == TR1TrackId::LaraTalk15 )
    {
        // 41
        if( m_engine.getLara().getCurrentAnimState() == LaraStateId::Hang )
            triggerNormalCdTrack( trackId, activationRequest, triggerType );
    }
    else if( trackId == TR1TrackId::LaraTalk16 )
    {
        // 42
        if( m_engine.getLara().getCurrentAnimState() == LaraStateId::Hang )
            triggerNormalCdTrack( TR1TrackId::LaraTalk17, activationRequest, triggerType );
        else
            triggerNormalCdTrack( trackId, activationRequest, triggerType );
    }
    else if( trackId < TR1TrackId::LaraTalk23 )
    {
        // 43..48
        triggerNormalCdTrack( trackId, activationRequest, triggerType );
    }
    else if( trackId == TR1TrackId::LaraTalk23 )
    {
        // 49
        if( m_engine.getLara().getCurrentAnimState() == LaraStateId::OnWaterStop )
            triggerNormalCdTrack( trackId, activationRequest, triggerType );
    }
    else if( trackId == TR1TrackId::LaraTalk24 )
    {
        // LaraTalk24 "Right. Now I better take off these wet clothes"
        if( m_cdTrackActivationStates[trackId].isOneshot() )
        {
            if( ++m_cdTrack50time == 120 )
            {
                m_engine.finishLevel();
                m_cdTrack50time = 0;
                triggerNormalCdTrack( trackId, activationRequest, triggerType );
            }
        }
        else if( m_engine.getLara().getCurrentAnimState() == LaraStateId::OnWaterExit )
        {
            triggerNormalCdTrack( trackId, activationRequest, triggerType );
        }
    }
    else
    {
        // 51..64
        triggerNormalCdTrack( trackId, activationRequest, triggerType );
    }
}

void AudioEngine::triggerNormalCdTrack(const TR1TrackId trackId,
                                       const floordata::ActivationState& activationRequest,
                                       const floordata::SequenceCondition triggerType)
{
    if( trackId >= TR1TrackId::Sentinel )
        return;

    if( m_cdTrackActivationStates[trackId].isOneshot() )
        return;

    if( triggerType == floordata::SequenceCondition::ItemActivated )
        m_cdTrackActivationStates[trackId] ^= activationRequest.getActivationSet();
    else if( triggerType == floordata::SequenceCondition::LaraOnGroundInverted )
        m_cdTrackActivationStates[trackId] &= ~activationRequest.getActivationSet();
    else
        m_cdTrackActivationStates[trackId] |= activationRequest.getActivationSet();

    if( !m_cdTrackActivationStates[trackId].isFullyActivated() )
    {
        playStopCdTrack( trackId, true );
        return;
    }

    if( activationRequest.isOneshot() )
        m_cdTrackActivationStates[trackId].setOneshot( true );

    if( !m_currentTrack.is_initialized() || *m_currentTrack != trackId )
        playStopCdTrack( trackId, false );
}

void AudioEngine::playStopCdTrack(const TR1TrackId trackId, bool stop)
{
    const auto trackInfo = m_engine.getScriptEngine()["getTrackInfo"].call<script::TrackInfo>( trackId );

    switch( trackInfo.type )
    {
        case audio::TrackType::AmbientEffect:
            if( !stop )
            {
                BOOST_LOG_TRIVIAL( debug ) << "playStopCdTrack - play effect "
                                           << toString( static_cast<TR1SoundId>(trackInfo.id) );
                playSound( core::SoundId{trackInfo.id}, nullptr );
            }
            else
            {
                BOOST_LOG_TRIVIAL( debug ) << "playStopCdTrack - stop effect "
                                           << toString( static_cast<TR1SoundId>(trackInfo.id) );
                stopSound( core::SoundId{trackInfo.id}, nullptr );
            }
            break;
        case audio::TrackType::LaraTalk:
            if( !stop )
            {
                const auto sfxId = static_cast<TR1SoundId>(trackInfo.id);

                if( !m_currentLaraTalk.is_initialized() || *m_currentLaraTalk != sfxId )
                {
                    BOOST_LOG_TRIVIAL( debug ) << "playStopCdTrack - play lara talk " << toString( sfxId );

                    if( m_currentLaraTalk.is_initialized() )
                        stopSound( *m_currentLaraTalk, &m_engine.getLara().m_state );

                    m_engine.getLara().playSoundEffect( sfxId );
                    m_currentLaraTalk = sfxId;
                }
            }
            else
            {
                BOOST_LOG_TRIVIAL( debug ) << "playStopCdTrack - stop lara talk "
                                           << toString( static_cast<TR1SoundId>(trackInfo.id) );
                stopSound( static_cast<TR1SoundId>(trackInfo.id), &m_engine.getLara().m_state );
                m_currentLaraTalk.reset();
            }
            break;
        case audio::TrackType::Ambient:
            if( !stop )
            {
                BOOST_LOG_TRIVIAL( debug ) << "playStopCdTrack - play ambient " << static_cast<size_t>(trackInfo.id);
                m_ambientStream = playStream( trackInfo.id ).get();
                m_ambientStream.lock()->setLooping( true );
                if( isPlaying( m_interceptStream ) )
                    m_ambientStream.lock()->getSource().lock()->pause();
                m_currentTrack = trackId;
            }
            else if( const auto str = m_ambientStream.lock() )
            {
                BOOST_LOG_TRIVIAL( debug ) << "playStopCdTrack - stop ambient " << static_cast<size_t>(trackInfo.id);
                m_soundEngine.getDevice().removeStream( str );
                m_currentTrack.reset();
            }
            break;
        case audio::TrackType::Interception:
            if( !stop )
            {
                BOOST_LOG_TRIVIAL( debug ) << "playStopCdTrack - play interception "
                                           << static_cast<size_t>(trackInfo.id);
                if( const auto str = m_interceptStream.lock() )
                    m_soundEngine.getDevice().removeStream( str );
                if( const auto str = m_ambientStream.lock() )
                    str->getSource().lock()->pause();
                m_interceptStream = playStream( trackInfo.id ).get();
                m_interceptStream.lock()->setLooping( false );
                m_currentTrack = trackId;
            }
            else if( const auto str = m_interceptStream.lock() )
            {
                BOOST_LOG_TRIVIAL( debug ) << "playStopCdTrack - stop interception "
                                           << static_cast<size_t>(trackInfo.id);
                m_soundEngine.getDevice().removeStream( str );
                if( const auto amb = m_ambientStream.lock() )
                    amb->play();
                m_currentTrack.reset();
            }
            break;
    }
}

gsl::not_null<std::shared_ptr<audio::Stream>> AudioEngine::playStream(size_t trackId)
{
    static constexpr size_t DefaultBufferSize = 8192;
    static constexpr size_t DefaultBufferCount = 4;

    std::shared_ptr<audio::Stream> result;
    if( boost::filesystem::is_regular_file( "data/tr1/audio/CDAUDIO.WAD" ) )
        result = m_soundEngine.getDevice().createStream(
                std::make_unique<audio::WadStreamSource>( "data/tr1/audio/CDAUDIO.WAD", trackId ),
                DefaultBufferSize,
                DefaultBufferCount );
    else
        result = m_soundEngine.getDevice().createStream(
                std::make_unique<audio::SndfileStreamSource>(
                        (boost::format( "data/tr1/audio/%03d.ogg" ) % trackId).str() ),
                DefaultBufferSize,
                DefaultBufferCount );

    result->setGain( 0.8f );
    return result;
}

std::shared_ptr<audio::SourceHandle> AudioEngine::playSound(const core::SoundId id, audio::Emitter* emitter)
{
    const auto snd = m_soundmap.at(id.get());
    if( snd < 0 )
    {
        BOOST_LOG_TRIVIAL( warning ) << "No mapped sound for id " << toString( id.get_as<TR1SoundId>() );
        return nullptr;
    }

    const loader::file::SoundDetails& details = m_soundDetails.at(snd);
    if( details.chance != 0 && util::rand15() > details.chance )
        return nullptr;

    size_t sample = details.sample.get();
    if( details.getSampleCount() > 1 )
        sample += util::rand15( details.getSampleCount() );
    BOOST_ASSERT( sample < m_sampleIndices.size() );

    float pitch = 1;
    if( details.useRandomPitch() )
        pitch = 0.9f + util::rand15( 0.2f );

    float volume = util::clamp( static_cast<float>(details.volume) / 0x7fff, 0.0f, 1.0f );
    if( details.useRandomVolume() )
        volume -= util::rand15( 0.25f );
    if( volume <= 0 )
        return nullptr;

    std::shared_ptr<audio::SourceHandle> handle;
    if( details.getPlaybackType( loader::file::level::Engine::TR1 ) == loader::file::PlaybackType::Looping )
    {
        auto handles = m_soundEngine.getSourcesForBuffer( emitter, sample );
        if( handles.empty() )
        {
            BOOST_LOG_TRIVIAL( debug ) << "Play looping sound " << toString( id.get_as<TR1SoundId>() );
            handle = m_soundEngine.playBuffer( sample, pitch, volume, emitter );
            handle->setLooping( true );
            handle->play();
        }
        else
        {
            BOOST_ASSERT( handles.size() == 1 );
        }
    }
    else if( details.getPlaybackType( loader::file::level::Engine::TR1 ) == loader::file::PlaybackType::Restart )
    {
        auto handles = m_soundEngine.getSourcesForBuffer( emitter, sample );
        if( !handles.empty() )
        {
            BOOST_ASSERT( handles.size() == 1 );
            BOOST_LOG_TRIVIAL( debug ) << "Update restarting sound " << toString( id.get_as<TR1SoundId>() );
            handle = handles[0];
            handle->setPitch( pitch );
            handle->setGain( volume );
            if( emitter != nullptr )
                handle->setPosition( emitter->getPosition() );
            handle->play();
        }
        else
        {
            BOOST_LOG_TRIVIAL( debug ) << "Play restarting sound " << toString( id.get_as<TR1SoundId>() );
            handle = m_soundEngine.playBuffer( sample, pitch, volume, emitter );
        }
    }
    else if( details.getPlaybackType( loader::file::level::Engine::TR1 ) == loader::file::PlaybackType::Wait )
    {
        auto handles = m_soundEngine.getSourcesForBuffer( emitter, sample );
        if( handles.empty() )
        {
            BOOST_LOG_TRIVIAL( debug ) << "Play non-playing sound " << toString( id.get_as<TR1SoundId>() );
            handle = m_soundEngine.playBuffer( sample, pitch, volume, emitter );
        }
        else
        {
            BOOST_ASSERT( handles.size() == 1 );
        }
    }
    else
    {
        BOOST_LOG_TRIVIAL( debug ) << "Default play mode - playing sound " << toString( id.get_as<TR1SoundId>() );
        handle = m_soundEngine.playBuffer( sample, pitch, volume, emitter );
    }

    return handle;
}

void AudioEngine::stopSound(const core::SoundId soundId, audio::Emitter* emitter)
{
    const auto& details = m_soundDetails.at( m_soundmap.at( soundId.get() ) );
    const size_t first = details.sample.get();
    const size_t last = first + details.getSampleCount();

    bool anyStopped = false;
    for( size_t i = first; i < last; ++i )
    {
        anyStopped |= m_soundEngine.stopBuffer( i, emitter );
    }

    if( !anyStopped )
        BOOST_LOG_TRIVIAL( debug ) << "Attempting to stop sound " << toString( soundId.get_as<TR1SoundId>() )
                                   << " (samples " << first << ".." << (last - 1) << ") didn't stop any sample";
    else
        BOOST_LOG_TRIVIAL( debug ) << "Stopped samples of sound " << toString( soundId.get_as<TR1SoundId>() );
}

void AudioEngine::setUnderwater(bool underwater)
{
    if( underwater )
    {
        if( isPlaying( m_ambientStream ) )
            m_ambientStream.lock()
                           ->getSource().lock()->setDirectFilter( m_soundEngine.getDevice().getUnderwaterFilter() );

        if( isPlaying( m_interceptStream ) )
            m_interceptStream.lock()
                             ->getSource().lock()->setDirectFilter( m_soundEngine.getDevice().getUnderwaterFilter() );

        if( m_underwaterAmbience.expired() )
        {
            m_underwaterAmbience = playSound( TR1SoundId::UnderwaterAmbience, nullptr );
            m_underwaterAmbience.lock()->setLooping( true );
        }
    }
    else if( !m_underwaterAmbience.expired() )
    {
        if( !m_ambientStream.expired() )
            m_ambientStream.lock()->getSource().lock()->setDirectFilter( nullptr );

        if( isPlaying( m_interceptStream ) )
            m_interceptStream.lock()->getSource().lock()->setDirectFilter( nullptr );

        stopSound( TR1SoundId::UnderwaterAmbience, nullptr );
        m_underwaterAmbience.reset();
    }

    if( !isPlaying( m_interceptStream ) )
    {
        if( const auto str = m_ambientStream.lock() )
            str->play();
    }
}
}
