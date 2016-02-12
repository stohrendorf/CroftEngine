#pragma once

#include "level.h"

namespace loader
{
class TR4Level : public Level
{
public:
    TR4Level(Game gameVersion, io::SDLReader&& reader)
        : Level(gameVersion, std::move(reader))
    {
    }

    void load(irr::video::IVideoDriver* drv) override;
};
} // namespace loader
