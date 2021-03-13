#include "level.h"

#include "engine/objects/laraobject.h"
#include "render/textureanimator.h"
#include "tr1level.h"
#include "tr2level.h"
#include "tr3level.h"
#include "tr4level.h"
#include "tr5level.h"
#include "util/md5.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/range/adaptors.hpp>
#include <filesystem>

using namespace loader::file;
using namespace level;

Level::~Level() = default;

/// \brief reads the mesh data.
void Level::readMeshData(io::SDLReader& reader)
{
  const auto meshDataWords = reader.readU32();
  const auto basePos = reader.tell();

  const auto meshDataSize = meshDataWords * 2;
  reader.skip(meshDataSize);

  reader.readVector(m_meshIndices, reader.readU32());
  const auto endPos = reader.tell();

  m_meshes.clear();

  uint32_t meshDataPos = 0;
  for(uint32_t i = 0; i < m_meshIndices.size(); i++)
  {
    replace(m_meshIndices.begin(), m_meshIndices.end(), meshDataPos, i);

    reader.seek(basePos + std::streamoff(meshDataPos));

    if(gameToEngine(m_gameVersion) >= Engine::TR4)
      m_meshes.emplace_back(*Mesh::readTr4(reader));
    else
      m_meshes.emplace_back(*Mesh::readTr1(reader));

    auto it = std::find_if(
      m_meshIndices.begin(), m_meshIndices.end(), [meshDataPos](uint32_t pos) { return pos > meshDataPos; });
    if(it != m_meshIndices.end())
      meshDataPos = *it;
  }

  reader.seek(endPos);
}

std::unique_ptr<Level> Level::createLoader(const std::filesystem::path& filename, Game gameVersion)
{
  std::filesystem::path sfxPath = filename;
  sfxPath.replace_filename("MAIN.SFX");

  io::SDLReader reader{filename};
  if(!reader.isOpen())
    return nullptr;

  if(gameVersion == Game::Unknown)
    gameVersion = probeVersion(reader, filename);
  if(gameVersion == Game::Unknown)
    return nullptr;

  reader.seek(0);
  return createLoader(std::move(reader), filename, gameVersion, sfxPath);
}

std::unique_ptr<Level> Level::createLoader(io::SDLReader&& reader,
                                           std::filesystem::path filename,
                                           Game game_version,
                                           const std::filesystem::path& sfxPath)
{
  if(!reader.isOpen())
    return nullptr;

  std::unique_ptr<Level> result;

  switch(game_version)
  {
  case Game::TR1: result = std::make_unique<TR1Level>(game_version, std::move(reader), filename); break;
  case Game::TR1Demo:
  case Game::TR1UnfinishedBusiness:
    result = std::make_unique<TR1Level>(game_version, std::move(reader), filename);
    result->m_demoOrUb = true;
    break;
  case Game::TR2: result = std::make_unique<TR2Level>(game_version, std::move(reader), filename); break;
  case Game::TR2Demo:
    result = std::make_unique<TR2Level>(game_version, std::move(reader), filename);
    result->m_demoOrUb = true;
    break;
  case Game::TR3: result = std::make_unique<TR3Level>(game_version, std::move(reader), filename); break;
  case Game::TR4:
  case Game::TR4Demo: result = std::make_unique<TR4Level>(game_version, std::move(reader), filename); break;
  case Game::TR5: result = std::make_unique<TR5Level>(game_version, std::move(reader), filename); break;
  default: BOOST_THROW_EXCEPTION(std::runtime_error("Invalid game version"));
  }

  result->m_sfxPath = sfxPath;
  return result;
}

