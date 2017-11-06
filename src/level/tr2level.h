#pragma once

#include "level.h"

namespace level
{
class TR2Level : public Level
{
public:
    TR2Level(Game gameVersion, loader::io::SDLReader&& reader, sol::state&& scriptEngine)
            : Level( gameVersion, std::move( reader ), std::move( scriptEngine ) )
    {
    }

    void loadFileData() override;
};
} // namespace loader
