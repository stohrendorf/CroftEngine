#pragma once

#include "magic.h"
#include "qs/quantity.h"
#include "units.h"
#include "vec.h"

#include <pybind11/cast.h>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>

namespace core
{
inline void initCoreModule(pybind11::module& m)
{
  namespace py = pybind11;

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
} // namespace core
