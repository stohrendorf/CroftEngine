#include "writeonlyxzarchive.h"

#include <archive.h>
#include <archive_entry.h>
#include <fstream>
#include <string>
#include <vector>

WriteOnlyXzArchive::WriteOnlyXzArchive(const std::filesystem::path& path)
    : m_archive{archive_write_new()}
{
  gsl_Assert(archive_write_set_format_pax_restricted(m_archive.get()) == ARCHIVE_OK);
  gsl_Assert(archive_write_add_filter_xz(m_archive.get()) == ARCHIVE_OK);
  gsl_Assert(archive_write_set_options(m_archive.get(), "compression-level=9") == ARCHIVE_OK);
  gsl_Assert(archive_write_open_filename(m_archive.get(), path.string().c_str()) == ARCHIVE_OK);
}

WriteOnlyXzArchive::~WriteOnlyXzArchive()
{
  gsl_Assert(archive_write_close(m_archive.get()) == ARCHIVE_OK);
  gsl_Assert(archive_write_free(m_archive.get()) == ARCHIVE_OK);
}

void WriteOnlyXzArchive::addFile(const std::filesystem::path& srcPath, const std::filesystem::path& archivePath)
{
  gsl::not_null entry{archive_entry_new()};
  const auto freeEntry = gsl::finally(
    [&entry]()
    {
      archive_entry_free(entry.get());
    });
  archive_entry_copy_pathname(entry, archivePath.filename().string().c_str());
  archive_entry_set_size(entry, std::filesystem::file_size(srcPath));
  archive_entry_set_filetype(entry, AE_IFREG);
  archive_entry_set_perm(entry, 0644);
  gsl_Assert(archive_write_header(m_archive.get(), entry) == ARCHIVE_OK);

  std::vector<char> buffer;
  buffer.resize(1024);
  std::ifstream f{srcPath, std::ios::in | std::ios::binary};
  gsl_Assert(f.is_open());
  do
  {
    f.read(buffer.data(), buffer.size());
    gsl_Assert(archive_write_data(m_archive.get(), buffer.data(), f.gcount()) == f.gcount());
  } while(f.gcount() > 0);
}
