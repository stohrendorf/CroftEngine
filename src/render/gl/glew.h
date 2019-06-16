#pragma once

#include <glbinding/gl/gl.h>

namespace render
{
namespace gl
{
using RawGlEnum = std::underlying_type_t<::gl::GLenum>;

extern void initializeGl();
}
}
