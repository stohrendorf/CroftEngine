#pragma once

#include "level.h"

namespace level
{
class TR5Level : public Level
{
public:
    TR5Level(Game gameVersion, loader::io::SDLReader&& reader)
        : Level(gameVersion, std::move(reader))
    {
    }

    void load(irr::video::IVideoDriver* drv) override;
};
} // namespace loader
