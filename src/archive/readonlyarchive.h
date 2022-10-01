#pragma once

#include <filesystem>
#include <gsl/gsl-lite.hpp>
#include <optional>
#include <string>

struct archive;
struct archive_entry;

class ReadOnlyArchive final
{
public:
  enum class EntryType
  {
    File,
    Directory,
    Other
  };

  explicit ReadOnlyArchive(const std::filesystem::path& path);

  ~ReadOnlyArchive();

  [[nodiscard]] bool next();

  [[nodiscard]] std::filesystem::path getCurrentPathName() const;

  void writeCurrentTo(const std::filesystem::path& destination) const;

  [[nodiscard]] bool failure() const
  {
    return m_failure;
  }

  [[nodiscard]] EntryType getType();

  [[nodiscard]] std::optional<std::string> getErrorString() const;

private:
  bool m_failure = true;
  gsl::not_null<archive*> m_archive;
  archive_entry* m_entry = nullptr;
};
