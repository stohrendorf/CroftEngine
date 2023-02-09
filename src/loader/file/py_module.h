#pragma once

#include "level/py_module.h"

#include <pybind11/pybind11.h>

namespace loader::file
{
inline void initFileModule(pybind11::module& m)
{
  namespace py = pybind11;

  m.doc() = "file module";

  auto levelModule = m.def_submodule("level");
  level::initLevelModule(levelModule);
}
} // namespace loader::file
