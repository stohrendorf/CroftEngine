#pragma once

#include "audio/soundengine.h"
#include "core/id.h"
#include "floordata/floordata.h"
#include "loader/file/audio.h"
#include "sounds_tr1.h"

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

  std::vector<loader::file::SoundDetails> m_soundDetails;
  std::vector<int16_t> m_soundmap;
  std::vector<uint32_t> m_sampleIndices;
  std::map<TR1TrackId, engine::floordata::ActivationState> m_cdTrackActivationStates;
  int m_cdTrack50time = 0;
  std::weak_ptr<audio::SourceHandle> m_underwaterAmbience;
  audio::SoundEngine m_soundEngine;
  std::weak_ptr<audio::Stream> m_ambientStream;
  std::weak_ptr<audio::Stream> m_interceptStream;
  std::optional<TR1TrackId> m_currentTrack;
  std::optional<TR1SoundId> m_currentLaraTalk;

public:
  explicit AudioEngine(Engine& engine,
                       std::filesystem::path rootPath,
                       std::vector<loader::file::SoundDetails> soundDetails,
                       std::vector<int16_t> soundmap,
                       std::vector<uint32_t> sampleIndices)
      : m_engine{engine}
      , m_rootPath{std::move(rootPath)}
      , m_soundDetails{std::move(soundDetails)}
      , m_soundmap{std::move(soundmap)}
      , m_sampleIndices{std::move(sampleIndices)}
  {
  }

  explicit AudioEngine(Engine& engine, const std::filesystem::path& rootPath)
      : AudioEngine{engine, rootPath, {}, {}, {}}
  {
  }

  std::shared_ptr<audio::SourceHandle> playSound(core::SoundId id, audio::Emitter* emitter);

  std::shared_ptr<audio::SourceHandle> playSound(const core::SoundId id, const glm::vec3& pos)
  {
    const auto handle = playSound(id, nullptr);
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

  void stopSound(core::SoundId soundId, audio::Emitter* emitter);

  void setUnderwater(bool underwater);

  [[nodiscard]] auto& getSoundEngine()
  {
    return m_soundEngine;
  }
};
} // namespace engine