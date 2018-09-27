#pragma once

#include "level.h"

namespace level
{
class TR4Level : public Level
{
public:
    TR4Level(const Game gameVersion, loader::io::SDLReader&& reader, sol::state&& scriptEngine)
            : Level{gameVersion, std::move( reader ), std::move( scriptEngine )}
    {
    }

    void loadFileData() override;
};
} // namespace loader
