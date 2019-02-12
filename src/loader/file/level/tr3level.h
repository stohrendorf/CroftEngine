#pragma once

#include "level.h"

namespace loader
{
namespace file
{
namespace level
{
class TR3Level : public Level
{
public:
    TR3Level(const Game gameVersion, io::SDLReader&& reader, sol::state&& scriptEngine)
            : Level{gameVersion, std::move( reader ), std::move( scriptEngine )}
    {
    }

    void loadFileData() override;
};
}
}
}
