#pragma once

namespace gl
{
extern void initializeGl(void* (*loadProc)(const char* name));
extern bool hasAnisotropicFilteringExtension();
extern float getMaxAnisotropyLevel();
} // namespace gl
