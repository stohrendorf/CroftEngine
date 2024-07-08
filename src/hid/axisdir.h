#pragma once

#include "glfw_axes.h"
#include "glfw_axis_dirs.h"

#include <utility>

namespace hid
{
using AxisDir = std::pair<GlfwAxis, GlfwAxisDir>;
}
