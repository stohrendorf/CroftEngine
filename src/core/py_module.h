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

  py::class_<Length>(m, "Length")
    .def(py::init<Length::type>())
    .def_property_readonly("value", &Length::get<>);

  py::class_<TRVec>(m, "Vec")
    .def(py::init<>())
    .def(py::init<Length, Length, Length>(), py::arg("x"), py::arg("y"), py::arg("z"))
    .def_readwrite("x", &TRVec::X)
    .def_readwrite("y", &TRVec::Y)
    .def_readwrite("z", &TRVec::Z);

  m.attr("QuarterSectorSize") = QuarterSectorSize.get();
  m.attr("SectorSize") = SectorSize.get();
}
} // namespace core
