#pragma once

#include "audio/soundengine.h"
#include "core/id.h"
#include "floordata/floordata.h"
#include "loader/file/audio.h"
#include "soundeffects_tr1.h"

#include <boost/container/flat_map.hpp>
#include <map>
#include <utility>

namespace engine
{
enum class TR1TrackId;

class Engine;

class AudioEngine
{
  Engine& m_engine;
  const std::filesystem::path m_rootPath;

  const std::vector<loader::file::SoundEffectProperties> m_soundEffectProperties;
  boost::container::flat_map<int, const loader::file::SoundEffectProperties*> m_soundEffects{};
  std::map<TR1TrackId, engine::floordata::ActivationState> m_cdTrackActivationStates;
  int m_cdTrack50time = 0;
  std::weak_ptr<audio::SourceHandle> m_underwaterAmbience;
  audio::SoundEngine m_soundEngine;
  std::weak_ptr<audio::Stream> m_ambientStream;
  std::weak_ptr<audio::Stream> m_interceptStream;
  std::optional<TR1TrackId> m_currentTrack;
  std::optional<TR1SoundEffect> m_currentLaraTalk;

public:
  explicit AudioEngine(Engine& engine,
                       std::filesystem::path rootPath,
                       std::vector<loader::file::SoundEffectProperties> soundEffectProperties,
                       const std::vector<int16_t>& soundEffects)
      : m_engine{engine}
      , m_rootPath{std::move(rootPath)}
      , m_soundEffectProperties{std::move(soundEffectProperties)}
  {
    for(size_t i = 0; i < soundEffects.size(); ++i)
    {
      if(soundEffects[i] < 0)
        continue;

      m_soundEffects[gsl::narrow<int>(i)] = &m_soundEffectProperties.at(soundEffects[i]);
    }
  }

  explicit AudioEngine(Engine& engine, const std::filesystem::path& rootPath)
      : AudioEngine{engine, rootPath, {}, {}}
  {
  }

  std::shared_ptr<audio::SourceHandle> playSoundEffect(const core::SoundEffectId& id, audio::Emitter* emitter);
  std::shared_ptr<audio::SourceHandle> playSoundEffect(const core::SoundEffectId id, const glm::vec3& pos)
  {
    auto handle = playSoundEffect(id, nullptr);
    if(handle == nullptr)
      return nullptr;

    handle->setPosition(pos);
    return handle;
  }

  gsl::not_null<std::shared_ptr<audio::Stream>> playStream(size_t trackId);

  void playStopCdTrack(TR1TrackId trackId, bool stop);

  void triggerNormalCdTrack(TR1TrackId trackId,
                            const floordata::ActivationState& activationRequest,
                            floordata::SequenceCondition triggerType);

  void triggerCdTrack(TR1TrackId trackId,
                      const floordata::ActivationState& activationRequest,
                      floordata::SequenceCondition triggerType);

  void stopSoundEffect(core::SoundEffectId id, audio::Emitter* emitter);

  void setUnderwater(bool underwater);

  [[nodiscard]] auto& getSoundEngine()
  {
    return m_soundEngine;
  }
};
} // namespace engine