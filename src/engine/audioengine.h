#pragma once

#include "audio/voicegroup.h"
#include "core/id.h"
#include "floordata/floordata.h"
#include "loader/file/audio.h"
#include "soundeffects_tr1.h"

#include <boost/container/flat_map.hpp>
#include <filesystem>
#include <gslu.h>
#include <map>
#include <optional>
#include <utility>

namespace engine::world
{
class World;
}

namespace engine::script
{
class Gameflow;
}

namespace audio
{
class SoundEngine;
class SourceHandle;
class BufferHandle;
class Voice;
class StreamVoice;
class Emitter;
} // namespace audio

namespace engine
{
enum class TR1TrackId;

class AudioEngine
{
  gsl::not_null<world::World*> m_world;
  std::filesystem::path m_rootPath;
  gslu::nn_shared<audio::SoundEngine> m_soundEngine;

  std::vector<loader::file::SoundEffectProperties> m_soundEffectProperties{};
  boost::container::flat_map<int, const loader::file::SoundEffectProperties*> m_soundEffects{};
  std::map<TR1TrackId, engine::floordata::ActivationState> m_cdTrackActivationStates;
  core::Frame m_cdTrack50time = 0_frame;
  std::shared_ptr<audio::Voice> m_underwaterAmbience;
  std::optional<TR1TrackId> m_currentTrack;
  std::vector<gslu::nn_shared<audio::BufferHandle>> m_samples;
  audio::VoiceGroup m_music{0.8f};
  audio::VoiceGroup m_sfx{0.8f};

public:
  explicit AudioEngine(const gsl::not_null<world::World*>& world,
                       std::filesystem::path rootPath,
                       std::shared_ptr<audio::SoundEngine> soundEngine);

  void initForWorld(const std::vector<loader::file::SoundEffectProperties>& soundEffectProperties,
                    const std::vector<int16_t>& soundEffects);

  std::shared_ptr<audio::Voice> playSoundEffect(const core::SoundEffectId& id, audio::Emitter* emitter);
  std::shared_ptr<audio::Voice> playSoundEffect(const core::SoundEffectId& id, const glm::vec3& pos);

  gslu::nn_shared<audio::StreamVoice> createStream(const std::filesystem::path& path,
                                                   const std::chrono::milliseconds& initialPosition
                                                   = std::chrono::milliseconds{0});

  void playStopCdTrack(const script::Gameflow& gameflow, TR1TrackId trackId, bool stop);

  void triggerNormalCdTrack(const script::Gameflow& gameflow,
                            TR1TrackId trackId,
                            const floordata::ActivationState& activationRequest,
                            floordata::SequenceCondition triggerType);

  void triggerCdTrack(const script::Gameflow& gameflow,
                      TR1TrackId trackId,
                      const floordata::ActivationState& activationRequest,
                      floordata::SequenceCondition triggerType);

  void stopSoundEffect(const core::SoundEffectId& id, const audio::Emitter* emitter);

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
    return *m_soundEngine;
  }

  [[nodiscard]] auto& getSoundEngine()
  {
    return *m_soundEngine;
  }

  void serialize(const serialization::Serializer<world::World>& ser) const;
  void deserialize(const serialization::Deserializer<world::World>& ser);
};
} // namespace engine
