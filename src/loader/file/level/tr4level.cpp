#include "tr4level.h"

#include "core/id.h"
#include "loader/file/animation.h"
#include "loader/file/audio.h"
#include "loader/file/datatypes.h"
#include "loader/file/item.h"
#include "loader/file/meshes.h"
#include "loader/file/texture.h"

#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <map>
#include <memory>
#include <stdexcept>
#include <vector>

namespace loader::file::level
{
#define TR_AUDIO_MAP_SIZE_TR4 370

void TR4Level::loadFileData()
{
  // Version
  const uint32_t file_version = m_reader.readU32();

  if(file_version != 0x00345254 /*&& file_version != 0x63345254*/) // +TRLE
    BOOST_THROW_EXCEPTION(std::runtime_error("TR4 Level: Wrong level version"));

  std::vector<WordTexture> texture16;
  {
    const auto numRoomTextiles = m_reader.readU16();
    const auto numObjTextiles = m_reader.readU16();
    const auto numBumpTextiles = m_reader.readU16();
    const auto numMiscTextiles = 2;
    const auto numTextiles = numRoomTextiles + numObjTextiles + numBumpTextiles + numMiscTextiles;

    uint32_t uncomp_size = m_reader.readU32();
    if(uncomp_size == 0)
      BOOST_THROW_EXCEPTION(std::runtime_error("TR4 Level: textiles32 is empty"));

    uint32_t comp_size = m_reader.readU32();
    if(comp_size > 0)
    {
      std::vector<uint8_t> comp_buffer(comp_size);
      m_reader.readBytes(comp_buffer.data(), comp_size);

      auto newsrc = io::SDLReader::decompress(comp_buffer, uncomp_size);
      newsrc.readVector(m_atlases, numTextiles - numMiscTextiles, &DWordTexture::read);
    }

    uncomp_size = m_reader.readU32();
    if(uncomp_size == 0)
      BOOST_THROW_EXCEPTION(std::runtime_error("TR4 Level: textiles16 is empty"));

    comp_size = m_reader.readU32();
    if(comp_size > 0)
    {
      if(m_atlases.empty())
      {
        std::vector<uint8_t> comp_buffer(comp_size);
        m_reader.readBytes(comp_buffer.data(), comp_size);

        auto newsrc = io::SDLReader::decompress(comp_buffer, uncomp_size);
        newsrc.readVector(texture16, numTextiles - numMiscTextiles, &WordTexture::read);
      }
      else
      {
        m_reader.skip(comp_size);
      }
    }

    uncomp_size = m_reader.readU32();
    if(uncomp_size == 0)
      BOOST_THROW_EXCEPTION(std::runtime_error("TR4 Level: textiles32d is empty"));

    comp_size = m_reader.readU32();
    if(comp_size > 0)
    {
      if(!m_atlases.empty())
      {
        m_reader.skip(comp_size);
      }
      else
      {
        if(uncomp_size / (256 * 256 * 4) > 2)
          BOOST_LOG_TRIVIAL(warning) << "TR4 Level: number of misc textiles > 2";

        if(m_atlases.empty())
        {
          m_atlases.resize(numTextiles);
        }
        std::vector<uint8_t> comp_buffer(comp_size);

        m_reader.readBytes(comp_buffer.data(), comp_size);

        auto newsrc = io::SDLReader::decompress(comp_buffer, uncomp_size);
        newsrc.appendVector(m_atlases, numMiscTextiles, &DWordTexture::read);
      }
    }
  }

  const auto uncomp_size = m_reader.readU32();
  if(uncomp_size == 0)
    BOOST_THROW_EXCEPTION(std::runtime_error("TR4 Level: packed geometry (decompressed) is empty"));

  const auto comp_size = m_reader.readU32();

  if(comp_size == 0)
    BOOST_THROW_EXCEPTION(std::runtime_error("TR4 Level: packed geometry (compressed) is empty"));

  std::vector<uint8_t> comp_buffer(comp_size);
  m_reader.readBytes(comp_buffer.data(), comp_size);

  auto newsrc = io::SDLReader::decompress(comp_buffer, uncomp_size);
  if(!newsrc.isOpen())
    BOOST_THROW_EXCEPTION(std::runtime_error("TR4 Level: packed geometry could not be decompressed"));

  // Unused
  if(newsrc.readU32() != 0)
    BOOST_LOG_TRIVIAL(warning) << "TR4 Level: Bad value for 'unused'";

  newsrc.readVector(m_rooms, newsrc.readU16(), &Room::readTr4);

  newsrc.readVector(m_floorData, newsrc.readU32());

  readMeshData(newsrc);

  newsrc.readVector(m_animations, newsrc.readU32(), &Animation::readTr4);

  newsrc.readVector(m_transitions, newsrc.readU32(), &Transitions::read);

  newsrc.readVector(m_transitionCases, newsrc.readU32(), TransitionCase::read);

  newsrc.readVector(m_animCommands, newsrc.readU32());

  newsrc.readVector(m_boneTrees, newsrc.readU32());

  newsrc.readVector(m_poseFrames, newsrc.readU32());

  {
    const auto n = m_reader.readU32();
    for(uint32_t i = 0; i < n; ++i)
    {
      auto m = SkeletalModelType::readTr1(m_reader);
      if(m_animatedModels.find(m->type) != m_animatedModels.end())
        BOOST_THROW_EXCEPTION(std::runtime_error("Duplicate type id"));

      m_animatedModels[m->type] = std::move(m);
    }
  }

  newsrc.readVector(m_staticMeshes, newsrc.readU32(), &StaticMesh::read);

  if(newsrc.readI8() != 'S')
    BOOST_THROW_EXCEPTION(std::runtime_error("TR4 Level: 'SPR' not found"));

  if(newsrc.readI8() != 'P')
    BOOST_THROW_EXCEPTION(std::runtime_error("TR4 Level: 'SPR' not found"));

  if(newsrc.readI8() != 'R')
    BOOST_THROW_EXCEPTION(std::runtime_error("TR4 Level: 'SPR' not found"));

  newsrc.readVector(m_sprites, newsrc.readU32(), &Sprite::readTr4);

  {
    const auto n = m_reader.readU32();
    for(uint32_t i = 0; i < n; ++i)
    {
      auto m = SpriteSequence::read(m_reader);
      if(m_spriteSequences.find(m->type) != m_spriteSequences.end())
        BOOST_THROW_EXCEPTION(std::runtime_error("Duplicate type id"));

      m_spriteSequences[m->type] = std::move(m);
    }
  }

  newsrc.readVector(m_cameras, newsrc.readU32(), &Camera::read);

  newsrc.readVector(m_flybyCameras, newsrc.readU32(), &FlybyCamera::read);

  newsrc.readVector(m_soundSources, newsrc.readU32(), &SoundSource::read);

  newsrc.readVector(m_boxes, newsrc.readU32(), &Box::readTr2);

  newsrc.readVector(m_overlaps, newsrc.readU32());

  m_baseZones.read(m_boxes.size(), m_reader);
  m_alternateZones.read(m_boxes.size(), m_reader);

  newsrc.readVector(m_animatedTextures, newsrc.readU32());

  m_animatedTexturesUvCount = newsrc.readU8();

  if(newsrc.readI8() != 'T')
    BOOST_THROW_EXCEPTION(std::runtime_error("TR4 Level: 'TEX' not found"));

  if(newsrc.readI8() != 'E')
    BOOST_THROW_EXCEPTION(std::runtime_error("TR4 Level: 'TEX' not found"));

  if(newsrc.readI8() != 'X')
    BOOST_THROW_EXCEPTION(std::runtime_error("TR4 Level: 'TEX' not found"));

  newsrc.readVector(m_textureTiles, newsrc.readU32(), &TextureTile::readTr4);

  newsrc.readVector(m_items, newsrc.readU32(), &Item::readTr4);

  newsrc.readVector(m_aiObjects, newsrc.readU32(), &AIObject::read);

  newsrc.readVector(m_demoData, newsrc.readU16());

  // Soundmap
  newsrc.readVector(m_soundEffects, TR_AUDIO_MAP_SIZE_TR4);

  newsrc.readVector(m_soundEffectProperties, newsrc.readU32(), &SoundEffectProperties::readTr3);

  // IMPORTANT NOTE: Sample indices ARE NOT USED in TR4 engine, but are parsed anyway.
  newsrc.readVector(m_sampleIndices, newsrc.readU32());

  // LOAD SAMPLES

  if(m_reader.readU32() > 0)
  {
    // Since sample data is the last part, we simply load whole last
    // block of file as single array.
    m_reader.readVector(m_samplesData, static_cast<size_t>(m_reader.size() - m_reader.tell()));
  }

  if(!m_atlases.empty())
    return;

  m_atlases.resize(texture16.size());
  for(size_t i = 0; i < texture16.size(); i++)
    convertTexture(texture16[i], m_atlases[i]);
}
} // namespace loader::file::level
