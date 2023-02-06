#include "texturing.h"

#include "atlastile.h"
#include "blockdata.hpp"
#include "core/i18n.h"
#include "core/id.h"
#include "loader/file/datatypes.h"
#include "loader/file/level/level.h"
#include "loader/file/texture.h"
#include "loader/trx/trx.h"
#include "render/textureatlas.h"
#include "serialization/glm.h"
#include "serialization/map.h"
#include "serialization/path.h"
#include "serialization/serialization.h"
#include "serialization/yamldocument.h"
#include "sprite.h"

#include <algorithm>
#include <array>
#include <boost/assert.hpp>
#include <boost/log/trivial.hpp>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <gl/cimgwrapper.h>
#include <gl/image.h>
#include <gl/pixel.h>
#include <gl/texture2darray.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <gsl/gsl-lite.hpp>
#include <iosfwd>
#include <map>
#include <memory>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

namespace engine::world
{
namespace
{
void remapRange(glm::vec2& co,
                const glm::vec2& rangeAMin,
                const glm::vec2& rangeAMax,
                const glm::vec2& rangeBMin,
                const glm::vec2& rangeBMax)
{
  co -= rangeAMin;
  co /= rangeAMax - rangeAMin;
  co *= rangeBMax - rangeBMin;
  co += rangeBMin;
  BOOST_ASSERT(co.x >= 0 && co.x <= 1);
  BOOST_ASSERT(co.y >= 0 && co.y <= 1);
}

void remap(AtlasTile& srcTile, const core::AtlasId& newAtlasId, const glm::vec2& dstUV0, const glm::vec2& dstUV1)
{
  srcTile.textureKey.atlasIdAndFlag &= ~loader::file::AtlasIdMask;
  srcTile.textureKey.atlasIdAndFlag |= newAtlasId.get();

  const auto [tileUvMin, tileUvMax] = srcTile.getMinMaxUv();
  const auto tileUvSize = tileUvMax - tileUvMin;
  if(tileUvSize.x == 0 || tileUvSize.y == 0)
    return;

  for(auto& uvComponent : srcTile.uvCoordinates)
  {
    if(uvComponent.x == 0 && uvComponent.y == 0)
      continue;

    remapRange(uvComponent, tileUvMin, tileUvMax, dstUV0, dstUV1);
  }
}

void remap(Sprite& srcSprite, const core::AtlasId& newAtlasId, const glm::vec2& dstUV0, const glm::vec2& dstUV1)
{
  srcSprite.atlasId = newAtlasId;

  // re-map uv coordinates
  const auto a = glm::round(srcSprite.uv0 * 256.0f) / 256.0f;
  const auto b = glm::round(srcSprite.uv1 * 256.0f) / 256.0f;

  remapRange(srcSprite.uv0, a, b, dstUV0, dstUV1);
  remapRange(srcSprite.uv1, a, b, dstUV0, dstUV1);
}

struct Tile
{
  glm::ivec2 position;
  glm::ivec2 size;

  // texture page, position, and size (in pixels)
  std::optional<std::tuple<size_t, glm::ivec2, glm::ivec2>> remapped{};

  explicit Tile(const glm::ivec2& position, const glm::ivec2& size)
      : position{size.x >= 0 ? position.x : position.x + size.x, size.y >= 0 ? position.y : position.y + size.y}
      , size{glm::abs(size)}
  {
  }

  [[nodiscard]] constexpr auto getXY1() const
  {
    return position + size;
  }

  [[nodiscard]] constexpr auto getRight() const
  {
    return position.x + size.x;
  }

  [[nodiscard]] constexpr auto getBottom() const
  {
    return position.y + size.y;
  }

  [[nodiscard]] constexpr bool contains(const Tile& other) const
  {
    return position.x <= other.position.x && position.y <= other.position.y && getRight() >= other.getRight()
           && getBottom() >= other.getBottom();
  }

