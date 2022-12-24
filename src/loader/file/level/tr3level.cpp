#include "tr3level.h"

#include "core/id.h"
#include "loader/file/animation.h"
#include "loader/file/audio.h"
#include "loader/file/color.h"
#include "loader/file/datatypes.h"
#include "loader/file/item.h"
#include "loader/file/meshes.h"
#include "loader/file/texture.h"

#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace loader::file::level
{
#define TR_AUDIO_MAP_SIZE_TR3 370

void TR3Level::loadFileData()
{
  // Version
  const uint32_t file_version = m_reader.readU32();

  if(file_version != 0xFF080038 && file_version != 0xFF180038 && file_version != 0xFF180034)
    BOOST_THROW_EXCEPTION(std::runtime_error("TR3 Level: Wrong level version"));

  m_palette = Palette::readTr1(m_reader);
  /*Palette palette16 =*/Palette::readTr2(m_reader);

  const auto numTextiles = m_reader.readU32();
  std::vector<ByteTexture> texture8;
  m_reader.readVector(texture8, numTextiles, &ByteTexture::read);
  std::vector<WordTexture> texture16;
  m_reader.readVector(texture16, numTextiles, &WordTexture::read);

  if(file_version == 0xFF180034) // VICT.TR2
  {
    return; // Here only palette and textiles
  }

  // Unused
  if(m_reader.readU32() != 0)
    BOOST_LOG_TRIVIAL(warning) << "TR3 Level: Bad value for 'unused'";

  m_reader.readVector(m_rooms, m_reader.readU16(), &Room::readTr3);

  m_reader.readVector(m_floorData, m_reader.readU32());

  readMeshData(m_reader);

  m_reader.readVector(m_animations, m_reader.readU32(), &Animation::readTr1);

  m_reader.readVector(m_transitions, m_reader.readU32(), &Transitions::read);

  m_reader.readVector(m_transitionCases, m_reader.readU32(), &TransitionCase::read);

  m_reader.readVector(m_animCommands, m_reader.readU32());

  m_reader.readVector(m_boneTrees, m_reader.readU32());

  m_reader.readVector(m_poseFrames, m_reader.readU32());

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

  m_reader.readVector(m_staticMeshes, m_reader.readU32(), &StaticMesh::read);

  m_reader.readVector(m_sprites, m_reader.readU32(), &Sprite::readTr1);

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

  m_reader.readVector(m_cameras, m_reader.readU32(), &Camera::read);

  m_reader.readVector(m_soundSources, m_reader.readU32(), &SoundSource::read);

  m_reader.readVector(m_boxes, m_reader.readU32(), &Box::readTr2);

  m_reader.readVector(m_overlaps, m_reader.readU32());

  m_baseZones.read(m_boxes.size(), m_reader);
  m_alternateZones.read(m_boxes.size(), m_reader);

  m_animatedTexturesUvCount = 0; // No UVRotate in TR3
  m_reader.readVector(m_animatedTextures, m_reader.readU32());

  m_reader.readVector(m_textureTiles, m_reader.readU32(), &TextureTile::readTr1);

  m_reader.readVector(m_items, m_reader.readU32(), &Item::readTr3);

  m_lightmap = LightMap::read(m_reader);

  m_reader.readVector(m_cinematicFrames, m_reader.readU16(), &CinematicFrame::read);

  m_reader.readVector(m_demoData, m_reader.readU16());

  // Soundmap
  m_reader.readVector(m_soundEffects, TR_AUDIO_MAP_SIZE_TR3);

  m_reader.readVector(m_soundEffectProperties, m_reader.readU32(), &SoundEffectProperties::readTr3);

  m_reader.readVector(m_sampleIndices, m_reader.readU32());

  // remap all sample indices here
  for(auto& soundDetail : m_soundEffectProperties)
  {
    if(soundDetail.sample.get() < m_sampleIndices.size())
    {
      soundDetail.sample = m_sampleIndices[soundDetail.sample.get()];
    }
  }

  // LOAD SAMPLES

  // In TR3, samples are stored in separate file called MAIN.SFX.
  // If there is no such files, no samples are loaded.

  io::SDLReader newsrc(m_sfxPath);
  if(!newsrc.isOpen())
  {
    BOOST_LOG_TRIVIAL(warning) << "TR3 Level: failed to open '" << m_sfxPath << "', no samples loaded.";
  }
  else
  {
    m_reader.readVector(m_samplesData, newsrc.size());
  }

  m_atlases.resize(texture16.size());
  for(size_t i = 0; i < texture16.size(); i++)
    convertTexture(texture16[i], m_atlases[i]);
}
} // namespace loader::file::level
