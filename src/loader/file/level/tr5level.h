#pragma once

#include "level/level.h"

namespace loader
{
namespace file
{
namespace level
{
class TR5Level : public ::level::Level
{
public:
    TR5Level(const ::level::Game gameVersion, loader::file::io::SDLReader&& reader, sol::state&& scriptEngine)
            : Level{gameVersion, std::move( reader ), std::move( scriptEngine )}
    {
    }

    void loadFileData() override;
};
}
}
}
