#include "scriptengine.h"

#include "engine/items_tr1.h"
#include "reflection.h"
#include "util.h"

#include <pybind11/stl.h> // IWYU pragma: keep

namespace engine::script
{
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
gsl::not_null<Gameflow*> ScriptEngine::getGameflow() const
{
  return gsl::not_null{pybind11::globals()["gameflow"].cast<Gameflow*>()};
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
std::optional<std::string> ScriptEngine::getLocaleOverride() const
{
  return get<std::string>(pybind11::globals(), "locale_override");
}
} // namespace engine::script
