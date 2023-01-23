#include "audioengine.h"

#include "audio/bufferhandle.h"
#include "audio/buffervoice.h"
#include "audio/device.h"
#include "audio/fadevolumecallback.h"
#include "audio/soundengine.h"
#include "audio/streamsource.h"
#include "audio/streamvoice.h"
#include "audio/wadstreamsource.h"
#include "objects/laraobject.h"
#include "script/reflection.h"
#include "serialization/map.h"
#include "serialization/optional.h"
#include "serialization/serialization.h"
#include "tracks_tr1.h"
#include "util/helpers.h"
#include "world/world.h"

#include <boost/format.hpp>

namespace engine
{
namespace
{
constexpr auto FadeOutDuration = std::chrono::seconds{2};
}

void AudioEngine::triggerCdTrack(const script::Gameflow& gameflow,
                                 TR1TrackId trackId,
                                 const floordata::ActivationState& activationRequest,
                                 const floordata::SequenceCondition triggerType)
{
  if(trackId >= TR1TrackId::Sentinel)
    return;

  if(trackId < TR1TrackId::LaraTalk2)
  { // NOLINT(bugprone-branch-clone)
    // 1..27
    triggerNormalCdTrack(gameflow, trackId, activationRequest, triggerType);
  }
  else if(trackId == TR1TrackId::LaraTalk2)
  {
    // 28
    // OK. Let's do some tumbling
    if(m_cdTrackActivationStates[trackId].isOneshot()
       && m_world.getObjectManager().getLara().getCurrentAnimState() == loader::file::LaraStateId::JumpUp)
    {
      // Now press it again
      trackId = TR1TrackId::LaraTalk3;
    }
    triggerNormalCdTrack(gameflow, trackId, activationRequest, triggerType);
  }
  else if(trackId < TR1TrackId::LaraTalk15)
  {
    // 29..40
    if(trackId != TR1TrackId::LaraTalk11) // Press forward, and I'll climb up
      triggerNormalCdTrack(gameflow, trackId, activationRequest, triggerType);
  }
  else if(trackId == TR1TrackId::LaraTalk15)
  { // NOLINT(bugprone-branch-clone)
    // 41
    // Nice
    if(m_world.getObjectManager().getLara().getCurrentAnimState() == loader::file::LaraStateId::Hang)
      triggerNormalCdTrack(gameflow, trackId, activationRequest, triggerType);
  }
  else if(trackId == TR1TrackId::LaraTalk16)
  {
    // 42
    // Try to vault up here
    if(m_world.getObjectManager().getLara().getCurrentAnimState() == loader::file::LaraStateId::Hang)
      // I can't climb up
      triggerNormalCdTrack(gameflow, TR1TrackId::LaraTalk17, activationRequest, triggerType);
    else
      triggerNormalCdTrack(gameflow, trackId, activationRequest, triggerType);
  }
  else if(trackId < TR1TrackId::LaraTalk23)
  {
    // 43..48
    triggerNormalCdTrack(gameflow, trackId, activationRequest, triggerType);
  }
  else if(trackId == TR1TrackId::LaraTalk23)
  {
    // 49
    // Wuuh! Ohh! Air!
    if(m_world.getObjectManager().getLara().getCurrentAnimState() == loader::file::LaraStateId::OnWaterStop)
      triggerNormalCdTrack(gameflow, trackId, activationRequest, triggerType);
  }
  else if(trackId == TR1TrackId::LaraTalk24)
  {
    // Right. Now I better take off these wet clothes
    if(m_cdTrackActivationStates[trackId].isOneshot())
    {
      m_cdTrack50time += 1_frame;
      if(m_cdTrack50time == core::FrameRate * 4_sec)
      {
        m_world.finishLevel();
        m_cdTrack50time = 0_frame;
        triggerNormalCdTrack(gameflow, trackId, activationRequest, triggerType);
      }
    }
    else if(m_world.getObjectManager().getLara().getCurrentAnimState() == loader::file::LaraStateId::OnWaterExit)
    {
      triggerNormalCdTrack(gameflow, trackId, activationRequest, triggerType);
    }
  }
  else
  {
    // 51..64
    triggerNormalCdTrack(gameflow, trackId, activationRequest, triggerType);
  }
}

void AudioEngine::triggerNormalCdTrack(const script::Gameflow& gameflow,
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
    playStopCdTrack(gameflow, trackId, true);
    return;
  }

  if(activationRequest.isOneshot())
    trackState.setOneshot(true);

  if(!m_currentTrack.has_value() || *m_currentTrack != trackId)
  {
    playStopCdTrack(gameflow, trackId, false);
  }
}

