#include "file/py_module.h"

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

// NOLINTNEXTLINE(cert-err58-cpp)
PYBIND11_EMBEDDED_MODULE(loader, m)
{
  m.doc() = "loader module";

  auto fileModule = m.def_submodule("file");
  loader::file::initFileModule(fileModule);
}
