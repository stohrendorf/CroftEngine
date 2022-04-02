#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <glm/mat4x4.hpp>
#include <iosfwd>
#include <memory>
#include <vector>

namespace engine::ghosting
{
struct GhostFrame
{
  struct BoneData
  {
    glm::mat4 matrix{1.0f};
    uint16_t meshIdx = 0;
    bool visible = false;

    void write(std::ostream& s) const;
    void read(std::istream& s);
  };

  uint16_t roomId = 0;
  glm::mat4 modelMatrix{1.0f};
  std::vector<BoneData> bones{};

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

  [[nodiscard]] bool isOpen() const
  {
    return m_file != nullptr;
  }

private:
  std::unique_ptr<std::istream> m_file;
};
} // namespace engine::ghosting