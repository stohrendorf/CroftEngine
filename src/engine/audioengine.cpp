#include "audioengine.h"

#include "objects/laraobject.h"
#include "script/reflection.h"
#include "tracks_tr1.h"
#include "world.h"

#include <boost/format.hpp>
#include <pybind11/pybind11.h>
#include <soloud_wavstream.h>

namespace engine
{
void AudioEngine::triggerCdTrack(TR1TrackId trackId,
                                 const floordata::ActivationState& activationRequest,
                                 const floordata::SequenceCondition triggerType)
{
  if(trackId >= TR1TrackId::Sentinel)
    return;

  if(trackId < TR1TrackId::LaraTalk2)
  { // NOLINT(bugprone-branch-clone)
    // 1..27
    triggerNormalCdTrack(trackId, activationRequest, triggerType);
  }
  else if(trackId == TR1TrackId::LaraTalk2)
  {
    // 28
    if(m_cdTrackActivationStates[trackId].isOneshot()
       && m_world.getObjectManager().getLara().getCurrentAnimState() == loader::file::LaraStateId::JumpUp)
    {
      trackId = TR1TrackId::LaraTalk3;
    }
    triggerNormalCdTrack(trackId, activationRequest, triggerType);
  }
  else if(trackId < TR1TrackId::LaraTalk15)
  {
    // 29..40
    if(trackId != TR1TrackId::LaraTalk11)
      triggerNormalCdTrack(trackId, activationRequest, triggerType);
  }
  else if(trackId == TR1TrackId::LaraTalk15)
  { // NOLINT(bugprone-branch-clone)
    // 41
    if(m_world.getObjectManager().getLara().getCurrentAnimState() == loader::file::LaraStateId::Hang)
      triggerNormalCdTrack(trackId, activationRequest, triggerType);
  }
  else if(trackId == TR1TrackId::LaraTalk16)
  {
    // 42
    if(m_world.getObjectManager().getLara().getCurrentAnimState() == loader::file::LaraStateId::Hang)
      triggerNormalCdTrack(TR1TrackId::LaraTalk17, activationRequest, triggerType);
    else
      triggerNormalCdTrack(trackId, activationRequest, triggerType);
  }
  else if(trackId < TR1TrackId::LaraTalk23)
  {
    // 43..48
    triggerNormalCdTrack(trackId, activationRequest, triggerType);
  }
  else if(trackId == TR1TrackId::LaraTalk23)
  {
    // 49
    if(m_world.getObjectManager().getLara().getCurrentAnimState() == loader::file::LaraStateId::OnWaterStop)
      triggerNormalCdTrack(trackId, activationRequest, triggerType);
  }
  else if(trackId == TR1TrackId::LaraTalk24)
  {
    // LaraTalk24 "Right. Now I better take off these wet clothes"
    if(m_cdTrackActivationStates[trackId].isOneshot())
    {
      m_cdTrack50time += 1_frame;
      if(m_cdTrack50time == 120_frame)
      {
        m_world.finishLevel();
        m_cdTrack50time = 0_frame;
        triggerNormalCdTrack(trackId, activationRequest, triggerType);
      }
    }
    else if(m_world.getObjectManager().getLara().getCurrentAnimState() == loader::file::LaraStateId::OnWaterExit)
    {
      triggerNormalCdTrack(trackId, activationRequest, triggerType);
    }
  }
  else
  {
    // 51..64
    triggerNormalCdTrack(trackId, activationRequest, triggerType);
  }
}

void AudioEngine::triggerNormalCdTrack(const TR1TrackId trackId,
                                       const floordata::ActivationState& activationRequest,
                                       const floordata::SequenceCondition triggerType)
{
  if(trackId >= TR1TrackId::Sentinel)
    return;

  if(m_cdTrackActivationStates[trackId].isOneshot())
    return;

  if(triggerType == floordata::SequenceCondition::ItemActivated)
    m_cdTrackActivationStates[trackId] ^= activationRequest.getActivationSet();
  else if(triggerType == floordata::SequenceCondition::LaraOnGroundInverted)
    m_cdTrackActivationStates[trackId] &= ~activationRequest.getActivationSet();
  else
    m_cdTrackActivationStates[trackId] |= activationRequest.getActivationSet();

  if(!m_cdTrackActivationStates[trackId].isFullyActivated())
  {
    playStopCdTrack(trackId, true);
    return;
  }

  if(activationRequest.isOneshot())
    m_cdTrackActivationStates[trackId].setOneshot(true);

  if(!m_currentTrack.has_value() || *m_currentTrack != trackId)
    playStopCdTrack(trackId, false);
}

void AudioEngine::playStopCdTrack(const TR1TrackId trackId, bool stop)
{
  const auto trackInfo = pybind11::globals()["getTrackInfo"](trackId).cast<script::TrackInfo>();

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
    m_ambientStream.reset();
    m_currentTrack.reset();

    if(!stop)
    {
      BOOST_LOG_TRIVIAL(debug) << "playStopCdTrack - play ambient " << static_cast<size_t>(trackInfo.id.get());
      m_ambientStream = playStream(trackInfo.id.get());
      m_ambientStream->setLooping(true);
      m_interceptStream.reset();
      m_currentTrack = trackId;
    }
    break;
  case audio::TrackType::Interception:
    m_interceptStream.reset();
    m_currentTrack.reset();

    if(!stop)
    {
      BOOST_LOG_TRIVIAL(debug) << "playStopCdTrack - play interception " << static_cast<size_t>(trackInfo.id.get());
      m_interceptStream = playStream(trackInfo.id.get());
      m_interceptStream->setLooping(false);
      m_currentTrack = trackId;
    }
    break;
  }
}

