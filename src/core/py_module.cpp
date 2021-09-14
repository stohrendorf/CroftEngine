#include "magic.h"
#include "qs/quantity.h"
#include "units.h"
#include "vec.h"

#include <pybind11/cast.h>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>

namespace py = pybind11;

// NOLINTNEXTLINE(cert-err58-cpp)
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

  m.attr("QuarterSectorSize") = core::QuarterSectorSize.get();
  m.attr("SectorSize") = core::SectorSize.get();
}
