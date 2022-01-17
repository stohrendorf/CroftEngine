#include "scriptengine.h"

#include "reflection.h"
#include "util.h"

#include <gsl/gsl-lite.hpp>
#include <pybind11/stl.h> // IWYU pragma: keep

namespace engine::script
{
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
const Gameflow& ScriptEngine::getGameflow() const
{
  return *gsl::not_null{pybind11::globals()["gameflow"].cast<Gameflow*>()};
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
std::optional<std::string> ScriptEngine::getLocaleOverride() const
{
  return get<std::string>(pybind11::globals(), "locale_override");
}
} // namespace engine::script
