#pragma once

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <gsl/gsl-lite.hpp>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace loader::trx
{
class Rectangle
{
public:
  explicit Rectangle() = default;

  explicit Rectangle(const std::string& serialized);

  explicit Rectangle(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1)
      : m_x0{std::min(x0, x1)}
      , m_x1{std::max(x0, x1)}
      , m_y0{std::min(y0, y1)}
      , m_y1{std::max(y0, y1)}
  {
  }

  bool operator<(const Rectangle& rhs) const
  {
    if(m_x0 != rhs.m_x0)
      return m_x0 < rhs.m_x0;
    if(m_x1 != rhs.m_x1)
      return m_x1 < rhs.m_x1;
    if(m_y0 != rhs.m_y0)
      return m_y0 < rhs.m_y0;
    return m_y1 < rhs.m_y1;
  }

  [[nodiscard]] uint32_t getX0() const noexcept
  {
    return m_x0;
  }

  [[nodiscard]] uint32_t getX1() const noexcept
  {
    return m_x1;
  }

  [[nodiscard]] uint32_t getY0() const noexcept
  {
    return m_y0;
  }

  [[nodiscard]] uint32_t getY1() const noexcept
  {
    return m_y1;
  }

  [[nodiscard]] int getWidth() const
  {
    return gsl::narrow<int>(m_x1 - m_x0);
  }

  [[nodiscard]] int getHeight() const
  {
    return gsl::narrow<int>(m_y1 - m_y0);
  }

  [[nodiscard]] bool contains(uint32_t x, uint32_t y) const
  {
    return x >= m_x0 && x <= m_x1 && y >= m_y0 && y <= m_y1;
  }

private:
  uint32_t m_x0 = 0;
  uint32_t m_x1 = 0;
  uint32_t m_y0 = 0;
  uint32_t m_y1 = 0;
};

inline std::ostream& operator<<(std::ostream& str, const Rectangle& r)
{
  return str << "(" << r.getX0() << "," << r.getY0() << ")-(" << r.getX1() << "," << r.getY1() << ")";
}

class TexturePart
{
public:
  explicit TexturePart(const std::string& serialized);

  explicit TexturePart(std::string textureId, const Rectangle& r)
      : m_textureId{std::move(textureId)}
      , m_rect{r}
  {
  }

  [[nodiscard]] const std::string& getId() const
  {
    return m_textureId;
  }

  [[nodiscard]] const Rectangle& getRectangle() const
  {
    return m_rect;
  }

  bool operator<(const TexturePart& rhs) const
  {
    if(m_textureId != rhs.m_textureId)
      return m_textureId < rhs.m_textureId;

    return m_rect < rhs.m_rect;
  }

private:
  std::string m_textureId;
  Rectangle m_rect{};
};

class EquivalenceSet
{
public:
  explicit EquivalenceSet(std::ifstream& file);

  const std::set<TexturePart>& getParts() const
  {
    return m_parts;
  }

  void markResolved() const
  {
    m_resolved = true;
  }

  bool isResolved() const
  {
    return m_resolved;
  }

private:
  std::set<TexturePart> m_parts;
  mutable bool m_resolved = false;
};

class Equiv
{
public:
  explicit Equiv(const std::filesystem::path& filename, const std::function<void(const std::string&)>& statusCallback);

  void resolve(const std::filesystem::path& root,
               std::map<TexturePart, std::filesystem::path>& filesByPart,
               const std::function<void(const std::string&)>& statusCallback) const;

private:
  std::vector<EquivalenceSet> m_equivalentSets;
};

class PathMap
{
public:
  explicit PathMap(const std::filesystem::path& baseTxtName, std::map<TexturePart, std::filesystem::path>& filesByPart);

  [[nodiscard]] const std::filesystem::path& getRoot() const
  {
    return m_root;
  }

private:
  std::filesystem::path m_root;
};

class Glidos
{
public:
  explicit Glidos(std::filesystem::path baseDir, const std::function<void(const std::string&)>& statusCallback);

  using TileMap = std::map<Rectangle, std::filesystem::path>;

  [[nodiscard]] TileMap getMappingsForTexture(const std::string& textureId) const;

  [[nodiscard]] const auto& getBaseDir() const noexcept
  {
    return m_baseDir;
  }

private:
  std::map<TexturePart, std::filesystem::path> m_filesByPart;
  const std::filesystem::path m_baseDir;
};
} // namespace loader::trx
