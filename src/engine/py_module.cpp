#include "ai/py_module.h"

#include "core/i18n.h"
#include "core/pybindmodule.h"
#include "engine/objects/objectstate.h"
#include "items_tr1.h"
#include "script/reflection.h"
#include "soundeffects_tr1.h"

#include <memory>

namespace py = pybind11;

PYBIND11_EMBEDDED_MODULE(engine, m)
{
  m.doc() = "edisonengine engine module";
  engine::ai::initAiModule(m.def_submodule("ai"));

  py::class_<engine::script::TrackInfo>(m, "TrackInfo").def(py::init<core::SoundEffectId::type, audio::TrackType>());

  py::class_<engine::script::ObjectInfo>(m, "ObjectInfo")
    .def(py::init<>())
    .def_readwrite("ai_agent", &engine::script::ObjectInfo::ai_agent)
    .def_readwrite("radius", &engine::script::ObjectInfo::radius)
    .def_readwrite("hit_points", &engine::script::ObjectInfo::hit_points)
    .def_readwrite("pivot_length", &engine::script::ObjectInfo::pivot_length)
    .def_readwrite("target_update_chance", &engine::script::ObjectInfo::target_update_chance);

  // NOLINTNEXTLINE(bugprone-unused-raii)
  py::class_<engine::script::LevelSequenceItem, std::shared_ptr<engine::script::LevelSequenceItem>>(
    m, "LevelSequenceItem", py::is_final{});
  py::class_<engine::script::Video, engine::script::LevelSequenceItem, std::shared_ptr<engine::script::Video>>(
    m, "Video", py::is_final{})
    .def(py::init<std::string>());

  py::class_<engine::script::Cutscene, engine::script::LevelSequenceItem, std::shared_ptr<engine::script::Cutscene>>(
    m, "Cutscene", py::is_final{})
    .def(py::init<std::string, engine::TR1TrackId, bool, bool, float>(),
         py::kw_only{},
         py::arg("name"),
         py::arg("track"),
         py::arg("flip_rooms") = false,
         py::arg("weapon_swap") = false,
         py::arg("camera_rot"))
    .def(py::init<std::string, engine::TR1TrackId, bool, bool, float, int, int>(),
         py::kw_only{},
         py::arg("name"),
         py::arg("track"),
         py::arg("flip_rooms") = false,
         py::arg("weapon_swap") = false,
         py::arg("camera_rot"),
         py::arg("camera_pos_x"),
         py::arg("camera_pos_z"));

  py::class_<engine::script::Level, engine::script::LevelSequenceItem, std::shared_ptr<engine::script::Level>>(
    m, "Level", py::is_final{})
    .def(py::init<std::string,
                  size_t,
                  bool,
                  std::unordered_map<std::string, std::string>,
                  std::unordered_map<std::string, std::unordered_map<engine::TR1ItemId, std::string>>,
                  std::unordered_map<engine::TR1ItemId, size_t>,
                  std::unordered_set<engine::TR1ItemId>,
                  std::optional<engine::TR1TrackId>,
                  bool>(),
         py::kw_only{},
         py::arg("name"),
         py::arg("secrets"),
         py::arg("use_alternative_lara") = false,
         py::arg("titles"),
         py::arg("item_titles") = py::dict{},
         py::arg("inventory") = py::dict{},
         py::arg("drop_inventory") = py::set{},
         py::arg("track") = std::nullopt,
         py::arg("allow_save") = true);

  py::class_<engine::script::TitleMenu, engine::script::Level, std::shared_ptr<engine::script::TitleMenu>>(
    m, "TitleMenu", py::is_final{})
    .def(py::init<std::string,
                  bool,
                  std::unordered_map<std::string, std::string>,
                  std::unordered_map<std::string, std::unordered_map<engine::TR1ItemId, std::string>>,
                  std::unordered_map<engine::TR1ItemId, size_t>,
                  std::unordered_set<engine::TR1ItemId>,
                  std::optional<engine::TR1TrackId>>(),
         py::kw_only{},
         py::arg("name"),
         py::arg("use_alternative_lara") = false,
         py::arg("titles"),
         py::arg("item_titles") = py::dict{},
         py::arg("inventory") = py::dict{},
         py::arg("drop_inventory") = py::set{},
         py::arg("track") = std::nullopt);

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
    auto e = py::enum_<engine::TR1SoundEffect>(m, "TR1SoundEffect");
    for(const auto& [key, value] : engine::EnumUtil<engine::TR1SoundEffect>::all())
      e.value(value.c_str(), key);
  }

  {
    auto e = py::enum_<engine::TR1TrackId>(m, "TR1TrackId");
    for(const auto& [key, value] : engine::EnumUtil<engine::TR1TrackId>::all())
      e.value(value.c_str(), key);
  }

  {
    auto e = py::enum_<engine::TR1ItemId>(m, "TR1ItemId");
    for(const auto& [key, value] : engine::EnumUtil<engine::TR1ItemId>::all())
      e.value(value.c_str(), key);
  }
}
