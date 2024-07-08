#include "binaryfile.h"

#include <algorithm>
#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <gsl/gsl-lite.hpp>
#include <ios>
#include <iosfwd>
#include <stdexcept>

namespace image
{
BinaryFile::BinaryFile(const std::filesystem::path& filepath)
    : m_file{filepath, std::ios::in | std::ios::binary}
    , m_filepath{filepath}
{
  BOOST_LOG_TRIVIAL(debug) << "open " << filepath;
  m_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  if(m_file.fail() || !m_file.is_open())
    BOOST_THROW_EXCEPTION(std::runtime_error("failed to open binary file"));
}

bool BinaryFile::read(const gsl::span<uint8_t>& buffer, const std::streampos& seek)
{
  std::fill(buffer.begin(), buffer.end(), uint8_t{0});
  m_file.seekg(seek, std::ios::beg);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  if(m_file.read(reinterpret_cast<char*>(buffer.data()), gsl::narrow<std::streamsize>(buffer.size()));
     gsl::narrow<size_t>(m_file.gcount()) != buffer.size())
  {
    BOOST_LOG_TRIVIAL(error) << "read operation failed (partial read)";
    return false;
  }
  if(m_file.fail())
  {
    BOOST_LOG_TRIVIAL(error) << "read operation failed (stream failure)";
    return false;
  }
  return true;
}

std::streamsize BinaryFile::size()
{
  m_file.seekg(0, std::ios::end);
  return m_file.tellg();
}
} // namespace image
