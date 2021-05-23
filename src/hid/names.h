#pragma once

#include <cstdint>
#include <string>

namespace hid
{
enum class GlfwKey : int32_t;
enum class GlfwGamepadButton : int32_t;
enum class Action : int32_t;

extern std::string getName(GlfwKey key);
extern std::string getName(GlfwGamepadButton button);
extern std::string getName(Action action);
} // namespace hid
