#include "audioengine.h"

#include "objects/laraobject.h"
#include "script/reflection.h"
#include "tracks_tr1.h"
#include "world.h"

#include <boost/format.hpp>
#include <pybind11/pybind11.h>

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
      if(++m_cdTrack50time == 120)
      {
        m_world.finishLevel();
        m_cdTrack50time = 0;
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
    if(!stop)
    {
      BOOST_LOG_TRIVIAL(debug) << "playStopCdTrack - play ambient " << static_cast<size_t>(trackInfo.id.get());
      m_ambientStream = playStream(trackInfo.id.get()).get();
      m_ambientStream.lock()->setLooping(true);
      if(isPlaying(m_interceptStream))
        m_ambientStream.lock()->getSource().lock()->pause();
      m_currentTrack = trackId;
    }
    else if(const auto str = m_ambientStream.lock())
    {
      BOOST_LOG_TRIVIAL(debug) << "playStopCdTrack - stop ambient " << static_cast<size_t>(trackInfo.id.get());
      m_soundEngine->getDevice().removeStream(str);
      m_currentTrack.reset();
    }
    break;
  case audio::TrackType::Interception:
    if(!stop)
    {
      BOOST_LOG_TRIVIAL(debug) << "playStopCdTrack - play interception " << static_cast<size_t>(trackInfo.id.get());
      if(const auto str = m_interceptStream.lock())
        m_soundEngine->getDevice().removeStream(str);
      if(const auto str = m_ambientStream.lock())
        str->getSource().lock()->pause();
      m_interceptStream = playStream(trackInfo.id.get()).get();
      m_interceptStream.lock()->setLooping(false);
      m_currentTrack = trackId;
    }
    else if(const auto str = m_interceptStream.lock())
    {
      BOOST_LOG_TRIVIAL(debug) << "playStopCdTrack - stop interception " << static_cast<size_t>(trackInfo.id.get());
      m_soundEngine->getDevice().removeStream(str);
      if(const auto amb = m_ambientStream.lock())
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
  if(std::filesystem::is_regular_file(m_rootPath / "CDAUDIO.WAD"))
    result = m_soundEngine->getDevice().createStream(
      std::make_unique<audio::WadStreamSource>(m_rootPath / "CDAUDIO.WAD", trackId),
      DefaultBufferSize,
      DefaultBufferCount);
  else
    result = m_soundEngine->getDevice().createStream(
      std::make_unique<audio::SndfileStreamSource>(m_rootPath / (boost::format("%03d.ogg") % trackId).str()),
      DefaultBufferSize,
      DefaultBufferCount);

  return result;
}

std::shared_ptr<audio::SourceHandle> AudioEngine::playSoundEffect(const core::SoundEffectId& id,
                                                                  audio::Emitter* emitter)
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

  float volume = util::clamp(static_cast<float>(soundEffect->volume) / 0x7fff, 0.0f, 1.0f);
  if(soundEffect->useRandomVolume())
    volume -= util::rand15(0.25f);
  if(volume <= 0)
    return nullptr;

  std::shared_ptr<audio::SourceHandle> handle;
  if(soundEffect->getPlaybackType(loader::file::level::Engine::TR1) == loader::file::PlaybackType::Looping)
  {
    auto handles = m_soundEngine->getSourcesForBuffer(emitter, sample);
    if(handles.empty())
    {
      BOOST_LOG_TRIVIAL(trace) << "Play looping sound effect " << toString(id.get_as<TR1SoundEffect>());
      handle = m_soundEngine->playBuffer(m_buffers.at(sample), sample, pitch, volume, emitter);
      handle->setLooping(true);
      handle->play();
    }
    else
    {
      BOOST_ASSERT(handles.size() == 1);
      handle = handles[0];
    }
  }
  else if(soundEffect->getPlaybackType(loader::file::level::Engine::TR1) == loader::file::PlaybackType::Restart)
  {
    auto handles = m_soundEngine->getSourcesForBuffer(emitter, sample);
    if(!handles.empty())
    {
      BOOST_ASSERT(handles.size() == 1);
      BOOST_LOG_TRIVIAL(debug) << "Update restarting sound effect " << toString(id.get_as<TR1SoundEffect>());
      handle = handles[0];
      handle->setPitch(pitch);
      handle->setGain(volume);
      if(emitter != nullptr)
        handle->setPosition(emitter->getPosition());
      handle->play();
    }
    else
    {
      BOOST_LOG_TRIVIAL(trace) << "Play restarting sound effect " << toString(id.get_as<TR1SoundEffect>());
      handle = m_soundEngine->playBuffer(m_buffers.at(sample), sample, pitch, volume, emitter);
    }
  }
  else if(soundEffect->getPlaybackType(loader::file::level::Engine::TR1) == loader::file::PlaybackType::Wait)
  {
    auto handles = m_soundEngine->getSourcesForBuffer(emitter, sample);
    if(handles.empty())
    {
      BOOST_LOG_TRIVIAL(trace) << "Play non-playing sound effect " << toString(id.get_as<TR1SoundEffect>());
      handle = m_soundEngine->playBuffer(m_buffers.at(sample), sample, pitch, volume, emitter);
    }
    else
    {
      BOOST_ASSERT(handles.size() == 1);
      handle = handles[0];
    }
  }
  else
  {
    BOOST_LOG_TRIVIAL(trace) << "Default play mode - playing sound effect " << toString(id.get_as<TR1SoundEffect>());
    handle = m_soundEngine->playBuffer(m_buffers.at(sample), sample, pitch, volume, emitter);
  }

  return handle;
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
    if(isPlaying(m_ambientStream))
      m_ambientStream.lock()->getSource().lock()->setDirectFilter(m_soundEngine->getDevice().getUnderwaterFilter());

    if(isPlaying(m_interceptStream))
      m_interceptStream.lock()->getSource().lock()->setDirectFilter(m_soundEngine->getDevice().getUnderwaterFilter());

    if(m_underwaterAmbience.expired())
    {
      m_underwaterAmbience = playSoundEffect(TR1SoundEffect::UnderwaterAmbience, nullptr);
      m_underwaterAmbience.lock()->setLooping(true);
    }
  }
  else if(!m_underwaterAmbience.expired())
  {
    if(!m_ambientStream.expired())
      m_ambientStream.lock()->getSource().lock()->setDirectFilter(nullptr);

    if(isPlaying(m_interceptStream))
      m_interceptStream.lock()->getSource().lock()->setDirectFilter(nullptr);

    stopSoundEffect(TR1SoundEffect::UnderwaterAmbience, nullptr);
    m_underwaterAmbience.reset();
  }

  if(!isPlaying(m_interceptStream))
  {
    if(const auto str = m_ambientStream.lock())
      str->play();
  }
}

void AudioEngine::addWav(const gsl::not_null<const uint8_t*>& buffer)
{
  auto buf = std::make_shared<audio::BufferHandle>();
  buf->fillFromWav(buffer.get());
  m_buffers.emplace_back(std::move(buf));
}
} // namespace engine
