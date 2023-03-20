#pragma once

#include <cstdint>
#include <gl/cimgwrapper.h>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <optional>

namespace render
{
/**
 * @brief A Binary Space Partition Tree for 2D space.
 *
 * Based on https://blackpawn.com/texts/lightmaps/default.html.
 */
#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable : 4324)
#endif
struct alignas(64) BSPTree final
{
  std::unique_ptr<BSPTree> left;
  std::unique_ptr<BSPTree> right;

  //! If @c true, denotes that there is no more free space in this node or its children.
  //! @note This is a pure caching mechanism to avoid unnecessary recursion.
  bool isFilled = false;

  int32_t x = 0;
  int32_t y = 0;
  int32_t width = 0;
  int32_t height = 0;

  BSPTree() = default;

  BSPTree(const int32_t x, const int32_t y, const int32_t w, const int32_t h)
      : x{x}
      , y{y}
      , width{w}
      , height{h}
  {
    gsl_Expects(x >= 0);
    gsl_Expects(y >= 0);
    gsl_Expects(w > 0);
    gsl_Expects(h > 0);
  }

  BSPTree(BSPTree&& rhs) noexcept
      : left{std::move(rhs.left)}
      , right{std::move(rhs.right)}
      , isFilled{rhs.isFilled}
      , x{rhs.x}
      , y{rhs.y}
      , width{rhs.width}
      , height{rhs.height}
  {
  }

  [[nodiscard]] bool isSplit() const
  {
    return left != nullptr && right != nullptr;
  }

  /**
     * @brief Split this node along its Y axis (X is split).
     * @param splitLocation Local X coordinate of the split point
     */
  void splitX(const int32_t splitLocation)
  {
    gsl_Expects(splitLocation < width);
    left = std::make_unique<BSPTree>(x, y, splitLocation, height);
    right = std::make_unique<BSPTree>(x + splitLocation, y, width - splitLocation, height);
  }

  /**
     * @brief Split this node along its X axis (Y is split).
     * @param splitLocation Local Y coordinate of the split point
     */
  void splitY(const int32_t splitLocation)
  {
    gsl_Expects(splitLocation < height);
    gsl_Expects(left == nullptr && right == nullptr);
    left = std::make_unique<BSPTree>(x, y, width, splitLocation);
    right = std::make_unique<BSPTree>(x, y + splitLocation, width, height - splitLocation);
  }

  [[nodiscard]] bool fits(const int32_t w, const int32_t h) const noexcept
  {
    gsl_Expects(w > 0);
    gsl_Expects(h > 0);
    return !isFilled && w <= width && h <= height;
  }

  /**
     * @brief Find a free space in this node or its children
     */
  // NOLINTNEXTLINE(misc-no-recursion)
  std::optional<glm::ivec2> tryInsert(const int32_t insWidth, const int32_t insHeight)
  {
    // Could this possibly fit?
    if(!fits(insWidth, insHeight))
      return std::nullopt;

    if(isSplit())
    {
      // This node is already split => Recurse!
      std::optional<glm::ivec2> found{};
      if(insWidth <= left->width && insHeight <= left->height)
      {
        found = left->tryInsert(insWidth, insHeight);
      }

      if(!found.has_value() && insWidth <= right->width && insHeight <= right->height)
      {
        found = right->tryInsert(insWidth, insHeight);
      }

      // If both children are filled, mark this node as filled and discard the children.
      if(left->isFilled && right->isFilled)
      {
        isFilled = true;
        left.reset();
        right.reset();
      }

      return found;
    }

    // We may split this node
    if(height == insHeight && width == insWidth)
    {
      // Perfect match
      isFilled = true;
      return glm::ivec2{x, y};
    }
    else if(height == insHeight)
    {
      // Split horizontally
      splitX(insWidth);

      // height already fits, width fits too now, so this is the result
      left->isFilled = true;
      return glm::ivec2{x, y};
    }
    else
    {
      const auto dw = width - insWidth;
      const auto dh = height - insHeight;
      if(dw > dh)
      {
        splitX(insWidth);
      }
      else
      {
        splitY(insHeight);
      }

      // Recurse, because the size may not match
      return left->tryInsert(insWidth, insHeight);
    }
  }
};
#ifdef _MSC_VER
#  pragma warning(pop)
#endif

class TextureAtlas final
{
  BSPTree m_layout;
  std::shared_ptr<gl::CImgWrapper> m_image;

public:
  explicit TextureAtlas(const int32_t pageSize, bool onlyLayout)
      : m_layout{0, 0, pageSize, pageSize}
      , m_image{onlyLayout ? nullptr : std::make_shared<gl::CImgWrapper>(pageSize)}
  {
  }

