#include "core/pybindmodule.h"
#include "vec.h"

namespace py = pybind11;

PYBIND11_EMBEDDED_MODULE(core, m)
{
  m.doc() = "edisonengine core module";

  py::class_<core::Length>(m, "Length")
    .def(py::init<core::Length::type>())
    .def_property_readonly("value", &core::Length::get<>);

  py::class_<core::TRVec>(m, "Vec")
    .def(py::init<>())
    .def(py::init<core::Length, core::Length, core::Length>(), py::arg("x"), py::arg("y"), py::arg("z"))
    .def_readwrite("x", &core::TRVec::X)
    .def_readwrite("y", &core::TRVec::Y)
    .def_readwrite("z", &core::TRVec::Z);
}
