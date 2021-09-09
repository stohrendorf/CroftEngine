#include "audioengine.h"

#include "audio/buffervoice.h"
#include "audio/device.h"
#include "audio/soundengine.h"
#include "audio/streamvoice.h"
#include "objects/laraobject.h"
#include "script/reflection.h"
#include "script/scriptengine.h"
#include "serialization/chrono.h"
#include "serialization/map.h"
#include "serialization/optional.h"
#include "serialization/serialization.h"
#include "tracks_tr1.h"
#include "world/world.h"

#include <boost/format.hpp>

namespace engine
{
void AudioEngine::triggerCdTrack(const script::ScriptEngine& scriptEngine,
                                 TR1TrackId trackId,
                                 const floordata::ActivationState& activationRequest,
                                 const floordata::SequenceCondition triggerType)
{
  if(trackId >= TR1TrackId::Sentinel)
    return;

  if(trackId < TR1TrackId::LaraTalk2)
  { // NOLINT(bugprone-branch-clone)
    // 1..27
    triggerNormalCdTrack(scriptEngine, trackId, activationRequest, triggerType);
  }
  else if(trackId == TR1TrackId::LaraTalk2)
  {
    // 28
    if(m_cdTrackActivationStates[trackId].isOneshot()
       && m_world.getObjectManager().getLara().getCurrentAnimState() == loader::file::LaraStateId::JumpUp)
    {
      trackId = TR1TrackId::LaraTalk3;
    }
    triggerNormalCdTrack(scriptEngine, trackId, activationRequest, triggerType);
  }
  else if(trackId < TR1TrackId::LaraTalk15)
  {
    // 29..40
    if(trackId != TR1TrackId::LaraTalk11)
      triggerNormalCdTrack(scriptEngine, trackId, activationRequest, triggerType);
  }
  else if(trackId == TR1TrackId::LaraTalk15)
  { // NOLINT(bugprone-branch-clone)
    // 41
    if(m_world.getObjectManager().getLara().getCurrentAnimState() == loader::file::LaraStateId::Hang)
      triggerNormalCdTrack(scriptEngine, trackId, activationRequest, triggerType);
  }
  else if(trackId == TR1TrackId::LaraTalk16)
  {
    // 42
    if(m_world.getObjectManager().getLara().getCurrentAnimState() == loader::file::LaraStateId::Hang)
      triggerNormalCdTrack(scriptEngine, TR1TrackId::LaraTalk17, activationRequest, triggerType);
    else
      triggerNormalCdTrack(scriptEngine, trackId, activationRequest, triggerType);
  }
  else if(trackId < TR1TrackId::LaraTalk23)
  {
    // 43..48
    triggerNormalCdTrack(scriptEngine, trackId, activationRequest, triggerType);
  }
  else if(trackId == TR1TrackId::LaraTalk23)
  {
    // 49
    if(m_world.getObjectManager().getLara().getCurrentAnimState() == loader::file::LaraStateId::OnWaterStop)
      triggerNormalCdTrack(scriptEngine, trackId, activationRequest, triggerType);
  }
  else if(trackId == TR1TrackId::LaraTalk24)
  {
    // LaraTalk24 "Right. Now I better take off these wet clothes"
    if(m_cdTrackActivationStates[trackId].isOneshot())
    {
      m_cdTrack50time += 1_frame;
      if(m_cdTrack50time == core::FrameRate * 4_sec)
      {
        m_world.finishLevel();
        m_cdTrack50time = 0_frame;
        triggerNormalCdTrack(scriptEngine, trackId, activationRequest, triggerType);
      }
    }
    else if(m_world.getObjectManager().getLara().getCurrentAnimState() == loader::file::LaraStateId::OnWaterExit)
    {
      triggerNormalCdTrack(scriptEngine, trackId, activationRequest, triggerType);
    }
  }
  else
  {
    // 51..64
    triggerNormalCdTrack(scriptEngine, trackId, activationRequest, triggerType);
  }
}

void AudioEngine::triggerNormalCdTrack(const script::ScriptEngine& scriptEngine,
                                       const TR1TrackId trackId,
                                       const floordata::ActivationState& activationRequest,
                                       const floordata::SequenceCondition triggerType)
{
  if(trackId >= TR1TrackId::Sentinel)
    return;

  auto& trackState = m_cdTrackActivationStates[trackId];
  if(trackState.isOneshot())
    return;

  if(triggerType == floordata::SequenceCondition::ItemActivated)
    trackState ^= activationRequest.getActivationSet();
  else if(triggerType == floordata::SequenceCondition::LaraOnGroundInverted)
    trackState &= ~activationRequest.getActivationSet();
  else
    trackState |= activationRequest.getActivationSet();

  if(!trackState.isFullyActivated())
  {
    playStopCdTrack(scriptEngine, trackId, true);
    return;
  }

  if(activationRequest.isOneshot())
    trackState.setOneshot(true);

  if(!m_currentTrack.has_value() || *m_currentTrack != trackId)
    playStopCdTrack(scriptEngine, trackId, false);
}

void AudioEngine::playStopCdTrack(const script::ScriptEngine& scriptEngine, const TR1TrackId trackId, bool stop)
{
  const auto trackInfo = scriptEngine.getTrackInfo(trackId);

  switch(trackInfo.type)
  {
  case audio::TrackType::AmbientEffect:
    if(!stop)
    {
      BOOST_LOG_TRIVIAL(debug) << "playStopCdTrack - play sound effect "
                               << toString(static_cast<TR1SoundEffect>(trackInfo.id.get()));
      playSoundEffect(core::SoundEffectId{trackInfo.id}, nullptr);
    }
    else
    {
      BOOST_LOG_TRIVIAL(debug) << "playStopCdTrack - stop effect "
                               << toString(static_cast<TR1SoundEffect>(trackInfo.id.get()));
      stopSoundEffect(core::SoundEffectId{trackInfo.id}, nullptr);
    }
    break;
  case audio::TrackType::LaraTalk:
    if(!stop)
    {
      const auto sfxId = static_cast<TR1SoundEffect>(trackInfo.id.get());

      if(!m_currentLaraTalk.has_value() || *m_currentLaraTalk != sfxId)
      {
        BOOST_LOG_TRIVIAL(debug) << "playStopCdTrack - play lara talk " << toString(sfxId);

        if(m_currentLaraTalk.has_value())
          stopSoundEffect(*m_currentLaraTalk, &m_world.getObjectManager().getLara().m_state);

        if(const auto lara = m_world.getObjectManager().getLaraPtr())
          lara->playSoundEffect(sfxId);
        else
          playSoundEffect(sfxId, nullptr);
        m_currentLaraTalk = sfxId;
      }
    }
    else
    {
      BOOST_LOG_TRIVIAL(debug) << "playStopCdTrack - stop lara talk "
                               << toString(static_cast<TR1SoundEffect>(trackInfo.id.get()));
      stopSoundEffect(static_cast<TR1SoundEffect>(trackInfo.id.get()), &m_world.getObjectManager().getLara().m_state);
      m_currentLaraTalk.reset();
    }
    break;
  case audio::TrackType::Ambient:
    if(m_ambientStream != nullptr)
    {
      m_music.remove(m_ambientStream);
      m_soundEngine->getDevice().removeStream(m_ambientStream);
    }

    m_ambientStream.reset();
    m_ambientStreamId.reset();
    m_currentTrack.reset();

    if(!stop)
    {
      BOOST_LOG_TRIVIAL(debug) << "playStopCdTrack - play ambient " << static_cast<size_t>(trackInfo.id.get());
      m_ambientStream = playStream(trackInfo.id.get());
      m_ambientStreamId = trackInfo.id.get();
      m_ambientStream->setLooping(true);
      m_interceptStream.reset();
      m_interceptStreamId.reset();
      m_currentTrack = trackId;
    }
    break;
  case audio::TrackType::Interception:
    if(m_interceptStream != nullptr)
    {
      m_music.remove(m_interceptStream);
      m_soundEngine->getDevice().removeStream(m_interceptStream);
    }

    m_interceptStream.reset();
    m_interceptStreamId.reset();
    m_currentTrack.reset();

    if(!stop)
    {
      BOOST_LOG_TRIVIAL(debug) << "playStopCdTrack - play interception " << static_cast<size_t>(trackInfo.id.get());
      m_interceptStream = playStream(trackInfo.id.get());
      m_interceptStreamId = trackInfo.id.get();
      m_interceptStream->setLooping(false);
      m_currentTrack = trackId;
    }
    break;
  }
}

gsl::not_null<std::shared_ptr<audio::StreamVoice>>
  AudioEngine::playStream(size_t trackId, const std::chrono::milliseconds& initialPosition)
{
  static constexpr size_t DefaultBufferSize = 8192;
  static constexpr size_t DefaultBufferCount = 4;

  if(std::filesystem::is_regular_file(m_rootPath / "CDAUDIO.WAD"))
  {
    auto stream = m_soundEngine->getDevice().createStream(
      std::make_unique<audio::WadStreamSource>(m_rootPath / "CDAUDIO.WAD", trackId),
      DefaultBufferSize,
      DefaultBufferCount,
      initialPosition);
    m_music.add(stream);
    stream->play();
    return stream;
  }
  else
  {
    auto stream
      = m_soundEngine->getDevice().createStream(std::make_unique<audio::SndfileStreamSource>(util::ensureFileExists(
                                                  m_rootPath / (boost::format("%03d.ogg") % trackId).str())),
                                                DefaultBufferSize,
                                                DefaultBufferCount,
                                                initialPosition);
    m_music.add(stream);
    stream->play();
    return stream;
  }
}

std::shared_ptr<audio::Voice> AudioEngine::playSoundEffect(const core::SoundEffectId& id, audio::Emitter* emitter)
{
  const auto soundEffectIt = m_soundEffects.find(id.get());
  if(soundEffectIt == m_soundEffects.end())
  {
    BOOST_LOG_TRIVIAL(warning) << "Sound effect " << toString(id.get_as<TR1SoundEffect>()) << " not found";
    return nullptr;
  }

  const auto soundEffect = soundEffectIt->second;
  if(soundEffect->chance != 0 && util::rand15() > soundEffect->chance)
    return nullptr;

  size_t sample = soundEffect->sample.get();
  if(soundEffect->getSampleCount() > 1)
    sample += util::rand15(soundEffect->getSampleCount());

  float pitch = 1;
  if(soundEffect->useRandomPitch())
    pitch = 0.9f + util::rand15(0.2f);

  float volume = std::clamp(static_cast<float>(soundEffect->volume) / 0x7fff, 0.0f, 1.0f);
  if(soundEffect->useRandomVolume())
    volume -= util::rand15(0.25f);
  if(volume <= 0)
    return nullptr;

  const auto& buffer = m_samples.at(sample);
  switch(soundEffect->getPlaybackType(loader::file::level::Engine::TR1))
  {
  case loader::file::PlaybackType::Looping:
    // BOOST_LOG_TRIVIAL(trace) << "Play looping sound effect " << toString(id.get_as<TR1SoundEffect>());
    if(auto voices = m_soundEngine->getVoicesForBuffer(emitter, sample); !voices.empty())
    {
      Expects(voices.size() == 1);
      return voices[0];
    }
    else
    {
      auto voice = m_soundEngine->playBuffer(buffer, sample, pitch, volume, emitter);
      m_sfx.add(voice.get());
      voice->setLooping(true);
      voice->play();
      return voice.get();
    }
  case loader::file::PlaybackType::Restart:
    // BOOST_LOG_TRIVIAL(trace) << "Play restarting sound effect " << toString(id.get_as<TR1SoundEffect>());
    if(auto voices = m_soundEngine->getVoicesForBuffer(emitter, sample); !voices.empty())
    {
      Expects(voices.size() == 1);
      auto voice = voices[0];
      voice->pause();
      voice->setPitch(pitch);
      voice->setLocalGain(volume);
      if(emitter != nullptr)
        voice->setPosition(emitter->getPosition());
      voice->rewind();
      m_sfx.add(voice);
      voice->play();
      return voice;
    }
    else
    {
      auto voice = m_soundEngine->playBuffer(buffer, sample, pitch, volume, emitter);
      m_sfx.add(voice.get());
      return voice.get();
    }
  case loader::file::PlaybackType::Wait:
    // BOOST_LOG_TRIVIAL(trace) << "Play single-instance sound effect " << toString(id.get_as<TR1SoundEffect>());
    if(auto voices = m_soundEngine->getVoicesForBuffer(emitter, sample); !voices.empty())
    {
      BOOST_ASSERT(voices.size() == 1);
      return voices[0];
    }
    else
    {
      auto voice = m_soundEngine->playBuffer(buffer, sample, pitch, volume, emitter);
      m_sfx.add(voice.get());
      return voice.get();
    }
  default:
  {
    auto handle = m_soundEngine->playBuffer(buffer, sample, pitch, volume, emitter);
    m_sfx.add(handle.get());
    return handle.get();
  }
  }
}

void AudioEngine::stopSoundEffect(const core::SoundEffectId& id, audio::Emitter* emitter)
{
  const auto soundEffectIt = m_soundEffects.find(id.get());
  if(soundEffectIt == m_soundEffects.end())
    return;

  const auto soundEffect = soundEffectIt->second;
  const size_t first = soundEffect->sample.get();
  const size_t last = first + soundEffect->getSampleCount();

  bool anyStopped = false;
  for(size_t i = first; i < last; ++i)
  {
    anyStopped |= m_soundEngine->stopBuffer(i, emitter);
  }

  if(!anyStopped)
    BOOST_LOG_TRIVIAL(debug) << "Attempting to stop sound effect " << toString(id.get_as<TR1SoundEffect>())
                             << " (samples " << first << ".." << (last - 1) << ") didn't stop any sample";
  else
    BOOST_LOG_TRIVIAL(debug) << "Stopped samples of sound effect " << toString(id.get_as<TR1SoundEffect>());
}

void AudioEngine::setUnderwater(bool underwater)
{
  if(underwater)
  {
    if(m_underwaterAmbience == nullptr)
    {
      m_soundEngine->getDevice().setFilter(m_soundEngine->getDevice().getUnderwaterFilter());
      m_underwaterAmbience = playSoundEffect(TR1SoundEffect::UnderwaterAmbience, nullptr);
      m_underwaterAmbience->setLooping(true);
    }
  }
  else if(m_underwaterAmbience != nullptr)
  {
    m_soundEngine->getDevice().setFilter(nullptr);
    stopSoundEffect(TR1SoundEffect::UnderwaterAmbience, nullptr);
    m_underwaterAmbience.reset();
  }
}

void AudioEngine::addWav(const gsl::not_null<const uint8_t*>& buffer)
{
  auto handle = std::make_shared<audio::BufferHandle>();
  handle->fillFromWav(buffer.get());
  m_samples.emplace_back(std::move(handle));
}

std::shared_ptr<audio::Voice> AudioEngine::playSoundEffect(const core::SoundEffectId& id, const glm::vec3& pos)
{
  auto voice = playSoundEffect(id, nullptr);
  if(voice != nullptr)
    voice->setPosition(pos);
  return voice;
}

void AudioEngine::serialize(const serialization::Serializer<world::World>& ser)
{
  std::chrono::milliseconds ambientPosition
    = m_ambientStream == nullptr ? std::chrono::milliseconds{0} : m_ambientStream->getStreamPosition();
  std::chrono::milliseconds interceptPosition
    = m_interceptStream == nullptr ? std::chrono::milliseconds{0} : m_interceptStream->getStreamPosition();

  ser(S_NV("currentTrack", m_currentTrack),
      S_NV("currentLaraTalk", m_currentLaraTalk),
      S_NV("cdTrackActivationStates", m_cdTrackActivationStates),
      S_NV("ambientStreamId", m_ambientStreamId),
      S_NV("ambientStreamPosition", ambientPosition),
      S_NV("interceptStreamId", m_interceptStreamId),
      S_NV("interceptStreamPosition", interceptPosition));

  if(ser.loading)
  {
    if(m_ambientStream != nullptr)
    {
      m_music.remove(m_ambientStream);
      m_soundEngine->getDevice().removeStream(m_ambientStream);
    }
    m_ambientStream.reset();

    if(m_ambientStreamId.has_value())
    {
      m_ambientStream = playStream(*m_ambientStreamId, ambientPosition);
      m_ambientStream->setLooping(true);
      m_ambientStream->update();
      m_ambientStream->play();
    }

    if(m_interceptStream != nullptr)
    {
      m_music.remove(m_interceptStream);
      m_soundEngine->getDevice().removeStream(m_interceptStream);
    }
    m_interceptStream.reset();

    if(m_interceptStreamId.has_value())
    {
      m_interceptStream = playStream(*m_interceptStreamId, interceptPosition);
      m_interceptStream->update();
      m_interceptStream->play();
    }
  }
}
} // namespace engine
