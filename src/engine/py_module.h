#pragma once

#include "ai/py_module.h"
#include "items_tr1.h"
#include "loader/file/level/game.h"
#include "objects/objectstate.h"
#include "objects/py_module.h"
#include "qs/quantity.h"
#include "script/reflection.h"
#include "soundeffects_tr1.h"
#include "tracks_tr1.h"
#include "weapontype.h"

#include <cstddef>
#include <map>
#include <memory>
#include <optional>
#include <pybind11/attr.h>
#include <pybind11/cast.h>
#include <pybind11/pytypes.h>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace engine
{
inline void initEngineModule(pybind11::module& m)
{
  namespace py = pybind11;

  ai::initAiModule(m.def_submodule("ai"));

  py::class_<script::TrackInfo, std::shared_ptr<script::TrackInfo>>(m, "TrackInfo")
    .def(py::init<std::vector<std::string>, size_t, bool, uint32_t>(),
         py::arg("paths"),
         py::arg("slot"),
         py::arg("looping"),
         py::arg("fade_duration_seconds") = 0);

  {
    auto e = py::enum_<WeaponType>(m, "WeaponType");
#define EXPOSE_ENUM_MEMBER(n) e.value(#n, WeaponType::n)
    e.value("None_", WeaponType::None);
    EXPOSE_ENUM_MEMBER(Pistols);
    EXPOSE_ENUM_MEMBER(Magnums);
    EXPOSE_ENUM_MEMBER(Uzis);
    EXPOSE_ENUM_MEMBER(Shotgun);
#undef EXPOSE_ENUM_MEMBER
  }

  py::class_<script::ObjectInfo, script::PyObjectInfo, std::shared_ptr<script::ObjectInfo>>(m, "ObjectInfo")
    .def(py::init<>())
    .def_readwrite("ai_agent", &script::ObjectInfo::ai_agent)
    .def_readwrite("radius", &script::ObjectInfo::radius)
    .def_readwrite("hit_points", &script::ObjectInfo::hit_points)
    .def_readwrite("pivot_length", &script::ObjectInfo::pivot_length)
    .def_readwrite("target_update_chance", &script::ObjectInfo::target_update_chance)
    .def_readwrite("step_limit", &script::ObjectInfo::step_limit)
    .def_readwrite("drop_limit", &script::ObjectInfo::drop_limit)
    .def_readwrite("fly_limit", &script::ObjectInfo::fly_limit)
    .def_readwrite("cannot_visit_blocked", &script::ObjectInfo::cannot_visit_blocked)
    .def_readwrite("cannot_visit_blockable", &script::ObjectInfo::cannot_visit_blockable);

  // NOLINTNEXTLINE(bugprone-unused-raii)
  py::class_<script::LevelSequenceItem, std::shared_ptr<script::LevelSequenceItem>>(
    m, "LevelSequenceItem", py::is_final{});
  py::class_<script::Video, script::LevelSequenceItem, std::shared_ptr<script::Video>>(m, "Video", py::is_final{})
    .def(py::init<std::vector<std::string>, bool>(), py::kw_only{}, py::arg("paths"), py::arg("optional") = false);

  py::class_<script::Cutscene, script::LevelSequenceItem, std::shared_ptr<script::Cutscene>>(
    m, "Cutscene", py::is_final{})
    .def(py::init<std::string, TR1TrackId, bool, bool, float, loader::file::level::Game>(),
         py::kw_only{},
         py::arg("name"),
         py::arg("track"),
         py::arg("flip_rooms") = false,
         py::arg("weapon_swap") = false,
         py::arg("camera_rot"),
         py::arg("game") = loader::file::level::Game::Unknown)
    .def(py::init<std::string, TR1TrackId, bool, bool, float, int, int, loader::file::level::Game>(),
         py::kw_only{},
         py::arg("name"),
         py::arg("track"),
         py::arg("flip_rooms") = false,
         py::arg("weapon_swap") = false,
         py::arg("camera_rot"),
         py::arg("camera_pos_x"),
         py::arg("camera_pos_z"),
         py::arg("game") = loader::file::level::Game::Unknown);

  py::class_<script::Level, script::LevelSequenceItem, std::shared_ptr<script::Level>>(m, "Level", py::is_final{})
    .def(py::init<std::string,
                  bool,
                  std::unordered_map<std::string, std::string>,
                  std::unordered_map<std::string, std::unordered_map<TR1ItemId, std::string>>,
                  std::optional<TR1TrackId>,
                  bool,
                  WeaponType,
                  std::tuple<float, float, float>,
                  float,
                  std::optional<std::string>,
                  loader::file::level::Game>(),
         py::kw_only{},
         py::arg("name"),
         py::arg("use_alternative_lara") = false,
         py::arg("titles"),
         py::arg("item_titles") = py::dict{},
         py::arg("ambient") = std::nullopt,
         py::arg("allow_save") = true,
         py::arg("default_weapon") = WeaponType::Pistols,
         py::arg("water_color") = script::Level::DefaultWaterColor,
         py::arg("water_density") = script::Level::DefaultWaterDensity,
         py::arg("alternative_splashscreen") = std::nullopt,
         py::arg("game") = loader::file::level::Game::Unknown);

  py::class_<script::ModifyInventory, script::LevelSequenceItem, std::shared_ptr<script::ModifyInventory>>(
    m, "ModifyInventory", py::is_final{})
    .def(py::init<std::unordered_map<TR1ItemId, size_t>, std::unordered_set<TR1ItemId>>(),
         py::kw_only{},
         py::arg("add_inventory") = py::dict{},
         py::arg("drop_inventory") = py::set{});

  py::class_<script::TitleMenu, script::Level, std::shared_ptr<script::TitleMenu>>(m, "TitleMenu", py::is_final{})
    .def(py::init<std::string,
                  bool,
                  std::unordered_map<std::string, std::string>,
                  std::unordered_map<std::string, std::unordered_map<TR1ItemId, std::string>>,
                  std::optional<TR1TrackId>,
                  loader::file::level::Game>(),
         py::kw_only{},
         py::arg("name"),
         py::arg("use_alternative_lara") = false,
         py::arg("titles"),
         py::arg("item_titles") = py::dict{},
         py::arg("ambient") = std::nullopt,
         py::arg("game") = loader::file::level::Game::Unknown);

  py::class_<script::SplashScreen, script::LevelSequenceItem, std::shared_ptr<script::SplashScreen>>(
    m, "SplashScreen", py::is_final{})
    .def(py::init<std::string, int, int, int>(),
         py::kw_only{},
         py::arg("path"),
         py::arg("duration_seconds"),
         py::arg("fade_in_duration_seconds"),
         py::arg("fade_out_duration_seconds"));

  py::class_<script::StopAudioSlot, script::LevelSequenceItem, std::shared_ptr<script::StopAudioSlot>>(
    m, "StopAudioSlot", py::is_final{})
    .def(py::init<size_t>(), py::kw_only{}, py::arg("slot"));

  py::class_<script::PlayAudioSlot, script::LevelSequenceItem, std::shared_ptr<script::PlayAudioSlot>>(
    m, "PlayAudioSlot", py::is_final{})
    .def(py::init<size_t, TR1TrackId>(), py::kw_only{}, py::arg("slot"), py::arg("track"));

  py::class_<script::ResetSoundEngine, script::LevelSequenceItem, std::shared_ptr<script::ResetSoundEngine>>(
    m, "ResetSoundEngine", py::is_final{})
    .def(py::init<>());

  py::enum_<objects::TriggerState>(m, "ActivationState")
    .value("INACTIVE", objects::TriggerState::Inactive)
    .value("DEACTIVATED", objects::TriggerState::Deactivated)
    .value("INVISIBLE", objects::TriggerState::Invisible);

  {
    auto e = py::enum_<TR1SoundEffect>(m, "TR1SoundEffect");
    for(const auto& [key, value] : EnumUtil<TR1SoundEffect>::all())
      e.value(value.c_str(), key);
  }

  {
    auto e = py::enum_<TR1TrackId>(m, "TR1TrackId");
    for(const auto& [key, value] : EnumUtil<TR1TrackId>::all())
      e.value(value.c_str(), key);
  }

  {
    auto e = py::enum_<TR1ItemId>(m, "TR1ItemId");
    for(const auto& [key, value] : EnumUtil<TR1ItemId>::all())
      e.value(value.c_str(), key);
  }

  py::class_<script::Gameflow>(m, "Gameflow", py::is_final{})
    .def(py::init<std::map<TR1ItemId, std::shared_ptr<script::ObjectInfo>>,
                  std::map<TR1TrackId, std::shared_ptr<script::TrackInfo>>,
                  std::vector<std::shared_ptr<script::LevelSequenceItem>>,
                  std::shared_ptr<script::LevelSequenceItem>,
                  std::string,
                  std::vector<std::shared_ptr<script::LevelSequenceItem>>,
                  std::vector<std::shared_ptr<script::LevelSequenceItem>>,
                  pybind11::dict,
                  std::string>(),
         py::kw_only{},
         py::arg("object_infos"),
         py::arg("tracks"),
         py::arg("level_sequence"),
         py::arg("title_menu"),
         py::arg("title_menu_backdrop"),
         py::arg("lara_home"),
         py::arg("early_boot"),
         py::arg("cheats"),
         py::arg("asset_root"));

  auto objectsMod = m.def_submodule("objects", "croftengine objects module");
  objects::initObjectsModule(objectsMod);
}
} // namespace engine
