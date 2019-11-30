#pragma once

#include "gsl-lite.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <chrono>
#include <map>
#include <regex>
#include <set>
#include <utility>

namespace loader::trx
{
class Rectangle
{
public:
  explicit Rectangle() = default;

  explicit Rectangle(const std::string& serialized);

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

  explicit TexturePart(std::string filename, const Rectangle& r)
      : m_textureId{std::move(filename)}
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
  explicit Equiv(const boost::filesystem::path& filename);

  void resolve(const boost::filesystem::path& root,
               std::map<std::string, std::chrono::system_clock::time_point>& timestamps,
               std::chrono::system_clock::time_point& rootTimestamp,
               std::map<TexturePart, boost::filesystem::path>& filesByPart,
               const std::function<void(const std::string&)>& statusCallback) const;

private:
  std::vector<EquivalenceSet> m_equivalentSets;
};

class PathMap
{
public:
  explicit PathMap(const boost::filesystem::path& baseTxtName,
                   std::map<std::string, std::chrono::system_clock::time_point>& timestamps,
                   std::chrono::system_clock::time_point& rootTimestamp,
                   std::map<TexturePart, boost::filesystem::path>& filesByPart);

  [[nodiscard]] const boost::filesystem::path& getRoot() const
  {
    return m_root;
  }

private:
  boost::filesystem::path m_root;
};

class Glidos
{
public:
  explicit Glidos(boost::filesystem::path baseDir, const std::function<void(const std::string&)>& statusCallback);

  void dump() const;

  struct TileMap
  {
    std::map<Rectangle, boost::filesystem::path> tiles;
    std::chrono::system_clock::time_point newestSource;
    boost::filesystem::path baseDir;
  };

  TileMap getMappingsForTexture(const std::string& textureId) const;

  const auto& getBaseDir() const noexcept
  {
    return m_baseDir;
  }

private:
  std::map<TexturePart, boost::filesystem::path> m_filesByPart;
  const boost::filesystem::path m_baseDir;
  mutable std::map<std::string, std::chrono::system_clock::time_point> m_newestTextureSourceTimestamps;
  std::chrono::system_clock::time_point m_rootTimestamp;
};
} // namespace loader::trx
