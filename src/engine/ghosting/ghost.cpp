#include "ghost.h"

#include "serialization/path.h"
#include "serialization/quantity.h"
#include "serialization/serialization.h"

#include <boost/assert.hpp>
#include <fstream>
#include <gsl/gsl-lite.hpp>

namespace engine::ghosting
{
constexpr uint32_t DataStreamVersion = 2;

namespace
{
constexpr int16_t MatrixRotationScale = 32767;

void writeMatrix(std::ostream& s, const glm::mat4& m)
{
  BOOST_ASSERT(m[0][3] == 0);
  BOOST_ASSERT(m[1][3] == 0);
  BOOST_ASSERT(m[2][3] == 0);
  BOOST_ASSERT(m[3][3] == 1);

  for(int x = 0; x < 4; ++x)
  {
    for(int y = 0; y < 3; ++y)
    {
      if(x != 3)
      {
        BOOST_ASSERT(m[x][y] >= -1.0f);
        BOOST_ASSERT(m[x][y] <= 1.0f);
        auto scaled = gsl::narrow_cast<int16_t>(m[x][y] * MatrixRotationScale);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        s.write(reinterpret_cast<const char*>(&scaled), sizeof(scaled));
      }
      else
      {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        s.write(reinterpret_cast<const char*>(&m[x][y]), sizeof(m[x][y]));
      }
    }
  }
}

[[nodiscard]] glm::mat4 readMatrix(std::istream& s)
{
  glm::mat4 m{1.0f};
  for(int x = 0; x < 4; ++x)
  {
    for(int y = 0; y < 3; ++y)
    {
      if(x != 3)
      {
        int16_t scaled = 0;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        s.read(reinterpret_cast<char*>(&scaled), sizeof(scaled));
        m[x][y] = static_cast<float>(scaled) / MatrixRotationScale;
      }
      else
      {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        s.read(reinterpret_cast<char*>(&m[x][y]), sizeof(m[x][y]));
      }
    }
  }
  return m;
}
} // namespace

void GhostFrame::write(std::ostream& s) const
{
  uint8_t size = gsl::narrow<uint8_t>(bones.size());
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  s.write(reinterpret_cast<const char*>(&size), sizeof(size));
  for(const auto& bone : bones)
    bone.write(s);

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  s.write(reinterpret_cast<const char*>(&roomId), sizeof(roomId));

  writeMatrix(s, modelMatrix);
}

void GhostFrame::read(std::istream& s)
{
  BOOST_ASSERT(!s.eof());

  uint8_t size = 0;
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  s.read(reinterpret_cast<char*>(&size), sizeof(size));
  bones.resize(size);
  for(auto& bone : bones)
    bone.read(s);

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  s.read(reinterpret_cast<char*>(&roomId), sizeof(roomId));

  modelMatrix = readMatrix(s);
}

GhostDataWriter::GhostDataWriter(const std::filesystem::path& path)
    : m_file{std::make_unique<std::ofstream>(path, std::ios::binary | std::ios::trunc)}
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  m_file->write(reinterpret_cast<const char*>(&DataStreamVersion), sizeof(DataStreamVersion));
}

GhostDataWriter::~GhostDataWriter() = default;

void GhostDataWriter::append(const GhostFrame& frame)
{
  frame.write(*m_file);
}

GhostDataReader::GhostDataReader(const std::filesystem::path& path)
    : m_file{std::make_unique<std::ifstream>(path, std::ios::binary)}
{
  uint32_t version = 0;
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  m_file->read(reinterpret_cast<char*>(&version), sizeof(version));
  if(version != DataStreamVersion)
  {
    m_file.reset();
  }
}

GhostDataReader::~GhostDataReader() = default;

GhostFrame GhostDataReader::read()
{
  GhostFrame result;
  if(m_file != nullptr && !m_file->eof())
  {
    result.read(*m_file);
  }
  return result;
}

void GhostFrame::BoneData::write(std::ostream& s) const
{
  writeMatrix(s, matrix);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  s.write(reinterpret_cast<const char*>(&meshIdx), sizeof(meshIdx));

  uint8_t tmp = visible ? 1 : 0;
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  s.write(reinterpret_cast<const char*>(&tmp), sizeof(tmp));
}

void GhostFrame::BoneData::read(std::istream& s)
{
  matrix = readMatrix(s);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  s.read(reinterpret_cast<char*>(&meshIdx), sizeof(meshIdx));

  uint8_t tmp = 0;
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  s.read(reinterpret_cast<char*>(&tmp), sizeof(tmp));
  visible = tmp != 0;
}

void GhostMeta::serialize(const serialization::Serializer<GhostMeta>& ser)
{
  ser(S_NV("duration", duration), S_NV("finishState", finishState), S_NV("level", level), S_NV("gameflow", gameflow));
}
} // namespace engine::ghosting
