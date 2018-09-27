#pragma once

#include "level.h"

namespace level
{
class TR1Level : public Level
{
public:
    TR1Level(const Game gameVersion, loader::io::SDLReader&& reader, sol::state&& scriptEngine)
            : Level{gameVersion, std::move( reader ), std::move( scriptEngine )}
    {
    }

    void loadFileData() override;
};
} // namespace loader
