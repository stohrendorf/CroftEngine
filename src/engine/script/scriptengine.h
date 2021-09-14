#pragma once

#include "core/id.h"
#include "engine/tracks_tr1.h"

#include <cstddef>
#include <filesystem>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <optional>
#include <pybind11/cast.h>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <string>
#include <vector>

namespace engine::script
{
class LevelSequenceItem;
struct TrackInfo;
struct ObjectInfo;

class ScriptEngine
{
public:
  explicit ScriptEngine(const std::filesystem::path& rootPath)
      : m_interpreter{std::make_unique<pybind11::scoped_interpreter>()}
  {
    pybind11::module::import("sys").attr("path").cast<pybind11::list>().append(
      std::filesystem::absolute(rootPath).string());
  }

  [[nodiscard]] gsl::not_null<LevelSequenceItem*> getTitleMenu() const;
  [[nodiscard]] std::vector<LevelSequenceItem*> getLaraHome() const;
  [[nodiscard]] std::vector<LevelSequenceItem*> getEarlyBoot() const;
  [[nodiscard]] LevelSequenceItem* getLevelSequenceItem(size_t idx) const;
  [[nodiscard]] TrackInfo getTrackInfo(engine::TR1TrackId trackId) const;
  [[nodiscard]] std::optional<std::string> getLocaleOverride() const;
  [[nodiscard]] ObjectInfo getObjectInfo(const core::TypeId& type) const;

  [[nodiscard]] bool isGodMode() const;
  [[nodiscard]] bool hasAllAmmoCheat() const;
  [[nodiscard]] pybind11::dict getCheatInventory() const;

private:
  std::unique_ptr<pybind11::scoped_interpreter> m_interpreter;
};
} // namespace engine::script
