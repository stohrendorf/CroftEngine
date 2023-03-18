#pragma once

#include <filesystem>
#include <gsl/gsl-lite.hpp>

struct archive;

class WriteOnlyXzArchive final
{
public:
  explicit WriteOnlyXzArchive(const std::filesystem::path& path);

  ~WriteOnlyXzArchive();

  void addFile(const std::filesystem::path& srcPath, const std::filesystem::path& archivePath);

private:
  gsl::not_null<archive*> m_archive;
};
