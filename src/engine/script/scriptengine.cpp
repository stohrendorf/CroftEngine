#include "scriptengine.h"

#include "reflection.h"
#include "util.h"
#include "util/helpers.h"

#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <gsl/gsl-lite.hpp>
#include <pybind11/stl.h> // IWYU pragma: keep

namespace engine::script
{
ScriptEngine::ScriptEngine(const std::filesystem::path& rootPath)
    : m_interpreter{nullptr}
{
#ifdef WIN32
  gsl_Assert(_putenv_s("PYTHONHOME", "") == 0);
  gsl_Assert(_putenv_s("PYTHONPATH", "") == 0);
#else
  gsl_Assert(setenv("PYTHONHOME", "", true) == 0);
  gsl_Assert(setenv("PYTHONPATH", "", true) == 0);
#endif

  m_interpreter = std::make_unique<pybind11::scoped_interpreter>();
  pybind11::module::import("sys").attr("path").cast<pybind11::list>().append(
    std::filesystem::absolute(rootPath).string());

  try
  {
    pybind11::eval_file(util::ensureFileExists(rootPath / "__init__.py").string());
  }
  catch(std::exception& e)
  {
    BOOST_LOG_TRIVIAL(fatal) << "Failed to load __init__.py: " << e.what();
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to load __init__.py"));
  }
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
const Gameflow& ScriptEngine::getGameflow() const
{
  return *gsl::not_null{pybind11::globals()["gameflow"].cast<Gameflow*>()};
}
} // namespace engine::script
