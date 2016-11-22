#pragma once

#include "level.h"

namespace level
{
class TR2Level : public Level
{
public:
    TR2Level(Game gameVersion, loader::io::SDLReader&& reader)
        : Level(gameVersion, std::move(reader))
    {
    }

    void load() override;
};
} // namespace loader