  ~TextureAtlas() = default;

  TextureAtlas(TextureAtlas&& rhs) noexcept
      : m_layout{std::move(rhs.m_layout)}
      , m_image{std::move(rhs.m_image)}
  {
  }

  std::optional<glm::ivec2> put(gl::CImgWrapper& img)
  {
    gsl_Assert(m_image != nullptr);
    auto dstArea = m_layout.tryInsert(img.width(), img.height());
    if(!dstArea.has_value())
      return std::nullopt;

    for(int y = 0; y < img.height(); ++y)
    {
      for(int x = 0; x < img.width(); ++x)
      {
        (*m_image)(x + dstArea->x, y + dstArea->y) = img(x, y);
      }
    }
    return dstArea;
  }

  std::optional<glm::ivec2> put(const glm::ivec2& size)
  {
    gsl_Assert(m_image == nullptr);
    return m_layout.tryInsert(size.x, size.y);
  }

  [[nodiscard]] std::shared_ptr<gl::CImgWrapper> takeImage()
  {
    gsl_Assert(m_image != nullptr);
    return std::move(m_image);
  }
};

class MultiTextureAtlas final
{
  std::vector<TextureAtlas> m_atlases{};
  int32_t m_pageSize;
  bool m_onlyLayout;

public:
  static constexpr int BoundaryMargin = 16;

  explicit MultiTextureAtlas(const int32_t pageSize, bool onlyLayout)
      : m_pageSize{pageSize}
      , m_onlyLayout{onlyLayout}
  {
  }

  ~MultiTextureAtlas() = default;

  [[nodiscard]] auto getSize() const
  {
    return m_pageSize;
  }

  std::pair<size_t, glm::ivec2> put(const gl::CImgWrapper& img)
  {
    gsl_Assert(!m_onlyLayout);
    auto extended = img;
    extended.extendBorder(BoundaryMargin);

    for(size_t i = 0; i < m_atlases.size(); ++i)
      if(const auto position = m_atlases[i].put(extended))
        return {i, *position + glm::ivec2{BoundaryMargin, BoundaryMargin}};

    m_atlases.emplace_back(m_pageSize, m_onlyLayout);
    auto position = m_atlases.back().put(extended);
    gsl_Assert(position.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    return {m_atlases.size() - 1, position.value() + glm::ivec2{BoundaryMargin, BoundaryMargin}};
  }

  std::pair<size_t, glm::ivec2> put(const glm::ivec2& size)
  {
    gsl_Assert(m_onlyLayout);

    for(size_t i = 0; i < m_atlases.size(); ++i)
      if(const auto position = m_atlases[i].put(size + 2 * glm::ivec2{BoundaryMargin, BoundaryMargin}))
        return {i, *position + glm::ivec2{BoundaryMargin, BoundaryMargin}};

    m_atlases.emplace_back(m_pageSize, m_onlyLayout);
    auto position = m_atlases.back().put(size + 2 * glm::ivec2{BoundaryMargin, BoundaryMargin});
    gsl_Assert(position.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    return {m_atlases.size() - 1, position.value() + glm::ivec2{BoundaryMargin, BoundaryMargin}};
  }

  std::vector<std::shared_ptr<gl::CImgWrapper>> takeImages()
  {
    gsl_Assert(!m_onlyLayout);

    std::vector<std::shared_ptr<gl::CImgWrapper>> result;
    result.reserve(m_atlases.size());
    for(auto& atlas : m_atlases)
      result.emplace_back(atlas.takeImage());
    return result;
  }

  [[nodiscard]] bool isOnlyLayout() const
  {
    return m_onlyLayout;
  }

  [[nodiscard]] auto numAtlases() const
  {
    return m_atlases.size();
  }
};
} // namespace render
