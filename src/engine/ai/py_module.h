#pragma once

#include "ai.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

namespace engine::ai
{
inline void initAiModule(const pybind11::module& m)
{
  namespace py = pybind11;

  py::class_<CreatureInfo>(m, "CreatureInfo")
    .def_readwrite("head_rotation", &CreatureInfo::headRotation)
    .def_readwrite("neck_rotation", &CreatureInfo::neckRotation)
    .def_readwrite("max_turn_speed", &CreatureInfo::maxTurnSpeed)
    .def_readwrite("mood", &CreatureInfo::mood)
    .def_readwrite("target", &CreatureInfo::target);

  py::enum_<engine::ai::Mood>(m, "Mood")
    .value("BORED", engine::ai::Mood::Bored)
    .value("ATTACK", engine::ai::Mood::Attack)
    .value("ESCAPE", engine::ai::Mood::Escape)
    .value("STALK", engine::ai::Mood::Stalk);
}
} // namespace engine::ai
