#pragma once

#include "ai.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace engine::ai
{
namespace py = pybind11;

void initAiModule(const py::module& m)
{
  py::class_<CreatureInfo>(m, "CreatureInfo")
    .def_readwrite("head_rotation", &CreatureInfo::head_rotation)
    .def_readwrite("neck_rotation", &CreatureInfo::neck_rotation)
    .def_readwrite("maximum_turn", &CreatureInfo::maximum_turn)
    .def_readwrite("flags", &CreatureInfo::flags)
    .def_readwrite("mood", &CreatureInfo::mood)
    .def_readwrite("target", &CreatureInfo::target);

  py::enum_<engine::ai::Mood>(m, "Mood")
    .value("BORED", engine::ai::Mood::Bored)
    .value("ATTACK", engine::ai::Mood::Attack)
    .value("ESCAPE", engine::ai::Mood::Escape)
    .value("STALK", engine::ai::Mood::Stalk);
}
} // namespace engine::ai
