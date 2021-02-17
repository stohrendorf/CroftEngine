#include "actions.h"
#include "glfw_gamepad_buttons.h"
#include "glfw_keys.h"

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

PYBIND11_EMBEDDED_MODULE(hid, m)
{
  m.doc() = "edisonengine hid module";

  {
    auto e = py::enum_<hid::GlfwKey>(m, "GlfwKey");
    for(const auto& [key, value] : hid::EnumUtil<hid::GlfwKey>::all())
      e.value(value.c_str(), key);
  }

  {
    auto e = py::enum_<hid::GlfwGamepadButton>(m, "GlfwGamepadButton");
    for(const auto& [key, value] : hid::EnumUtil<hid::GlfwGamepadButton>::all())
      e.value(value.c_str(), key);
  }

  {
    auto e = py::enum_<hid::Action>(m, "Action");
    for(const auto& [key, value] : hid::EnumUtil<hid::Action>::all())
      e.value(value.c_str(), key);
  }
}
