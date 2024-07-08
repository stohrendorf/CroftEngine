#pragma once

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <functional>
#include <glm/vec2.hpp>
#include <map>
#include <ostream>
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

  explicit Rectangle(const glm::ivec2& xy0, const glm::ivec2& xy1)
      : m_xy0{std::min(xy0.x, xy1.x), std::min(xy0.y, xy1.y)}
      , m_xy1{std::max(xy0.x, xy1.x), std::max(xy0.y, xy1.y)}
  {
  }

  bool operator<(const Rectangle& rhs) const
  {
    if(m_xy0.x != rhs.m_xy0.x)
      return m_xy0.x < rhs.m_xy0.x;
    if(m_xy1.x != rhs.m_xy1.x)
      return m_xy1.x < rhs.m_xy1.x;
    if(m_xy0.y != rhs.m_xy0.y)
      return m_xy0.y < rhs.m_xy0.y;
    return m_xy1.y < rhs.m_xy1.y;
  }

  [[nodiscard]] const auto& getXY0() const noexcept
  {
    return m_xy0;
  }

  [[nodiscard]] const auto& getXY1() const noexcept
  {
    return m_xy1;
  }

  [[nodiscard]] auto getSize() const
  {
    return m_xy1 - m_xy0;
  }

  [[nodiscard]] int getWidth() const
  {
    return m_xy1.x - m_xy0.x;
  }

  [[nodiscard]] int getHeight() const
  {
    return m_xy1.y - m_xy0.y;
  }

  [[nodiscard]] bool contains(int x, int y) const
  {
    return x >= m_xy0.x && x < m_xy1.x && y >= m_xy0.y && y < m_xy1.y;
  }

private:
  glm::ivec2 m_xy0{0, 0};
  glm::ivec2 m_xy1{0, 0};
};

inline std::ostream& operator<<(std::ostream& str, const Rectangle& r)
{
  return str << "(" << r.getXY0().x << "," << r.getXY0().y << ")-(" << r.getXY1().x << "," << r.getXY1().y << ")";
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
  Rectangle m_rect;
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
  explicit Equiv(const std::filesystem::path& filename,
                 const std::function<void(const std::string&)>& statusCallback,
                 std::filesystem::file_time_type& newestFile);

  void resolve(const std::filesystem::path& root,
               std::map<TexturePart, std::filesystem::path>& filesByPart,
               const std::function<void(const std::string&)>& statusCallback,
               std::filesystem::file_time_type& newestFile) const;

private:
  std::vector<EquivalenceSet> m_equivalentSets;
};

class PathMap
{
public:
  explicit PathMap(const std::filesystem::path& baseTxtName,
                   std::map<TexturePart, std::filesystem::path>& filesByPart,
                   std::filesystem::file_time_type& newestFile);

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
  void insertInternalMapping(const std::string& textureId, const Rectangle& tile);

  [[nodiscard]] const auto& getNewestFileTime() const
  {
    return m_newestFile;
  }

  [[nodiscard]] const auto& getBaseDir() const
  {
    return m_baseDir;
  }

private:
  std::map<TexturePart, std::filesystem::path> m_filesByPart;
  std::filesystem::path m_baseDir;
  std::filesystem::file_time_type m_newestFile;
};
} // namespace loader::trx
