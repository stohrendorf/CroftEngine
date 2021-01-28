#pragma once

#include "engine/objects/object.h"
#include "engine/particle.h"
#include "game.h"
#include "loader/file/animation.h"
#include "loader/file/datatypes.h"
#include "loader/file/io/sdlreader.h"
#include "loader/file/item.h"
#include "loader/file/mesh.h"

#include <memory>
#include <utility>
#include <vector>

namespace loader
{
namespace trx
{
class Glidos;
}

namespace file::level
{
class Level
{
public:
  Level(const Game gameVersion, io::SDLReader&& reader, std::filesystem::path filename)
      : m_gameVersion{gameVersion}
      , m_reader{std::move(reader)}
      , m_filename{std::move(filename)}
  {
  }

  Level(const Level&) = delete;

  Level(Level&&) = delete;

  Level& operator=(const Level&) = delete;

  Level& operator=(Level&&) = delete;

  virtual ~Level();

  const Game m_gameVersion;

  std::vector<DWordTexture> m_textures;

  std::unique_ptr<Palette> m_palette;

  std::vector<Room> m_rooms;

  std::vector<uint32_t> m_meshIndices;

  std::vector<Animation> m_animations;

  std::vector<Transitions> m_transitions;

  std::vector<TransitionCase> m_transitionCases;

  std::vector<int16_t> m_animCommands;

  std::map<core::TypeId, std::unique_ptr<SkeletalModelType>> m_animatedModels;

  std::vector<TextureTile> m_textureTiles;

  size_t m_animatedTexturesUvCount = 0;

  std::vector<Sprite> m_sprites;

  std::map<core::TypeId, std::unique_ptr<SpriteSequence>> m_spriteSequences;

  std::vector<Camera> m_cameras;

  std::vector<FlybyCamera> m_flybyCameras;

  std::vector<SoundSource> m_soundSources;

  std::vector<Box> m_boxes;

  std::vector<uint16_t> m_overlaps;

  Zones m_baseZones;

  Zones m_alternateZones;

  std::vector<Item> m_items;

  std::unique_ptr<LightMap> m_lightmap;

  std::vector<AIObject> m_aiObjects;

  std::vector<CinematicFrame> m_cinematicFrames;

  std::vector<uint8_t> m_demoData;

  std::vector<int16_t> m_soundEffects;

  std::vector<SoundEffectProperties> m_soundEffectProperties;

  size_t m_samplesCount = 0;

  std::vector<uint8_t> m_samplesData;

  std::vector<uint32_t> m_sampleIndices;

  std::vector<int16_t> m_poseFrames;

  std::vector<int32_t> m_boneTrees;

  std::filesystem::path m_sfxPath = "MAIN.SFX";

  engine::floordata::FloorData m_floorData;
  std::vector<Mesh> m_meshes;
  std::vector<StaticMesh> m_staticMeshes;
  std::vector<uint16_t> m_animatedTextures;

  /*
     * 0 Normal
     * 3 Catsuit
     * 4 Divesuit
     * 6 Invisible
     */
  uint16_t m_laraType = 0;

  /*
     * 0 No weather
     * 1 Rain
     * 2 Snow (in title.trc these are red triangles falling from the sky).
     */
  uint16_t m_weatherType = 0;

  static std::unique_ptr<Level> createLoader(const std::filesystem::path& filename, Game gameVersion);

  virtual void loadFileData() = 0;

  [[nodiscard]] const StaticMesh* findStaticMeshById(core::StaticMeshId meshId) const;

  [[nodiscard]] std::shared_ptr<render::scene::Mesh> findStaticRenderMeshById(core::StaticMeshId meshId) const;

  [[nodiscard]] const std::unique_ptr<SkeletalModelType>& findAnimatedModelForType(core::TypeId type) const;

  [[nodiscard]] const std::unique_ptr<SpriteSequence>& findSpriteSequenceForType(core::TypeId type) const;

  void updateRoomBasedCaches();

  const auto& getFilename() const
  {
    return m_filename;
  }

protected:
  io::SDLReader m_reader;

  bool m_demoOrUb = false;

  void readMeshData(io::SDLReader& reader);

  static void convertTexture(ByteTexture& tex, Palette& pal, DWordTexture& dst);

  static void convertTexture(WordTexture& tex, DWordTexture& dst);

  void postProcessDataStructures();

private:
  const std::filesystem::path m_filename;

  static Game probeVersion(io::SDLReader& reader, const std::filesystem::path& filename);

  static std::unique_ptr<Level> createLoader(io::SDLReader&& reader,
                                             std::filesystem::path filename,
                                             Game game_version,
                                             const std::filesystem::path& sfxPath);
};
} // namespace file::level
} // namespace loader
