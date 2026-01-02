#pragma once

#include <filesystem>
#include <gsl-lite/gsl-lite.hpp>

struct archive;

class WriteOnlyXzArchive final
{
public:
  explicit WriteOnlyXzArchive(const std::filesystem::path& path);

  ~WriteOnlyXzArchive();

  void addFile(const std::filesystem::path& srcPath, const std::filesystem::path& archivePath);

private:
  gsl_lite::not_null<archive*> m_archive;
};