  [[nodiscard]] std::pair<glm::vec2, glm::vec2> getRemappedPos(const glm::ivec2& srcPos,
                                                               const glm::ivec2& srcSize) const
  {
    const auto remapScale = getRemapScale();
    const auto remappedPos = glm::vec2{std::get<1>(*remapped)} + glm::vec2{srcPos - position} * remapScale;
    const auto remappedSize = glm::vec2{srcSize} * remapScale - glm::vec2{1, 1};

    return {remappedPos, remappedPos + remappedSize};
  }

  [[nodiscard]] glm::vec2 getRemapScale() const
  {
    gsl_Assert(remapped.has_value());
    return glm::vec2{std::get<2>(*remapped)} / glm::vec2{size};
  }
};

std::vector<std::vector<Tile>> getMaximizedAtlasesTiles(const loader::file::level::Level& level)
{
  BOOST_LOG_TRIVIAL(debug) << "Gathering texture tiles";
  std::vector<std::vector<Tile>> atlasesTiles;
  atlasesTiles.resize(level.m_atlases.size());

  for(const auto& objTex : level.m_textureTiles)
  {
    auto minU = std::numeric_limits<float>::max();
    auto maxU = std::numeric_limits<float>::min();
    auto minV = std::numeric_limits<float>::max();
    auto maxV = std::numeric_limits<float>::min();
    for(const auto& co : objTex.uvCoordinates)
    {
      if(co.isUnset())
        continue;

      const auto glCo = co.toGl();
      minU = std::min(minU, glCo.x);
      minV = std::min(minV, glCo.y);
      maxU = std::max(maxU, glCo.x);
      maxV = std::max(maxV, glCo.y);
    }

    if(minU >= maxU || minV >= maxV)
      continue;

    auto& atlasTiles = atlasesTiles.at(objTex.textureKey.atlasIdAndFlag & loader::file::AtlasIdMask);
    atlasTiles.emplace_back(glm::ivec2{std::lround(minU * 256.0f), std::lround(minV * 256.0f)},
                            glm::ivec2{std::lround((maxU - minU) * 256.0f), std::lround((maxV - minV) * 256.0f)});
  }

  for(const auto& sprTex : level.m_sprites)
  {
    const auto uv0 = sprTex.uv0.toGl();
    const auto uv1 = sprTex.uv1.toGl();
    const auto minU = std::min(uv0.x, uv1.x);
    const auto maxU = std::max(uv0.x, uv1.x);
    const auto minV = std::min(uv0.y, uv1.y);
    const auto maxV = std::max(uv0.y, uv1.y);

    auto& atlasTiles = atlasesTiles.at(sprTex.atlas_id.get());
    atlasTiles.emplace_back(glm::ivec2{std::lround(minU * 256.0f), std::lround(minV * 256.0f)},
                            glm::ivec2{std::lround((maxU - minU) * 256.0f), std::lround((maxV - minV) * 256.0f)});
  }

  // remove every tile that's contained within another one
  for(auto& atlasTiles : atlasesTiles)
  {
    for(size_t i = 0; i < atlasTiles.size() - 1; ++i)
    {
      bool iRemoved = false;
      for(size_t j = i + 1; j < atlasTiles.size();)
      {
        if(atlasTiles[i].contains(atlasTiles[j]))
        {
          atlasTiles.erase(std::next(atlasTiles.begin(), j));
        }
        else if(atlasTiles[j].contains(atlasTiles[i]))
        {
          atlasTiles.erase(std::next(atlasTiles.begin(), i));
          iRemoved = true;
        }
        else
        {
          ++j;
        }
      }

      if(iRemoved)
      {
        --i;
      }
    }

    atlasTiles.shrink_to_fit();
  }

  return atlasesTiles;
}

bool tryRemapTile(Tile& maximizedTile,
                  const loader::trx::Rectangle& glidosTile,
                  render::MultiTextureAtlas& atlases,
                  std::map<std::filesystem::path, glm::ivec2>& textureSizes,
                  const std::filesystem::path& path,
                  const std::shared_ptr<gl::Image<gl::SRGBA8>>& textureImage)
{
  if(glidosTile.getXY0() != maximizedTile.position || glidosTile.getXY1() != maximizedTile.getXY1())
  {
    return false;
  }

  if(maximizedTile.remapped.has_value())
  {
    return true;
  }

  const bool hasExternalTexture = !path.empty() && std::filesystem::is_regular_file(path);

  if(atlases.isOnlyLayout())
  {
    if(!hasExternalTexture)
    {
      const auto remapped = atlases.put(maximizedTile.size);
      maximizedTile.remapped = {remapped.first, remapped.second, maximizedTile.size};
    }
    else
    {
      const auto& size = textureSizes.at(path);
      const auto remapped = atlases.put(size);
      maximizedTile.remapped = {remapped.first, remapped.second, size};
    }
  }
  else
  {
    if(!hasExternalTexture)
    {
      gl::CImgWrapper replacementImg(
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<const uint8_t*>(textureImage->getRawData()),
        256,
        256,
        true);
      replacementImg.crop(
        maximizedTile.position.x, maximizedTile.position.y, maximizedTile.getXY1().x - 1, maximizedTile.getXY1().y - 1);
      const auto remapped = atlases.put(replacementImg);
      maximizedTile.remapped = {remapped.first, remapped.second, maximizedTile.size};
    }
    else
    {
      gl::CImgWrapper tmp{path};
      const auto remapped = atlases.put(tmp);
      maximizedTile.remapped = {remapped.first, remapped.second, {tmp.width(), tmp.height()}};
      textureSizes.emplace(path, glm::ivec2{tmp.width(), tmp.height()});
    }
  }

  return true;
}

// draws 0..5% progress
void layoutAtlases(const loader::file::level::Level& level,
                   loader::trx::Glidos& glidos,
                   render::MultiTextureAtlas& atlases,
                   std::vector<AtlasTile>& atlasTiles,
                   std::vector<Sprite>& sprites,
                   std::unordered_set<AtlasTile*>& doneTiles,
                   std::unordered_set<Sprite*>& doneSprites,
                   std::map<std::filesystem::path, glm::ivec2>& textureSizes,
                   const std::function<void(const std::string&)>& drawLoadingScreen)
{
  auto maximizedAtlasesTiles = getMaximizedAtlasesTiles(level);
  gsl_Assert(maximizedAtlasesTiles.size() == level.m_atlases.size());

  BOOST_LOG_TRIVIAL(debug) << "Layouting texture atlases";
  for(size_t atlasId = 0; atlasId < level.m_atlases.size(); ++atlasId)
  {
    drawLoadingScreen(_("Building atlases (%1%%%)", atlasId * 5 / level.m_atlases.size()));

    const auto& atlas = level.m_atlases[atlasId];
    BOOST_LOG_TRIVIAL(debug) << "Layouting atlas " << atlas.md5;
    auto& maximizedAtlasTiles = maximizedAtlasesTiles[atlasId];
    auto mappings = glidos.getMappingsForTexture(atlas.md5);
    BOOST_LOG_TRIVIAL(debug) << "Layouting atlas " << atlas.md5 << ", " << mappings.size() << " possible mappings";
    for(auto& maximizedTile : maximizedAtlasTiles)
    {
      bool found = false;
      for(const auto& [tile, path] : mappings)
      {
        if(tryRemapTile(maximizedTile, tile, atlases, textureSizes, path, atlas.image))
        {
          found = true;
          break;
        }
      }

      if(found)
        continue;

      const auto mappedRect = loader::trx::Rectangle{maximizedTile.position, maximizedTile.getXY1()};
      glidos.insertInternalMapping(atlas.md5, mappedRect);
      gsl_Assert(tryRemapTile(maximizedTile, mappedRect, atlases, textureSizes, {}, atlas.image));
    }
  }

  for(size_t atlasId = 0; atlasId < level.m_atlases.size(); ++atlasId)
  {
    const auto& maximizedAtlasTiles = maximizedAtlasesTiles[atlasId];

    // find the containing maximized tile

    bool remapped = false;
    for(auto& srcTile : atlasTiles)
    {
      if(doneTiles.count(&srcTile) != 0)
        continue;

      if((srcTile.textureKey.atlasIdAndFlag & loader::file::AtlasIdMask) != atlasId)
        continue;

      const auto [minUv, maxUv] = srcTile.getMinMaxUv();
      const auto pos = glm::ivec2{std::lround(minUv.x * 256.0f), std::lround(minUv.y * 256.0f)};
      const auto size = glm::ivec2{std::lround(maxUv.x * 256.0f), std::lround(maxUv.y * 256.0f)} - pos;
      const auto needleTile = Tile{pos, size};

      const Tile* matchingTile = nullptr;
      for(const auto& maximizedTile : maximizedAtlasTiles)
      {
        gsl_Assert(maximizedTile.remapped.has_value());

        if(!maximizedTile.contains(needleTile))
          continue;

        matchingTile = &maximizedTile;
        break;
      }

      gsl_Assert(matchingTile != nullptr);
      const auto page = std::get<0>(*matchingTile->remapped);
      const auto [remappedPos1, remappedPos2] = matchingTile->getRemappedPos(pos, size);
      const auto remappedPos1Uv = glm::vec2{remappedPos1} / static_cast<float>(atlases.getSize() - 1);
      const auto remappedPos2Uv = glm::vec2{remappedPos2} / static_cast<float>(atlases.getSize() - 1);

      doneTiles.emplace(&srcTile);
      remapped = true;
      remap(srcTile, page, remappedPos1Uv, remappedPos2Uv);
    }

    for(auto& sprite : sprites)
    {
      if(doneSprites.count(&sprite) != 0)
        continue;

      if(sprite.atlasId.get() != atlasId)
        continue;

      const auto a = glm::ivec2{std::lround(sprite.uv0.x * 256.0f), std::lround(sprite.uv0.y * 256.0f)};
      const auto b = glm::ivec2{std::lround(sprite.uv1.x * 256.0f), std::lround(sprite.uv1.y * 256.0f)};

      const auto needleTile = Tile{a, b - a};

      const Tile* matchingTile = nullptr;
      for(const auto& maximizedTile : maximizedAtlasTiles)
      {
        if(!maximizedTile.remapped.has_value())
          continue;

        if(!maximizedTile.contains(needleTile))
          continue;

        matchingTile = &maximizedTile;
        break;
      }

      gsl_Assert(matchingTile != nullptr);
      const auto page = std::get<0>(*matchingTile->remapped);
      const auto [remappedPos1, remappedPos2] = matchingTile->getRemappedPos(a, b - a);
      const auto remappedPos1Uv = glm::vec2{remappedPos1} / static_cast<float>(atlases.getSize() - 1);
      const auto remappedPos2Uv = glm::vec2{remappedPos2} / static_cast<float>(atlases.getSize() - 1);

      doneSprites.emplace(&sprite);
      remapped = true;
      remap(sprite, page, remappedPos1Uv, remappedPos2Uv);
    }

    if(!remapped)
    {
      BOOST_LOG_TRIVIAL(error) << "Failed to re-map texture tile";
    }
  }

  BOOST_LOG_TRIVIAL(debug) << "Re-mapped " << doneTiles.size() << " tiles and " << doneSprites.size() << " sprites";
}

// draws 5..75% progress
void materializeAtlases(const loader::file::level::Level& level,
                        render::MultiTextureAtlas& atlases,
                        std::vector<AtlasTile>& atlasTiles,
                        std::vector<Sprite>& sprites,
                        std::unordered_set<AtlasTile*>& doneTiles,
                        std::unordered_set<Sprite*>& doneSprites,
                        const std::function<void(const std::string&)>& drawLoadingScreen)
{
  const auto atlasUvScale = 256.0f / gsl::narrow_cast<float>(atlases.getSize());

  struct SourceTile final
  {
    core::AtlasId atlasId;
    std::pair<glm::ivec2, glm::ivec2> px;

    bool operator<(const SourceTile& rhs) const noexcept
    {
      if(atlasId != rhs.atlasId)
        return atlasId < rhs.atlasId;

      if(px.first.x != rhs.px.first.x)
        return px.first.x < rhs.px.first.x;
      if(px.first.y != rhs.px.first.y)
        return px.first.y < rhs.px.first.y;
      if(px.second.x != rhs.px.second.x)
        return px.second.x < rhs.px.second.x;
      return px.second.y < rhs.px.second.y;
    }

    bool operator==(const SourceTile& rhs) const noexcept
    {
      return atlasId == rhs.atlasId && px == rhs.px;
    }
  };

  std::map<SourceTile, std::pair<core::AtlasId, glm::ivec2>> replaced;

  std::vector<AtlasTile*> tilesOrderedBySize;
  tilesOrderedBySize.reserve(atlasTiles.size());
  for(auto& tile : atlasTiles)
    tilesOrderedBySize.emplace_back(&tile);

  std::sort(tilesOrderedBySize.begin(),
            tilesOrderedBySize.end(),
            [](AtlasTile* a, AtlasTile* b)
            {
              return a->getArea() > b->getArea();
            });

  for(size_t tileId = 0; tileId < tilesOrderedBySize.size(); ++tileId)
  {
    drawLoadingScreen(_("Building atlases (%1%%%)", 5 + tileId * 70 / tilesOrderedBySize.size()));

    auto* const tile = tilesOrderedBySize[tileId];
    if(!doneTiles.emplace(tile).second)
      continue;
    auto atlasId = tile->textureKey.atlasIdAndFlag & loader::file::AtlasIdMask;
    const auto [srcMinUv, srcMaxUv] = tile->getMinMaxUv();
    const auto srcMinPx = glm::ivec2{srcMinUv * 256.0f};
    const auto srcMaxPx = glm::ivec2{srcMaxUv * 256.0f};
    const SourceTile srcTile{atlasId, {srcMinPx, srcMaxPx}};
    std::pair<core::AtlasId, glm::ivec2> replacementPos{{0}, {0, 0}};
    if(auto it = replaced.find(srcTile); it != replaced.end())
    {
      replacementPos = it->second;
    }
    else
    {
      const auto& atlas = level.m_atlases.at(atlasId);
      if(!atlases.isOnlyLayout())
      {
        gl::CImgWrapper replacementImg(
          // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
          reinterpret_cast<const uint8_t*>(atlas.image->getRawData()),
          256,
          256,
          true);
        replacementImg.crop(srcMinPx.x, srcMinPx.y, srcMaxPx.x, srcMaxPx.y);
        replacementPos = atlases.put(replacementImg);
      }
      else
      {
        replacementPos = atlases.put((srcMaxPx - srcMinPx) + glm::ivec2{1, 1});
      }
      replaced.emplace(srcTile, replacementPos);
    }

    const auto srcUvDims = tile->getMinMaxUv();
    const auto replacementUvPos = glm::vec2{replacementPos.second} / gsl::narrow_cast<float>(atlases.getSize());
    remap(*tile,
          replacementPos.first,
          replacementUvPos,
          replacementUvPos + (srcUvDims.second - srcUvDims.first) * atlasUvScale);
  }

  std::vector<Sprite*> spritesOrderedBySize;
  spritesOrderedBySize.reserve(sprites.size());
  for(auto& sprite : sprites)
    spritesOrderedBySize.emplace_back(&sprite);

  std::sort(spritesOrderedBySize.begin(),
            spritesOrderedBySize.end(),
            [](Sprite* a, Sprite* b)
            {
              const auto aSize = a->uv1 - a->uv0;
              const auto aArea = glm::abs(aSize.x * aSize.y);
              const auto bSize = b->uv1 - b->uv0;
              const auto bArea = glm::abs(bSize.x * bSize.y);
              return aArea > bArea;
            });

  for(auto* sprite : spritesOrderedBySize)
  {
    if(!doneSprites.emplace(sprite).second)
      continue;

    const std::pair minMaxPx{glm::ivec2{sprite->uv0 * 256.0f}, glm::ivec2{sprite->uv1 * 256.0f}};

    const SourceTile srcTile{sprite->atlasId.get(), minMaxPx};
    auto it = replaced.find(srcTile);
    std::pair<core::AtlasId, glm::ivec2> replacementPos{{0}, {0, 0}};

    if(it == replaced.end())
    {
      const auto& atlas = level.m_atlases.at(sprite->atlasId.get());
      if(atlases.isOnlyLayout())
      {
        replacementPos = atlases.put((minMaxPx.second - minMaxPx.first) + glm::ivec2{1, 1});
      }
      else
      {
        auto replacementImg = std::make_unique<gl::CImgWrapper>(
          // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
          reinterpret_cast<const uint8_t*>(atlas.image->getRawData()),
          256,
          256,
          true);
        replacementImg->crop(minMaxPx.first.x, minMaxPx.first.y, minMaxPx.second.x, minMaxPx.second.y);
        replacementPos = atlases.put(*replacementImg);
      }
      replaced.emplace(srcTile, replacementPos);
    }
    else
    {
      replacementPos = it->second;
    }
    const auto replacementUvPos = glm::vec2{replacementPos.second} / gsl::narrow_cast<float>(atlases.getSize());
    const std::pair minMaxUv{sprite->uv0, sprite->uv1};
    remap(*sprite,
          replacementPos.first,
          replacementUvPos,
          replacementUvPos + (minMaxUv.second - minMaxUv.first) * atlasUvScale);
    sprite->atlasId = replacementPos.first;
  }

  gsl_Ensures(doneTiles.size() == atlasTiles.size());
  gsl_Ensures(doneSprites.size() == sprites.size());
}

struct DataPart
{
  const uint32_t* src;
  uint32_t width;
  uint32_t lines;
  uint32_t offset;
};

class DataProvider
{
public:
  explicit DataProvider(gl::CImgWrapper& img)
      : m_bmp{std::make_shared<Bitmap>(
        glm::ivec2{img.width(), img.height()},
        gsl::span{reinterpret_cast<const uint32_t*>(img.pixels().data()), img.pixels().size()})}
  {
    m_current = m_bmp[0];
  }
  ~DataProvider() = default;

