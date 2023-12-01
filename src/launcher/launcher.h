#pragma once

#include <optional>
#include <string>
#include <tuple>

namespace launcher
{
[[nodiscard]] extern std::optional<std::tuple<std::string, std::string,bool>> showLauncher(int argc, char** argv);
}
