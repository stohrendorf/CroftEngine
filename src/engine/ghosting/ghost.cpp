#include "ghost.h"

#include <boost/assert.hpp>
#include <fstream>
#include <gsl/gsl-lite.hpp>

namespace engine::ghosting
{
constexpr uint32_t DataStreamVersion = 1;

void GhostFrame::write(std::ostream& s) const
{
  auto size = static_cast<uint32_t>(bones.size());
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  s.write(reinterpret_cast<const char*>(&size), sizeof(size));
  for(const auto& bone : bones)
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    s.write(reinterpret_cast<const char*>(&bone), sizeof(bone));

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  s.write(reinterpret_cast<const char*>(&roomId), sizeof(roomId));

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  s.write(reinterpret_cast<const char*>(&modelMatrix), sizeof(modelMatrix));
}

void GhostFrame::read(std::istream& s)
{
  BOOST_ASSERT(!s.eof());

  uint32_t size = 0;
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  s.read(reinterpret_cast<char*>(&size), sizeof(size));
  bones.resize(size);
  for(auto& bone : bones)
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    s.read(reinterpret_cast<char*>(&bone), sizeof(bone));

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  s.read(reinterpret_cast<char*>(&roomId), sizeof(roomId));

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  s.read(reinterpret_cast<char*>(&modelMatrix), sizeof(modelMatrix));
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
  Expects(version == DataStreamVersion);
}

GhostDataReader::~GhostDataReader() = default;

GhostFrame GhostDataReader::read()
{
  GhostFrame result;
  if(!m_file->eof())
  {
    result.read(*m_file);
    ++m_position;
  }
  return result;
}
} // namespace engine::ghosting