  [[nodiscard]] uint32_t numberOfParts() const
  {
    return ((m_bmp[0]->size().y / 4) + m_linesPerPart - 1) / m_linesPerPart;
  }

  [[nodiscard]] DataPart nextPart()
  {
    assert(!m_done);

    const auto [ptr, lines, done] = m_current->nextBlock(m_linesPerPart);
    DataPart ret{ptr, std::max<uint32_t>(4, m_current->size().x), lines, m_offset};

    m_offset += m_current->size().x / 4 * lines;

    m_done |= done;

    return ret;
  }

  [[nodiscard]] const glm::ivec2& size() const
  {
    return m_bmp[0]->size();
  }

private:
  std::vector<std::shared_ptr<Bitmap>> m_bmp;
  std::shared_ptr<Bitmap> m_current;
  uint32_t m_offset = 0;
  uint32_t m_linesPerPart = 32;
  bool m_done = false;
};

void compressEtc2(gl::CImgWrapper& wrapper, const std::filesystem::path& output)
{
  wrapper.interleave();

  BOOST_LOG_TRIVIAL(debug) << "init data provider";
  DataProvider dp{wrapper};
  auto num = dp.numberOfParts();

  BOOST_LOG_TRIVIAL(debug) << "parts: " << num;

  BOOST_LOG_TRIVIAL(debug) << "create block data";
  BlockData bd{output.string().c_str(), dp.size()};

  std::vector<std::future<void>> futures;
  futures.reserve(num);
  for(uint32_t i = 0; i < num; i++)
  {
    futures.emplace_back(std::async(std::launch::async,
                                    [part = dp.nextPart(), &bd]()
                                    {
                                      bd.processRgba(
                                        part.src, part.width / 4 * part.lines, part.offset, part.width, true);
                                    }));
  }
  for(const auto& f : futures)
  {
    f.wait();
  }
}
} // namespace

std::unique_ptr<gl::Texture2DArray<gl::PremultipliedSRGBA8>>
  buildTextures(const loader::file::level::Level& level,
                const std::unique_ptr<loader::trx::Glidos>& glidos,
                render::MultiTextureAtlas& atlases,
                std::vector<AtlasTile>& atlasTiles,
                std::vector<Sprite>& sprites,
                const std::function<void(const std::string&)>& drawLoadingScreen,
                const std::filesystem::path& cacheDir)
{
  drawLoadingScreen(_("Building atlases"));

  BOOST_LOG_TRIVIAL(debug) << "Converting level atlases to images";
  for(auto& atlas : level.m_atlases)
  {
    atlas.toImage();
  }

  std::unordered_set<AtlasTile*> doneTiles;
  std::unordered_set<Sprite*> doneSprites;

  if(glidos != nullptr)
  {
    std::map<std::filesystem::path, glm::ivec2> textureSizes;
    if(atlases.isOnlyLayout() && std::filesystem::is_regular_file(cacheDir / "_texturesizes.yaml"))
    {
      serialization::YAMLDocument<true> doc{cacheDir / "_texturesizes.yaml"};
      doc.deserialize("sizes", level, textureSizes);
    }
    layoutAtlases(
      level, *glidos, atlases, atlasTiles, sprites, doneTiles, doneSprites, textureSizes, drawLoadingScreen);
    if(!atlases.isOnlyLayout())
    {
      serialization::YAMLDocument<false> doc{cacheDir / "_texturesizes.yaml"};
      doc.serialize("sizes", level, textureSizes);
      doc.write();
    }
  }

  materializeAtlases(level, atlases, atlasTiles, sprites, doneTiles, doneSprites, drawLoadingScreen);

  const int textureLevels = static_cast<int>(std::log2(atlases.getSize()) + 1) / 2;
  auto allTextures = std::make_unique<gl::Texture2DArray<gl::PremultipliedSRGBA8>>(
    glm::ivec3{atlases.getSize(), atlases.getSize(), gsl::narrow<int>(atlases.numAtlases())},
    "all-textures",
    textureLevels);

  if(atlases.isOnlyLayout())
  {
    std::vector<std::future<std::shared_ptr<Bitmap>>> loaders;

    for(size_t i = 0; i < atlases.numAtlases(); ++i)
    {
      loaders.emplace_back(std::async(std::launch::async,
                                      [i, &cacheDir, &atlases]()
                                      {
                                        const auto cacheFile = cacheDir / (std::to_string(i) + ".pvr");
                                        BOOST_LOG_TRIVIAL(info) << "Loading cache texture " << cacheFile;
                                        const auto data = std::make_shared<BlockData>(cacheFile.string().c_str());
                                        const auto bmp = data->decode();

                                        gsl_Assert(bmp->size().x == atlases.getSize());
                                        gsl_Assert(bmp->size().y == atlases.getSize());

                                        return bmp;
                                      }));
    }

    for(size_t i = 0; i < atlases.numAtlases(); ++i)
    {
      drawLoadingScreen(_("Building atlases (%1%%%)", 75 + i * 25 / atlases.numAtlases()));

      const auto bmp = loaders[i].get();

      allTextures->assign(gsl::span{(gl::PremultipliedSRGBA8*)bmp->data(), (size_t)(bmp->size().x * bmp->size().y)},
                          gsl::narrow_cast<int>(i));
    }
  }
  else
  {
    auto images = atlases.takeImages();
    for(size_t i = 0; i < images.size(); ++i)
    {
      drawLoadingScreen(_("Building atlases (%1%%%)", 75 + i * 25 / images.size()));

      const auto cacheFile = cacheDir / (std::to_string(i) + ".pvr");
      BOOST_LOG_TRIVIAL(info) << "Saving cache texture " << cacheFile;
      compressEtc2(*images[i], cacheFile);

      allTextures->assign(images[i]->premultipliedPixels(), gsl::narrow_cast<int>(i));

      images[i].reset();
    }
  }

  allTextures->generateMipmaps();

  return allTextures;
}
} // namespace engine::world
