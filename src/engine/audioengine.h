#pragma once

#include "audio/soundengine.h"
#include "audio/voicegroup.h"
#include "core/id.h"
#include "floordata/floordata.h"
#include "loader/file/audio.h"
#include "soundeffects_tr1.h"

#include <boost/container/flat_map.hpp>
#include <filesystem>
#include <map>
#include <optional>
#include <utility>

namespace engine::world
{
class World;
}

namespace engine::script
{
class ScriptEngine;
}

namespace audio
{
class SourceHandle;
class BufferHandle;
class Voice;
} // namespace audio

namespace engine
{
enum class TR1TrackId;

class AudioEngine
{
  world::World& m_world;
  const std::filesystem::path m_rootPath;
  const std::shared_ptr<audio::SoundEngine> m_soundEngine;

  std::vector<loader::file::SoundEffectProperties> m_soundEffectProperties{};
  boost::container::flat_map<int, const loader::file::SoundEffectProperties*> m_soundEffects{};
  std::map<TR1TrackId, engine::floordata::ActivationState> m_cdTrackActivationStates;
  core::Frame m_cdTrack50time = 0_frame;
  std::shared_ptr<audio::Voice> m_underwaterAmbience;
  std::shared_ptr<audio::StreamVoice> m_ambientStream;
  std::optional<size_t> m_ambientStreamId{};
  std::shared_ptr<audio::StreamVoice> m_interceptStream;
  std::optional<size_t> m_interceptStreamId{};
  std::optional<TR1TrackId> m_currentTrack;
  std::optional<TR1SoundEffect> m_currentLaraTalk;
  std::vector<std::shared_ptr<audio::BufferHandle>> m_samples;
  audio::VoiceGroup m_music{0.8f};
  audio::VoiceGroup m_sfx{0.8f};

public:
  explicit AudioEngine(world::World& world,
                       std::filesystem::path rootPath,
                       std::shared_ptr<audio::SoundEngine> soundEngine)
      : m_world{world}
      , m_rootPath{std::move(rootPath)}
      , m_soundEngine{std::move(soundEngine)}
  {
  }

  void init(const std::vector<loader::file::SoundEffectProperties>& soundEffectProperties,
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
    m_ambientStream.reset();
    m_ambientStreamId.reset();
    m_interceptStream.reset();
    m_interceptStreamId.reset();
    m_currentTrack.reset();
    m_currentLaraTalk.reset();
  }

  std::shared_ptr<audio::Voice> playSoundEffect(const core::SoundEffectId& id, audio::Emitter* emitter);
  std::shared_ptr<audio::Voice> playSoundEffect(const core::SoundEffectId& id, const glm::vec3& pos);

  gsl::not_null<std::shared_ptr<audio::StreamVoice>>
    playStream(size_t trackId, const std::chrono::milliseconds& initialPosition = std::chrono::milliseconds{0});

  void playStopCdTrack(const script::ScriptEngine& scriptEngine, TR1TrackId trackId, bool stop);

  void triggerNormalCdTrack(const script::ScriptEngine& scriptEngine,
                            TR1TrackId trackId,
                            const floordata::ActivationState& activationRequest,
                            floordata::SequenceCondition triggerType);

  void triggerCdTrack(const script::ScriptEngine& scriptEngine,
                      TR1TrackId trackId,
                      const floordata::ActivationState& activationRequest,
                      floordata::SequenceCondition triggerType);

  void stopSoundEffect(const core::SoundEffectId& id, audio::Emitter* emitter);

  void setUnderwater(bool underwater);

  void addWav(const gsl::not_null<const uint8_t*>& buffer);

  void setMusicGain(float gain)
  {
    m_music.setGain(gain);
  }

  void setSfxGain(float gain)
  {
    m_sfx.setGain(gain);
  }

  [[nodiscard]] auto getMusicGain() const
  {
    return m_music.getGain();
  }

  [[nodiscard]] auto getSfxGain() const
  {
    return m_sfx.getGain();
  }

  [[nodiscard]] const auto& getInterceptStream() const
  {
    return m_interceptStream;
  }

  [[nodiscard]] const auto& getCurrentTrack() const
  {
    return m_currentTrack;
  }

  void cleanup()
  {
    m_music.cleanup();
    m_sfx.cleanup();
  }

  [[nodiscard]] const auto& getSoundEngine() const
  {
    return m_soundEngine;
  }

  void serialize(const serialization::Serializer<world::World>& ser);
};
} // namespace engine
