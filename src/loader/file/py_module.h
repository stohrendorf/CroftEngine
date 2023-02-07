#include "level/py_module.h"

#include <pybind11/pybind11.h>

namespace loader::file
{
namespace py = pybind11;

inline void initFileModule(py::module& m)
{
  m.doc() = "file module";

  auto levelModule = m.def_submodule("level");
  level::initLevelModule(levelModule);
}
} // namespace loader::file
