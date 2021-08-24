#include "scriptengine.h"

#include "reflection.h"

#include <pybind11/stl.h>

namespace engine::script
{
namespace
{
template<typename T, typename U>
std::optional<T> get(const pybind11::dict& d, const U& key)
{
  const auto pyKey = pybind11::cast(key);
  if(!d.contains(pyKey))
    return std::nullopt;
  return d[pyKey].template cast<T>();
}
} // namespace

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
gsl::not_null<LevelSequenceItem*> ScriptEngine::getTitleMenu() const
{
  return gsl::not_null{pybind11::globals()["title_menu"].cast<LevelSequenceItem*>()};
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
std::vector<LevelSequenceItem*> ScriptEngine::getLaraHome() const
{
  return pybind11::globals()["lara_home"].cast<std::vector<LevelSequenceItem*>>();
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
std::vector<LevelSequenceItem*> ScriptEngine::getEarlyBoot() const
{
  return pybind11::globals()["early_boot"].cast<std::vector<LevelSequenceItem*>>();
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
LevelSequenceItem* ScriptEngine::getLevelSequenceItem(size_t idx) const
{
  auto levelSequence = pybind11::globals()["level_sequence"];
  if(idx >= pybind11::len(levelSequence))
    return nullptr;

  return levelSequence[pybind11::cast(idx)].cast<LevelSequenceItem*>();
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
TrackInfo ScriptEngine::getTrackInfo(engine::TR1TrackId trackId) const
{
  return pybind11::globals()["getTrackInfo"](trackId).cast<script::TrackInfo>();
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
std::optional<std::string> ScriptEngine::getLanguageOverride() const
{
  return get<std::string>(pybind11::globals(), "language_override");
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
ObjectInfo ScriptEngine::getObjectInfo(const core::TypeId& type) const
{
  return pybind11::globals()["getObjectInfo"](type.get()).cast<ObjectInfo>();
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
bool ScriptEngine::isGodMode() const
{
  return get<bool>(get<pybind11::dict>(pybind11::globals(), "cheats").value_or(pybind11::dict{}), "godMode")
    .value_or(false);
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
bool ScriptEngine::hasAllAmmoCheat() const
{
  return get<bool>(get<pybind11::dict>(pybind11::globals(), "cheats").value_or(pybind11::dict{}), "allAmmoCheat")
    .value_or(false);
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
pybind11::dict ScriptEngine::getCheatInventory() const
{
  return get<pybind11::dict>(get<pybind11::dict>(pybind11::globals(), "cheats").value_or(pybind11::dict{}), "inventory")
    .value_or(pybind11::dict{});
}
} // namespace engine::script