Game Level::probeVersion(io::SDLReader& reader, const std::filesystem::path& filename)
{
  if(!reader.isOpen() || !std::filesystem::is_regular_file(filename))
    return Game::Unknown;

  const std::string ext = boost::algorithm::to_upper_copy(filename.extension().string());

  reader.seek(0);
  std::array<uint8_t, 4> check{};
  reader.readBytes(check.data(), check.size());

  Game ret = Game::Unknown;
  if(ext == ".PHD")
  {
    if(check[0] == 0x20 && check[1] == 0x00 && check[2] == 0x00 && check[3] == 0x00)
    {
      ret = Game::TR1;
    }
  }
  else if(ext == ".TUB")
  {
    if(check[0] == 0x20 && check[1] == 0x00 && check[2] == 0x00 && check[3] == 0x00)
    {
      ret = Game::TR1UnfinishedBusiness;
    }
  }
  else if(ext == ".TR2")
  {
    if(check[0] == 0x2D && check[1] == 0x00 && check[2] == 0x00 && check[3] == 0x00)
    {
      ret = Game::TR2;
    }
    else if((check[0] == 0x38 || check[0] == 0x34) && check[1] == 0x00 && (check[2] == 0x18 || check[2] == 0x08)
            && check[3] == 0xFF)
    {
      ret = Game::TR3;
    }
  }
  else if(ext == ".TR4")
  {
    if(check[0] == 0x54 && // T
       check[1] == 0x52 && // R
       check[2] == 0x34 && // 4
       check[3] == 0x00)
    { // NOLINT(bugprone-branch-clone)
      ret = Game::TR4;
    }
    else if(check[0] == 0x54 && // T
            check[1] == 0x52 && // R
            check[2] == 0x34 && // 4
            check[3] == 0x63)   //
    {
      ret = Game::TR4;
    }
    else if(check[0] == 0xF0 && check[1] == 0xFF && check[2] == 0xFF && check[3] == 0xFF)
    {
      ret = Game::TR4;
    }
  }
  else if(ext == ".TRC")
  {
    if(check[0] == 0x54 && // T
       check[1] == 0x52 && // R
       check[2] == 0x43 && // C
       check[3] == 0x00)
    {
      ret = Game::TR5;
    }
  }

  return ret;
}

const StaticMesh* Level::findStaticMeshById(const core::StaticMeshId meshId) const
{
  auto it = std::find_if(
    m_staticMeshes.begin(), m_staticMeshes.end(), [meshId](const auto& mesh) { return mesh.id == meshId; });
  if(it != m_staticMeshes.end())
    return &*it;

  return nullptr;
}

std::shared_ptr<render::scene::Mesh> Level::findStaticRenderMeshById(const core::StaticMeshId meshId) const
{
  auto it = std::find_if(m_staticMeshes.begin(), m_staticMeshes.end(), [meshId](const auto& mesh) {
    return mesh.isVisible() && mesh.id == meshId;
  });
  if(it != m_staticMeshes.end())
    return it->renderMesh;

  return nullptr;
}

const std::unique_ptr<SkeletalModelType>& Level::findAnimatedModelForType(const core::TypeId type) const
{
  const auto it = m_animatedModels.find(type);
  if(it != m_animatedModels.end())
    return it->second;

  static const std::unique_ptr<SkeletalModelType> none;
  return none;
}

const std::unique_ptr<SpriteSequence>& Level::findSpriteSequenceForType(const core::TypeId type) const
{
  const auto it = m_spriteSequences.find(type);
  if(it != m_spriteSequences.end())
    return it->second;

  static const std::unique_ptr<SpriteSequence> none;
  return none;
}

void Level::convertTexture(ByteTexture& tex, Palette& pal, DWordTexture& dst)
{
  for(int y = 0; y < 256; y++)
  {
    for(int x = 0; x < 256; x++)
    {
      const auto col = tex.pixels[y][x];

      if(col > 0)
        dst.pixels[y][x] = {pal.colors[col].r, pal.colors[col].g, pal.colors[col].b, 255};
      else
        dst.pixels[y][x] = {0, 0, 0, 0};
    }
  }

  dst.md5 = util::md5(&tex.pixels[0][0], 256 * 256);
}

