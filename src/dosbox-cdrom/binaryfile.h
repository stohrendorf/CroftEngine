#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <gsl/gsl-lite.hpp>

class BinaryFile final
{
public:
  explicit BinaryFile(const std::filesystem::path& filepath);
  BinaryFile() = delete;
  bool read(const gsl::span<uint8_t>& buffer, std::streampos seek);
  std::streamsize size();
  [[nodiscard]] const auto& getFilepath() const
  {
    return m_filepath;
  }

private:
  std::ifstream m_file;
  std::filesystem::path m_filepath;
};
