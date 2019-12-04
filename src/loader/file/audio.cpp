#include "audio.h"

#include "io/util.h"

namespace loader::file
{
std::unique_ptr<SoundSource> SoundSource::read(io::SDLReader& reader)
{
  std::unique_ptr<SoundSource> sound_source = std::make_unique<SoundSource>();
  sound_source->position = readCoordinates32(reader);
  sound_source->sound_id = reader.readU16();
  sound_source->flags = reader.readU16();
  return sound_source;
}

std::unique_ptr<SoundDetails> SoundDetails::readTr1(io::SDLReader& reader)
{
  std::unique_ptr<SoundDetails> sound_details = std::make_unique<SoundDetails>();
  sound_details->sample = core::SampleId::type(reader.readU16());
  sound_details->volume = reader.readU16();
  sound_details->chance = reader.readU16();
  sound_details->sampleCountAndLoopType = reader.readU8();
  sound_details->flags = reader.readU8();
  return sound_details;
}

std::unique_ptr<SoundDetails> SoundDetails::readTr3(io::SDLReader& reader)
{
  std::unique_ptr<SoundDetails> sound_details = std::make_unique<SoundDetails>();
  sound_details->sample = core::SampleId::type(reader.readU16());
  sound_details->volume = reader.readU8();
  sound_details->sound_range = reader.readU8();
  sound_details->chance = reader.readU8();
  sound_details->pitch = reader.readI8();
  sound_details->sampleCountAndLoopType = reader.readU8();
  sound_details->flags = reader.readU8();
  return sound_details;
}
} // namespace loader::file
