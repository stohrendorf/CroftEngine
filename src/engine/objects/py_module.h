#pragma once

#include "doppelganger.h"
#include "object.h"

#include <pybind11/attr.h>
#include <pybind11/cast.h>
#include <pybind11/pytypes.h>

namespace engine::objects
{
inline void initObjectsModule(pybind11::module& m)
{
  namespace py = pybind11;

  py::class_<Object, std::shared_ptr<Object>>(m, "Object");
  py::class_<Doppelganger, Object, std::shared_ptr<Doppelganger>>(m, "Doppelganger")
    .def("set_center", &Doppelganger::setCenter, py::arg("x"), py::arg("z"));
}
} // namespace engine::objects
