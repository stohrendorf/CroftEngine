#include "ai/py_module.h"

#include "core/pybindmodule.h"
#include "engine/objects/objectstate.h"
#include "items_tr1.h"
#include "script/reflection.h"
#include "sounds_tr1.h"
#include "tracks_tr1.h"

namespace py = pybind11;

PYBIND11_EMBEDDED_MODULE(engine, m)
{
  m.doc() = "edisonengine engine module";
  engine::ai::initAiModule(m.def_submodule("ai"));

  py::class_<engine::script::TrackInfo>(m, "TrackInfo").def(py::init<core::SoundId::type, audio::TrackType>());

  py::class_<engine::script::ObjectInfo>(m, "ObjectInfo")
    .def(py::init<>())
    .def_readwrite("ai_agent", &engine::script::ObjectInfo::ai_agent)
    .def_readwrite("radius", &engine::script::ObjectInfo::radius)
    .def_readwrite("hit_points", &engine::script::ObjectInfo::hit_points)
    .def_readwrite("pivot_length", &engine::script::ObjectInfo::pivot_length)
    .def_readwrite("target_update_chance", &engine::script::ObjectInfo::target_update_chance);

  py::enum_<engine::objects::TriggerState>(m, "ActivationState")
    .value("INACTIVE", engine::objects::TriggerState::Inactive)
    .value("DEACTIVATED", engine::objects::TriggerState::Deactivated)
    .value("INVISIBLE", engine::objects::TriggerState::Invisible);

  py::enum_<audio::TrackType>(m, "TrackType")
    .value("AMBIENT", audio::TrackType::Ambient)
    .value("INTERCEPTION", audio::TrackType::Interception)
    .value("AMBIENT_EFFECT", audio::TrackType::AmbientEffect)
    .value("LARA_TALK", audio::TrackType::LaraTalk);

  {
    auto e = py::enum_<engine::TR1SoundId>(m, "TR1SoundId");
    for(const auto& entry : engine::EnumUtil<engine::TR1SoundId>::all())
      e.value(entry.second.c_str(), entry.first);
  }

  {
    auto e = py::enum_<engine::TR1TrackId>(m, "TR1TrackId");
    for(const auto& entry : engine::EnumUtil<engine::TR1TrackId>::all())
      e.value(entry.second.c_str(), entry.first);
  }

  {
    auto e = py::enum_<engine::TR1ItemId>(m, "TR1ItemId");
    for(const auto& entry : engine::EnumUtil<engine::TR1ItemId>::all())
      e.value(entry.second.c_str(), entry.first);
  }
}
