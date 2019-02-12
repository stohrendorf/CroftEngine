#pragma once

#include "level.h"

namespace loader
{
namespace file
{
namespace level
{
class TR5Level : public Level
{
public:
    TR5Level(const Game gameVersion, loader::file::io::SDLReader&& reader, sol::state&& scriptEngine)
            : Level{gameVersion, std::move( reader ), std::move( scriptEngine )}
    {
    }

    void loadFileData() override;
};
}
}
}
