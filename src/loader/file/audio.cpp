#include "audio.h"

#include "io/util.h"

namespace loader::file
{
std::unique_ptr<SoundSource> SoundSource::read(io::SDLReader& reader)
{
  std::unique_ptr<SoundSource> result = std::make_unique<SoundSource>();
  result->position = readCoordinates32(reader);
  result->sound_effect_id = reader.readU16();
  result->flags = reader.readU16();
  return result;
}

std::unique_ptr<SoundEffectProperties> SoundEffectProperties::readTr1(io::SDLReader& reader)
{
  std::unique_ptr<SoundEffectProperties> result = std::make_unique<SoundEffectProperties>();
  result->sample = core::SampleId::type(reader.readU16());
  result->volume = reader.readU16();
  result->chance = reader.readU16();
  result->sampleCountAndLoopType = reader.readU8();
  result->flags = reader.readU8();
  return result;
}

std::unique_ptr<SoundEffectProperties> SoundEffectProperties::readTr3(io::SDLReader& reader)
{
  std::unique_ptr<SoundEffectProperties> result = std::make_unique<SoundEffectProperties>();
  result->sample = core::SampleId::type(reader.readU16());
  result->volume = reader.readU8();
  result->sound_range = reader.readU8();
  result->chance = reader.readU8();
  result->pitch = reader.readI8();
  result->sampleCountAndLoopType = reader.readU8();
  result->flags = reader.readU8();
  return result;
}
} // namespace loader::file
