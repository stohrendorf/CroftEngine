#include "binaryfile.h"

#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>

BinaryFile::BinaryFile(const std::filesystem::path& filepath)
    : m_file{filepath, std::ios::in | std::ios::binary}
    , m_filepath{filepath}
{
  BOOST_LOG_TRIVIAL(debug) << "open " << filepath;
  m_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  if(m_file.fail() || !m_file.is_open())
    BOOST_THROW_EXCEPTION(std::runtime_error("failed to open binary file"));
}

bool BinaryFile::read(const gsl::span<uint8_t>& buffer, std::streampos seek)
{
  std::fill(buffer.begin(), buffer.end(), uint8_t{0});
  m_file.seekg(seek, std::ios::beg);
  m_file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
  return !m_file.fail();
}

std::streamsize BinaryFile::size()
{
  m_file.seekg(0, std::ios::end);
  return m_file.tellg();
}
