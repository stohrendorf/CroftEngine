#include "soundengine.h"

#include "buffervoice.h"
#include "device.h"
#include "emitter.h"
#include "ffmpegstreamsource.h"
#include "listener.h"
#include "serialization/chrono.h"
#include "serialization/map.h"
#include "serialization/path.h"
#include "serialization/serialization.h"
#include "streamsource.h"
#include "streamvoice.h"
#include "util/helpers.h"
#include "voice.h"
#include "voicegroup.h"

#include <AL/al.h>
#include <algorithm>
#include <boost/log/trivial.hpp>
#include <exception>
#include <glm/fwd.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <iterator>
#include <type_traits>
#include <utility>

namespace audio
{
void SoundEngine::update()
{
  m_device->update();
  if(m_listener != nullptr)
  {
    m_device->setListenerTransform(m_listener->getPosition(), m_listener->getFrontVector(), m_listener->getUpVector());
  }

  for(auto& [emitter, idsAndVoices] : m_voices)
  {
    glm::vec3 pos;
    if(emitter != nullptr)
      pos = emitter->getPosition();

    for(auto& [id, voices] : idsAndVoices)
    {
      auto old = std::move(voices);
      std::copy_if(old.begin(),
                   old.end(),
                   std::back_inserter(voices),
                   [](const auto& v)
                   {
                     return !v.expired();
                   });

      if(emitter == nullptr)
        continue;

      for(const auto& voice : voices)
      {
        if(const auto locked = voice.lock())
        {
          locked->setPosition(pos);
        }
      }
    }
  }
}

std::vector<gslu::nn_shared<Voice>> SoundEngine::getVoicesForBuffer(Emitter* emitter, size_t buffer) const
{
  const auto it1 = m_voices.find(emitter);
  if(it1 == m_voices.end())
    return {};

  const auto it2 = it1->second.find(buffer);
  if(it2 == it1->second.end())
    return {};

  std::vector<gslu::nn_shared<Voice>> result;
  for(const auto& v : it2->second)
    if(auto locked = v.lock())
      result.emplace_back(std::move(locked));

  return result;
}

bool SoundEngine::stopBuffer(size_t bufferId, const Emitter* emitter)
{
  auto buffersVoicesIt = m_voices.find(emitter);
  if(buffersVoicesIt == m_voices.end())
    return false;

  const auto voicesIt = buffersVoicesIt->second.find(bufferId);
  if(voicesIt == buffersVoicesIt->second.end())
    return false;

  bool any = false;
  for(const auto& v : voicesIt->second)
  {
    if(const auto locked = v.lock())
    {
      locked->stop();
      any = true;
    }
  }

  buffersVoicesIt->second.erase(voicesIt);

  return any;
}

gslu::nn_shared<BufferVoice> SoundEngine::playBuffer(
  const gslu::nn_shared<BufferHandle>& buffer, size_t bufferId, ALfloat pitch, ALfloat volume, Emitter* emitter)
{
  auto voice = gsl::make_shared<BufferVoice>(buffer);
  voice->setPitch(pitch);
  voice->setLocalGain(volume);
  if(emitter != nullptr)
    voice->setPosition(emitter->getPosition());
  voice->play();

  m_voices[emitter][bufferId].emplace_back(voice.get());
  m_device->registerVoice(voice);

  return voice;
}

void SoundEngine::dropEmitter(const Emitter* emitter)
{
  const auto it = m_voices.find(emitter);
  if(it == m_voices.end())
    return;

  for(const auto& [id, voices] : it->second)
    for(const auto& voice : voices)
      if(const auto locked = voice.lock())
        locked->stop();

  m_voices.erase(it);
}

SoundEngine::SoundEngine()
    : m_device{gsl::make_unique<Device>()}
{
}

SoundEngine::~SoundEngine()
{
  reset();

  for(auto& emitter : m_emitters)
  {
    dropEmitter(emitter);
    emitter->m_engine = nullptr;
  }

  for(auto& listener : m_listeners)
    listener->m_engine = nullptr;
}

gslu::nn_shared<BufferVoice> SoundEngine::playBuffer(
  const gslu::nn_shared<BufferHandle>& buffer, size_t bufferId, ALfloat pitch, ALfloat volume, const glm::vec3& pos)
{
  auto voice = playBuffer(buffer, bufferId, pitch, volume, nullptr);
  voice->setPosition(pos);
  return voice;
}

void SoundEngine::reset()
{
  BOOST_LOG_TRIVIAL(debug) << "Resetting sound engine";
  for(const auto& [streamId, slotStream] : m_slots)
    m_device->removeStream(slotStream.stream);
  m_slots.clear();
  m_device->reset();
  m_voices.clear();
  m_listener = nullptr;
  m_emitters.clear();
  m_listeners.clear();
}

void SoundEngine::setListenerGain(float gain)
{
  m_device->setListenerGain(gain);
}

void SoundEngine::setSlotStream(size_t slot,
                                const gsl::shared_ptr<StreamVoice>& stream,
                                const std::filesystem::path& path)
{
  m_slots.erase(slot);
  m_slots.emplace(slot, SlotStream{stream, path});
}

std::shared_ptr<StreamVoice> SoundEngine::tryGetStream(size_t slot)
{
  if(auto slotIt = m_slots.find(slot); slotIt != m_slots.end())
  {
    return slotIt->second.stream.lock();
  }

  return nullptr;
}

void SoundEngine::freeSlot(size_t slot)
{
  m_slots.erase(slot);
}

void SoundEngine::serializeStreams(const serialization::Serializer<engine::world::World>& ser)
{
  std::map<size_t, std::chrono::milliseconds> positions;
  std::map<size_t, std::filesystem::path> names;
  std::map<size_t, bool> looping;

  auto tmp = std::exchange(m_slots, {});
  for(const auto& [slot, slotStream] : tmp)
  {
    if(auto stream = slotStream.stream.lock(); stream != nullptr && !stream->isStopped())
    {
      m_slots.emplace(slot, slotStream);
      positions.emplace(slot, stream->getStreamPosition());
      names.emplace(slot, slotStream.name);
      looping.emplace(slot, stream->isLooping());
    }
  }

  ser(S_NV("streamNames", names), S_NV("streamPositions", positions), S_NV("streamLooping", looping));
}

void SoundEngine::deserializeStreams(const serialization::Deserializer<engine::world::World>& ser,
                                     const std::filesystem::path& rootPath,
                                     VoiceGroup& streamGroup)
{
  std::map<size_t, std::chrono::milliseconds> positions;
  std::map<size_t, std::filesystem::path> names;
  std::map<size_t, bool> looping;

  for(const auto& [slot, slotStream] : m_slots)
    m_device->removeStream(slotStream.stream);
  m_slots.clear();

  ser(S_NV("streamNames", names), S_NV("streamPositions", positions), S_NV("streamLooping", looping));

  for(auto& [slot, streamName] : names)
  {
    const auto stream = createStream(rootPath / streamName, positions.at(slot));
    stream->setLooping(looping.at(slot));
    stream->play();
    setSlotStream(slot, stream, streamName);
    streamGroup.add(stream);
  }
}

gslu::nn_shared<StreamVoice> SoundEngine::createStream(const std::filesystem::path& path,
                                                       const std::chrono::milliseconds& initialPosition)
{
  static constexpr size_t DefaultBufferSize = 8192;
  static constexpr size_t DefaultBufferCount = 4;

  return m_device->createStream(std::make_unique<FfmpegStreamSource>(util::ensureFileExists(path)),
                                DefaultBufferSize,
                                DefaultBufferCount,
                                initialPosition);
}
} // namespace audio