void AudioEngine::playStopCdTrack(const script::Gameflow& gameflow, const TR1TrackId trackId, bool stop)
{
  const gsl::not_null trackInfo{gameflow.getTracks().at(trackId)};

  m_currentTrack.reset();

  if(auto currentlyPlaying = m_soundEngine->tryGetStream(trackInfo->slot); currentlyPlaying != nullptr)
  {
    BOOST_LOG_TRIVIAL(debug) << "playStopCdTrack - fade out slot " << trackInfo->slot;
    m_soundEngine->getDevice().registerUpdateCallback(audio::FadeVolumeCallback{
      0.0f,
      FadeOutDuration,
      gsl::not_null{currentlyPlaying},
      audio::FadeVolumeCallback::FinalCallback{[currentlyPlaying, slot = trackInfo->slot, this]()
                                               {
                                                 BOOST_LOG_TRIVIAL(debug) << "playStopCdTrack - fade out slot " << slot
                                                                          << " done, removing stream";
                                                 m_soundEngine->getDevice().removeStream(currentlyPlaying);
                                               }}});
  }

  m_soundEngine->freeSlot(trackInfo->slot);

  if(stop)
    return;

  BOOST_LOG_TRIVIAL(debug) << "playStopCdTrack - play " << toString(trackId) << ", slot " << trackInfo->slot
                           << ", looping " << trackInfo->looping;
  const auto stream = createStream(trackInfo->getFirstValidAlternative(m_rootPath));
  stream->setLooping(trackInfo->looping);
  m_soundEngine->setSlotStream(trackInfo->slot, stream, trackInfo->getFirstValidAlternative(m_rootPath));
  m_currentTrack = trackId;

  if(trackInfo->fadeDurationSeconds > 0)
  {
    stream->setLocalGain(0.0f);
    m_soundEngine->getDevice().registerUpdateCallback(
      audio::FadeVolumeCallback{1.0f, std::chrono::seconds{trackInfo->fadeDurationSeconds}, stream, {}});
  }

  stream->play();
}

gslu::nn_shared<audio::StreamVoice> AudioEngine::createStream(const std::filesystem::path& path,
                                                              const std::chrono::milliseconds& initialPosition)
{
  auto stream = m_soundEngine->createStream(m_rootPath / path, initialPosition);
  m_music.add(stream);
  return stream;
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
    if(auto voices = m_soundEngine->getVoicesForBuffer(emitter, sample); !voices.empty())
    {
      gsl_Assert(voices.size() == 1);
      return voices[0];
    }
    else
    {
      auto voice = m_soundEngine->playBuffer(buffer, sample, pitch, volume, emitter);
      m_sfx.add(voice);
      voice->setLooping(true);
      voice->play();
      return voice.get();
    }
  case loader::file::PlaybackType::Restart:
    if(auto voices = m_soundEngine->getVoicesForBuffer(emitter, sample); !voices.empty())
    {
      gsl_Assert(voices.size() == 1);
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
      m_sfx.add(voice);
      return voice.get();
    }
  case loader::file::PlaybackType::Wait:
    if(auto voices = m_soundEngine->getVoicesForBuffer(emitter, sample); !voices.empty())
    {
      BOOST_ASSERT(voices.size() == 1);
      return voices[0];
    }
    else
    {
      auto voice = m_soundEngine->playBuffer(buffer, sample, pitch, volume, emitter);
      m_sfx.add(voice);
      return voice.get();
    }
  default:
  {
    auto handle = m_soundEngine->playBuffer(buffer, sample, pitch, volume, emitter);
    m_sfx.add(handle);
    return handle.get();
  }
  }
}

void AudioEngine::stopSoundEffect(const core::SoundEffectId& id, const audio::Emitter* emitter)
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
  ser(S_NV("currentTrack", m_currentTrack), S_NV("cdTrackActivationStates", m_cdTrackActivationStates));

  m_soundEngine->serializeStreams(ser, m_rootPath, m_music);
}

AudioEngine::AudioEngine(world::World& world,
                         std::filesystem::path rootPath,
                         std::shared_ptr<audio::SoundEngine> soundEngine)
    : m_world{world}
    , m_rootPath{std::move(rootPath)}
    , m_soundEngine{std::move(soundEngine)}
{
}

void AudioEngine::initForWorld(const std::vector<loader::file::SoundEffectProperties>& soundEffectProperties,
                               const std::vector<int16_t>& soundEffects)
{
  m_soundEffectProperties = soundEffectProperties;
  m_soundEffects.clear();
  for(size_t i = 0; i < soundEffects.size(); ++i)
  {
    if(soundEffects[i] < 0)
      continue;

    m_soundEffects[gsl::narrow<int>(i)] = &m_soundEffectProperties.at(soundEffects[i]);
  }
  m_cdTrackActivationStates.clear();
  m_cdTrack50time = 0_frame;
  m_underwaterAmbience.reset();
  m_currentTrack.reset();
}
} // namespace engine
