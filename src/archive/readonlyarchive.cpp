#include "readonlyarchive.h"

#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
// --- boost must be included before these, otherwise linking on (at least) windows fails
#include <archive.h>
#include <archive_entry.h>
#include <fstream>
#include <stdexcept>
#include <vector>

ReadOnlyArchive::ReadOnlyArchive(const std::filesystem::path& path)
    : m_archive{archive_read_new()}
{
  archive_read_support_format_all(m_archive.get());
  archive_read_support_filter_all(m_archive.get());
  m_failure = archive_read_open_filename(m_archive.get(), path.string().c_str(), 10240) != ARCHIVE_OK;
}

ReadOnlyArchive::~ReadOnlyArchive()
{
  gsl_Assert(archive_read_close(m_archive.get()) == ARCHIVE_OK);
  gsl_Assert(archive_read_free(m_archive.get()) == ARCHIVE_OK);
}

bool ReadOnlyArchive::next()
{
  gsl_Expects(!m_failure);
  switch(archive_read_next_header(m_archive.get(), &m_entry))
  {
  case ARCHIVE_OK:
    gsl_Assert(m_entry != nullptr);
    return true;
  case ARCHIVE_EOF:
    m_entry = nullptr;
    return false;
  case ARCHIVE_WARN:
    BOOST_LOG_TRIVIAL(warning) << "Warning occurred while reading archive entry: "
                               << archive_error_string(m_archive.get());
    return true;
  case ARCHIVE_RETRY:
    BOOST_LOG_TRIVIAL(warning) << "Retry request occurred while reading archive entry: "
                               << archive_error_string(m_archive.get());
    return true;
  case ARCHIVE_FATAL:
    BOOST_LOG_TRIVIAL(fatal) << "Fatal error while reading archive entry: " << archive_error_string(m_archive.get());
    BOOST_THROW_EXCEPTION(std::runtime_error("Fatal error while reading archive entry"));
  default:
    BOOST_LOG_TRIVIAL(fatal) << "Unexpected status code while reading archive entry: "
                             << archive_error_string(m_archive.get());
    BOOST_THROW_EXCEPTION(std::runtime_error("Unexpected status code while reading archive entry"));
  }
}

std::filesystem::path ReadOnlyArchive::getCurrentPathName() const
{
  gsl_Expects(!m_failure);
  gsl_Assert(m_entry != nullptr);
  return archive_entry_pathname(m_entry);
}

void ReadOnlyArchive::writeCurrentTo(const std::filesystem::path& destination) const
{
  gsl_Expects(!m_failure);
  std::vector<char> buffer;
  buffer.resize(8192);

  std::ofstream dst{destination, std::ios::binary | std::ios::trunc};
  while(true)
  {
    const auto read = archive_read_data(m_archive.get(), buffer.data(), buffer.size());
    if(read == 0)
      break;
    dst.write(buffer.data(), read);
  }
}

std::optional<std::string> ReadOnlyArchive::getErrorString() const
{
  if(m_failure)
    return "Failed to open archive";

  const auto s = archive_error_string(m_archive.get());
  return s == nullptr ? std::nullopt : std::optional{std::string{s}};
}

ReadOnlyArchive::EntryType ReadOnlyArchive::getType()
{
  gsl_Assert(!m_failure);
  gsl_Assert(m_entry != nullptr);
  switch(archive_entry_filetype(m_entry))
  {
  case AE_IFDIR:
    return EntryType::Directory;
  case AE_IFREG:
    return EntryType::File;
    break;
  default:
    return EntryType::Other;
  }
}
