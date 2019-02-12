#pragma once

#include "level/level.h"

namespace loader
{
namespace file
{
namespace level
{
class TR3Level : public ::level::Level
{
public:
    TR3Level(const ::level::Game gameVersion, io::SDLReader&& reader, sol::state&& scriptEngine)
            : Level{gameVersion, std::move( reader ), std::move( scriptEngine )}
    {
    }

    void loadFileData() override;
};
}
}
}
