#include "texturing.h"

#include "atlastile.h"
#include "core/i18n.h"
#include "core/id.h"
#include "loader/file/datatypes.h"
#include "loader/file/level/level.h"
#include "loader/file/texture.h"
#include "loader/trx/trx.h"
#include "render/textureatlas.h"
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
struct UVRect
{
  explicit UVRect(const std::array<glm::vec2, 4>& cos)
  {
    xy0.x = xy0.y = 1;
    xy1.x = xy1.y = 0;
    for(const auto& co : cos)
    {
      if(co.x == 0 && co.y == 0)
        continue;

      xy0.x = std::min(xy0.x, co.x);
      xy0.y = std::min(xy0.y, co.y);
      xy1.x = std::max(xy1.x, co.x);
      xy1.y = std::max(xy1.y, co.y);
    }
  }

  UVRect(const glm::vec2& t0, const glm::vec2& t1)
  {
    xy0.x = std::min(t0.x, t1.x);
    xy0.y = std::min(t0.y, t1.y);
    xy1.x = std::max(t0.x, t1.x);
    xy1.y = std::max(t0.y, t1.y);
  }

  bool operator==(const UVRect& rhs) const noexcept
  {
    return xy0 == rhs.xy0 && xy1 == rhs.xy1;
  }

  constexpr bool operator<(const UVRect& rhs) const noexcept
  {
    if(xy0.x != rhs.xy0.x)
      return xy0.x < rhs.xy0.x;
    if(xy0.y != rhs.xy1.y)
      return xy0.y < rhs.xy0.y;
    if(xy1.x != rhs.xy1.x)
      return xy1.x < rhs.xy1.x;
    return xy1.y < rhs.xy1.y;
  }

  glm::vec2 xy0{};
  glm::vec2 xy1{};
};

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

void remap(AtlasTile& tile, size_t atlas, const glm::vec2& replacementUvPos, const glm::vec2& replacementUvMax)
{
  tile.textureKey.tileAndFlag &= ~loader::file::TextureIndexMask;
  tile.textureKey.tileAndFlag |= atlas;

  const auto [tileUvMin, tileUvMax] = tile.getMinMaxUv();
  const auto tileUvSize = tileUvMax - tileUvMin;
  if(tileUvSize.x == 0 || tileUvSize.y == 0)
    return;

  for(auto& uvComponent : tile.uvCoordinates)
  {
    if(uvComponent.x == 0 && uvComponent.y == 0)
      continue;

    remapRange(uvComponent, tileUvMin, tileUvMax, replacementUvPos, replacementUvMax);
  }
}
void remap(Sprite& sprite, size_t atlas, const glm::vec2& replacementUvPos, const glm::vec2& replacementUvMax)
{
  sprite.textureId = core::TextureId{atlas};

  // re-map uv coordinates
  const auto a = glm::round(sprite.uv0 * 256.0f) / 256.0f;
  const auto b = glm::round(sprite.uv1 * 256.0f) / 256.0f;

  remapRange(sprite.uv0, a, b, replacementUvPos, replacementUvMax);
  remapRange(sprite.uv1, a, b, replacementUvPos, replacementUvMax);
}

void processGlidosPack(const loader::file::level::Level& level,
                       const loader::trx::Glidos& glidos,
                       render::MultiTextureAtlas& atlases,
                       std::vector<AtlasTile>& atlasTiles,
                       std::vector<Sprite>& sprites,
                       std::unordered_set<AtlasTile*>& doneTiles,
                       std::unordered_set<Sprite*>& doneSprites)
{
  for(size_t texIdx = 0; texIdx < level.m_textures.size(); ++texIdx)
  {
    const auto& texture = level.m_textures[texIdx];
    const auto mappings = glidos.getMappingsForTexture(texture.md5);

    for(const auto& [tile, path] : mappings)
    {
      std::unique_ptr<gl::CImgWrapper> replacementImg;
      if(path.empty() || !std::filesystem::is_regular_file(path))
      {
        replacementImg = std::make_unique<gl::CImgWrapper>(
          // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
          reinterpret_cast<const uint8_t*>(texture.image->getRawData()),
          256,
          256,
          true);
        replacementImg->crop(tile.getX0(), tile.getY0(), tile.getX1(), tile.getY1());
      }
      else
      {
        replacementImg = std::make_unique<gl::CImgWrapper>(path);
      }

      auto [page, replacementPos] = atlases.put(*replacementImg);
      const auto replacementUvPos = glm::vec2{replacementPos} / gsl::narrow_cast<float>(atlases.getSize());
      const auto replacementUvMax = replacementUvPos
                                    + glm::vec2{replacementImg->width() - 1, replacementImg->height() - 1}
                                        / gsl::narrow_cast<float>(atlases.getSize());

      bool remapped = false;
      for(auto& srcTile : atlasTiles)
      {
        if(doneTiles.count(&srcTile) != 0)
          continue;

        if((srcTile.textureKey.tileAndFlag & loader::file::TextureIndexMask) != texIdx)
          continue;

        const auto [minUv, maxUv] = srcTile.getMinMaxUv();
        const auto minPx = glm::ivec2{minUv * 256.0f};
        const auto maxPx = glm::ivec2{maxUv * 256.0f};
        if(!tile.contains(minPx.x, minPx.y) || !tile.contains(maxPx.x, maxPx.y))
          continue;

        doneTiles.emplace(&srcTile);
        remapped = true;
        remap(srcTile, page, replacementUvPos, replacementUvMax);
      }

      for(auto& sprite : sprites)
      {
        if(doneSprites.count(&sprite) != 0)
          continue;

        if(sprite.textureId.get() != texIdx)
          continue;

        const auto a = glm::ivec2{sprite.uv0 * 256.0f};
        const auto b = glm::ivec2{sprite.uv1 * 256.0f};
        if(!tile.contains(a.x, a.y) || !tile.contains(b.x, b.y))
          continue;

        doneSprites.emplace(&sprite);
        remapped = true;
        remap(sprite, page, replacementUvPos, replacementUvMax);
      }

      if(!remapped)
      {
        BOOST_LOG_TRIVIAL(error) << "Failed to re-map texture tile " << tile;
      }
    }
  }

  BOOST_LOG_TRIVIAL(debug) << "Re-mapped " << doneTiles.size() << " tiles and " << doneSprites.size() << " sprites";
}

