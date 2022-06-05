#pragma once

#include <optional>
#include <string>
#include <tuple>

namespace launcher
{
[[nodiscard]] extern std::optional<std::tuple<std::string, std::string>> showLauncher(int argc, char** argv);
}
