#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <pybind11/cast.h>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <string>

namespace engine::script
{
class Gameflow;

class ScriptEngine
{
public:
  explicit ScriptEngine(const std::filesystem::path& rootPath)
      : m_interpreter{std::make_unique<pybind11::scoped_interpreter>()}
  {
    pybind11::module::import("sys").attr("path").cast<pybind11::list>().append(
      std::filesystem::absolute(rootPath).string());
  }

  [[nodiscard]] const Gameflow& getGameflow() const;
  [[nodiscard]] std::optional<std::string> getLocaleOverride() const;

private:
  std::unique_ptr<pybind11::scoped_interpreter> m_interpreter;
};
} // namespace engine::script
