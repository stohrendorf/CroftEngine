#pragma once

#include <cstdint>
#include <filesystem>
#include <glm/matrix.hpp>
#include <iosfwd>
#include <vector>

namespace engine::ghosting
{
struct GhostFrame
{
  struct BoneData
  {
    glm::mat4 matrix;
    uint32_t meshIdx;
    bool visible;
  };

  uint32_t roomId;
  glm::mat4 modelMatrix;
  std::vector<BoneData> bones;

  void write(std::ostream& s) const;
  void read(std::istream& s);
};

class GhostDataWriter
{
public:
  explicit GhostDataWriter(const std::filesystem::path& path);
  ~GhostDataWriter();

  void append(const GhostFrame& frame);

private:
  std::unique_ptr<std::ostream> m_file;
};

class GhostDataReader
{
public:
  explicit GhostDataReader(const std::filesystem::path& path);
  ~GhostDataReader();

  [[nodiscard]] GhostFrame read();

  [[nodiscard]] size_t tell() const
  {
    return m_position;
  }

private:
  std::unique_ptr<std::istream> m_file;
  size_t m_position = 0;
};
} // namespace engine::ghosting