#pragma once

#include <filesystem>
#include <optional>

[[nodiscard]] extern std::filesystem::path getExpectedSysUserDataDir();
[[nodiscard]] extern std::filesystem::path getExpectedLocalUserDataDir();
[[nodiscard]] extern std::optional<std::filesystem::path> findUserDataDir();

[[nodiscard]] extern std::filesystem::path getExpectedSysEngineDataDir();
[[nodiscard]] extern std::filesystem::path getExpectedLocalEngineDataDir();
[[nodiscard]] extern std::optional<std::filesystem::path> findEngineDataDir();