void Level::convertTexture(WordTexture& tex, DWordTexture& dst)
{
  for(int y = 0; y < 256; y++)
  {
    for(int x = 0; x < 256; x++)
    {
      const auto col = tex.pixels[y][x];

      if((col & 0x8000u) != 0)
      {
        const auto r = static_cast<uint8_t>((col & 0x00007c00u) >> 7u);
        const auto g = static_cast<uint8_t>((col & 0x000003e0u) >> 2u);
        const auto b = static_cast<uint8_t>((col & 0x0000001fu) << 3u);
        dst.pixels[y][x] = {r, g, b, 1};
      }
      else
      {
        dst.pixels[y][x] = {0, 0, 0, 0};
      }
    }
  }
}

void Level::connectSectors()
{
  for(Room& room : m_rooms)
  {
    for(auto& sector : room.typedSectors)
      sector.connect(m_rooms);
  }
}

void Level::postProcessDataStructures()
{
  BOOST_LOG_TRIVIAL(info) << "Post-processing data structures";

  m_typedAnimations.resize(m_animations.size());
  m_typedTransitions.resize(m_transitions.size());
  for(size_t i = 0; i < m_animations.size(); ++i)
  {
    const auto& anim = m_animations[i];
    const AnimFrame* frames = nullptr;

    if(anim.poseDataOffset.index<decltype(m_poseFrames[0])>() >= m_poseFrames.size())
    {
      BOOST_LOG_TRIVIAL(warning) << "Pose frame data index " << anim.poseDataOffset.index<decltype(m_poseFrames[0])>()
                                 << " out of range 0.." << m_poseFrames.size() - 1;
    }
    else
    {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      frames = reinterpret_cast<const AnimFrame*>(&anim.poseDataOffset.from(m_poseFrames));
    }

    Expects(anim.nextAnimationIndex < m_typedAnimations.size());
    auto nextAnimation = &m_typedAnimations[anim.nextAnimationIndex];

    Expects((anim.animCommandIndex + anim.animCommandCount).exclusiveIn(m_animCommands));
    Expects((anim.transitionsIndex + anim.transitionsCount).exclusiveIn(m_typedTransitions));
    gsl::span<const TypedTransitions> transitions;
    if(anim.transitionsCount > 0)
      transitions = gsl::span{&anim.transitionsIndex.from(m_typedTransitions), anim.transitionsCount};

    m_typedAnimations[i]
      = engine::world::Animation{frames,
                                 anim.segmentLength,
                                 anim.state_id,
                                 anim.speed,
                                 anim.acceleration,
                                 anim.firstFrame,
                                 anim.lastFrame,
                                 anim.nextFrame,
                                 anim.animCommandCount,
                                 anim.animCommandCount == 0 ? nullptr : &anim.animCommandIndex.from(m_animCommands),
                                 nextAnimation,
                                 std::move(transitions)};
  }

  for(const std::unique_ptr<SkeletalModelType>& model : m_animatedModels | boost::adaptors::map_values)
  {
    if(model->pose_data_offset.index<decltype(m_poseFrames[0])>() >= m_poseFrames.size())
    {
      BOOST_LOG_TRIVIAL(warning) << "Pose frame data index "
                                 << model->pose_data_offset.index<decltype(m_poseFrames[0])>() << " out of range 0.."
                                 << m_poseFrames.size() - 1;
      continue;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    model->frames = reinterpret_cast<const AnimFrame*>(&model->pose_data_offset.from(m_poseFrames));
    if(model->nMeshes > 1)
    {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      model->boneTree = gsl::make_span(reinterpret_cast<const BoneTreeEntry*>(&model->bone_index.from(m_boneTrees)),
                                       model->nMeshes - 1);
    }

    if(model->animation_index.index != 0xffff)
      model->animations = &model->animation_index.from(m_typedAnimations);
  }

  for(TransitionCase& transitionCase : m_transitionCases)
  {
    const engine::world::Animation* anim = nullptr;
    if(transitionCase.targetAnimationIndex.index < m_typedAnimations.size())
      anim = &transitionCase.targetAnimationIndex.from(m_typedAnimations);
    else
      BOOST_LOG_TRIVIAL(warning) << "Animation index " << transitionCase.targetAnimationIndex.index << " not less than "
                                 << m_typedAnimations.size();

    m_typedTransitionCases.emplace_back(
      TypedTransitionCase{transitionCase.firstFrame, transitionCase.lastFrame, transitionCase.targetFrame, anim});
  }

  Expects(m_transitions.size() == m_typedTransitions.size());
  std::transform(
    m_transitions.begin(), m_transitions.end(), m_typedTransitions.begin(), [this](const Transitions& transitions) {
      Expects((transitions.firstTransitionCase + transitions.transitionCaseCount).exclusiveIn(m_typedTransitionCases));
      if(transitions.transitionCaseCount > 0)
        return TypedTransitions{
          transitions.stateId,
          gsl::span{&transitions.firstTransitionCase.from(m_typedTransitionCases), transitions.transitionCaseCount}};
      return TypedTransitions{};
    });

  for(const auto& sequence : m_spriteSequences | boost::adaptors::map_values)
  {
    Expects(sequence != nullptr);
    Expects(sequence->length <= 0);
    Expects(gsl::narrow<size_t>(sequence->offset - sequence->length) <= m_sprites.size());
    sequence->sprites = gsl::make_span(&m_sprites[sequence->offset], -sequence->length);
  }

  m_typedBoxes.resize(m_boxes.size());
  auto getOverlaps = [this](const uint16_t idx) {
    std::vector<gsl::not_null<engine::world::Box*>> result;
    const auto first = &m_overlaps.at(idx);
    auto current = first;
    const auto endOfUniverse = &m_overlaps.back() + 1;

    while(current < endOfUniverse && (*current & 0x8000u) == 0)
    {
      result.emplace_back(&m_typedBoxes.at(*current));
      ++current;
    }
    result.emplace_back(&m_typedBoxes.at(*current & 0x7FFFu));

    return result;
  };

  std::transform(m_boxes.begin(), m_boxes.end(), m_typedBoxes.begin(), [&getOverlaps](const Box& box) {
    return engine::world::Box{
      box.zmin, box.zmax, box.xmin, box.xmax, box.floor, box.blocked, box.blockable, getOverlaps(box.overlap_index)};
  });

  Expects(m_baseZones.flyZone.size() == m_typedBoxes.size());
  Expects(m_baseZones.groundZone1.size() == m_typedBoxes.size());
  Expects(m_baseZones.groundZone2.size() == m_typedBoxes.size());
  Expects(m_alternateZones.flyZone.size() == m_typedBoxes.size());
  Expects(m_alternateZones.groundZone1.size() == m_typedBoxes.size());
  Expects(m_alternateZones.groundZone2.size() == m_typedBoxes.size());
  for(size_t i = 0; i < m_typedBoxes.size(); ++i)
  {
    m_typedBoxes[i].zoneFly = m_baseZones.flyZone[i];
    m_typedBoxes[i].zoneGround1 = m_baseZones.groundZone1[i];
    m_typedBoxes[i].zoneGround2 = m_baseZones.groundZone2[i];
    m_typedBoxes[i].zoneFlySwapped = m_alternateZones.flyZone[i];
    m_typedBoxes[i].zoneGround1Swapped = m_alternateZones.groundZone1[i];
    m_typedBoxes[i].zoneGround2Swapped = m_alternateZones.groundZone2[i];
  }

  for(auto& room : m_rooms)
  {
    room.typedSectors.clear();
    std::transform(
      room.sectors.begin(), room.sectors.end(), std::back_inserter(room.typedSectors), [this](const Sector& sector) {
        return engine::world::Sector{sector, m_rooms, m_typedBoxes, m_floorData};
      });
  }

  Ensures(m_typedAnimations.size() == m_animations.size());
  Ensures(m_typedTransitionCases.size() == m_transitionCases.size());
  Ensures(m_typedTransitions.size() == m_transitions.size());
  Ensures(m_typedBoxes.size() == m_boxes.size());

  connectSectors();
}