void remapTextures(const loader::file::level::Level& level,
                   render::MultiTextureAtlas& atlases,
                   std::vector<AtlasTile>& atlasTiles,
                   std::vector<Sprite>& sprites,
                   std::unordered_set<AtlasTile*>& doneTiles,
                   std::unordered_set<Sprite*>& doneSprites)
{
  const auto atlasUvScale = 256.0f / gsl::narrow_cast<float>(atlases.getSize());

  struct SourceTile final
  {
    int textureId;
    std::pair<glm::ivec2, glm::ivec2> px;

    bool operator<(const SourceTile& rhs) const noexcept
    {
      if(textureId != rhs.textureId)
        return textureId < rhs.textureId;

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
      return textureId == rhs.textureId && px == rhs.px;
    }
  };

  std::map<SourceTile, std::pair<size_t, glm::ivec2>> replaced;

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

  for(auto* tile : tilesOrderedBySize)
  {
    if(!doneTiles.emplace(tile).second)
      continue;
    auto textureId = tile->textureKey.tileAndFlag & loader::file::TextureIndexMask;
    const auto [srcMinUv, srcMaxUv] = tile->getMinMaxUv();
    const auto srcMinPx = glm::ivec2{srcMinUv * 256.0f};
    const auto srcMaxPx = glm::ivec2{srcMaxUv * 256.0f};
    const SourceTile srcTile{textureId, {srcMinPx, srcMaxPx}};
    auto it = replaced.find(srcTile);
    std::pair<size_t, glm::ivec2> replacementPos;
    if(it == replaced.end())
    {
      const auto& texture = level.m_textures.at(textureId);
      auto replacementImg = std::make_unique<gl::CImgWrapper>(
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<const uint8_t*>(texture.image->getRawData()),
        256,
        256,
        true);
      replacementImg->crop(srcMinPx.x, srcMinPx.y, srcMaxPx.x, srcMaxPx.y);

      replacementPos = atlases.put(*replacementImg);
      replaced[srcTile] = replacementPos;
    }
    else
    {
      replacementPos = it->second;
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

    std::pair minMaxPx{glm::ivec2{sprite->uv0 * 256.0f}, glm::ivec2{sprite->uv1 * 256.0f}};

    const SourceTile srcTile{sprite->textureId.get(), minMaxPx};
    auto it = replaced.find(srcTile);
    std::pair<size_t, glm::ivec2> replacementPos;

    if(it == replaced.end())
    {
      const auto& texture = level.m_textures.at(sprite->textureId.get());
      auto replacementImg = std::make_unique<gl::CImgWrapper>(
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<const uint8_t*>(texture.image->getRawData()),
        256,
        256,
        true);
      replacementImg->crop(minMaxPx.first.x, minMaxPx.first.y, minMaxPx.second.x, minMaxPx.second.y);

      replacementPos = atlases.put(*replacementImg);
      replaced[srcTile] = replacementPos;
    }
    else
    {
      replacementPos = it->second;
    }
    const auto replacementUvPos = glm::vec2{replacementPos.second} / gsl::narrow_cast<float>(atlases.getSize());
    std::pair minMaxUv{sprite->uv0, sprite->uv1};
    remap(*sprite,
          replacementPos.first,
          replacementUvPos,
          replacementUvPos + (minMaxUv.second - minMaxUv.first) * atlasUvScale);
    sprite->textureId = replacementPos.first;
  }

  Expects(doneTiles.size() == atlasTiles.size());
  Expects(doneSprites.size() == sprites.size());
}
} // namespace

std::unique_ptr<gl::Texture2DArray<gl::SRGBA8>>
  buildTextures(const loader::file::level::Level& level,
                const std::unique_ptr<loader::trx::Glidos>& glidos,
                render::MultiTextureAtlas& atlases,
                std::vector<AtlasTile>& atlasTiles,
                std::vector<Sprite>& sprites,
                const std::function<void(const std::string&)>& drawLoadingScreen)
{
  drawLoadingScreen(_("Building textures"));

  for(auto& texture : level.m_textures)
  {
    texture.toImage();
  }

  BOOST_LOG_TRIVIAL(info) << "Building texture atlases";

  std::unordered_set<AtlasTile*> doneTiles;
  std::unordered_set<Sprite*> doneSprites;

  if(glidos != nullptr)
  {
    processGlidosPack(level, *glidos, atlases, atlasTiles, sprites, doneTiles, doneSprites);
  }

  remapTextures(level, atlases, atlasTiles, sprites, doneTiles, doneSprites);

  const int textureLevels = static_cast<int>(std::log2(atlases.getSize()) + 1) / 2;
  auto images = atlases.takeImages();

  auto allTextures = std::make_unique<gl::Texture2DArray<gl::SRGBA8>>(
    glm::ivec3{atlases.getSize(), atlases.getSize(), gsl::narrow<int>(images.size())}, "all-textures", textureLevels);

  for(size_t i = 0; i < images.size(); ++i)
    allTextures->assign(images[i]->pixels(), gsl::narrow_cast<int>(i));
  allTextures->generateMipmaps();

  return allTextures;
}
} // namespace engine::world
