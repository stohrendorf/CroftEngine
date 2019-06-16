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
    TR5Level(const Game gameVersion, loader::file::io::SDLReader&& reader)
        : Level{gameVersion, std::move(reader)}
    {
    }

    void loadFileData() override;
};
} // namespace level
} // namespace file
} // namespace loader
