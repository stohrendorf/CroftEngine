#pragma once

#include <filesystem>

[[nodiscard]] extern std::filesystem::path getUserDataDir();
[[nodiscard]] extern std::filesystem::path getEngineDataDir();