gsl::not_null<std::shared_ptr<audio::Voice>> AudioEngine::playStream(size_t trackId)
{
  auto wav = std::make_shared<SoLoud::WavStream>();
  wav->load((m_rootPath / (boost::format("%03d.ogg") % trackId).str()).string().c_str());
  auto voice = m_soundEngine->playBackground(wav, m_streamVolume);
  voice->setProtect(true);
  return voice;
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

  const auto& audioSource = m_samples.at(sample);
  switch(soundEffect->getPlaybackType(loader::file::level::Engine::TR1))
  {
  case loader::file::PlaybackType::Looping:
    if(auto voices = m_soundEngine->getVoicesForAudioSource(emitter, audioSource); !voices.empty())
    {
      BOOST_ASSERT(voices.size() == 1);
      return voices[0];
    }
    else
    {
      auto voice = m_soundEngine->play(audioSource, pitch, volume, emitter);
      voice->setLooping(true);
      voice->play();
      return std::move(voice);
    }
  case loader::file::PlaybackType::Restart:
    if(auto voices = m_soundEngine->getVoicesForAudioSource(emitter, audioSource); !voices.empty())
    {
      auto handle = voices[0];
      if(!handle->isValid())
        return m_soundEngine->play(audioSource, pitch, volume, emitter);

      handle->setRelativePlaySpeed(pitch);
      handle->setVolume(volume);
      if(emitter != nullptr)
        handle->setPosition(emitter->getPosition());
      handle->restart();
      return std::move(handle);
    }
    else
    {
      return m_soundEngine->play(audioSource, pitch, volume, emitter);
    }
  case loader::file::PlaybackType::Wait:
    if(auto voices = m_soundEngine->getVoicesForAudioSource(emitter, audioSource); !voices.empty())
    {
      BOOST_ASSERT(voices.size() == 1);
      return voices[0];
    }
    else
    {
      return m_soundEngine->play(audioSource, pitch, volume, emitter);
    }
  default: return m_soundEngine->play(audioSource, pitch, volume, emitter);
  }
}

void AudioEngine::stopSoundEffect(core::SoundEffectId id, audio::Emitter* emitter)
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
    anyStopped |= m_soundEngine->stop(m_samples.at(i), emitter);
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
      m_soundEngine->getSoLoud().setGlobalFilter(0, &m_soundEngine->getUnderwaterFilter());
      m_soundEngine->getSoLoud().setFilterParameter(0, 0, SoLoud::BiquadResonantFilter::WET, 0.5f);
      m_underwaterAmbience = playSoundEffect(TR1SoundEffect::UnderwaterAmbience, nullptr);
      m_underwaterAmbience->setLooping(true);
    }
  }
  else if(m_underwaterAmbience != nullptr)
  {
    m_soundEngine->getSoLoud().setGlobalFilter(0, nullptr);

    stopSoundEffect(TR1SoundEffect::UnderwaterAmbience, nullptr);
    m_underwaterAmbience.reset();
  }
}

void AudioEngine::addWav(const gsl::not_null<const uint8_t*>& buffer)
{
  m_samples.emplace_back(audio::loadWav(buffer.get()));
}

void AudioEngine::playSoundEffect(const core::SoundEffectId id, const glm::vec3& pos)
{
  auto voice = playSoundEffect(id, nullptr);
  if(voice != nullptr)
    voice->setPosition(pos);
}
} // namespace engine
