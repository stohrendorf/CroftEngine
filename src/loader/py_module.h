#pragma once

#include "file/py_module.h"

#include <pybind11/pybind11.h>

namespace loader
{
inline void initLoaderModule(pybind11::module& m)
{
  namespace py = pybind11;

  auto fileModule = m.def_submodule("file");
  loader::file::initFileModule(fileModule);
}
} // namespace loader
