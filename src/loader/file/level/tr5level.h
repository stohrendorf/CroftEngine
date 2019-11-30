#pragma once

#include "level.h"

namespace loader::file::level
{
class TR5Level : public Level
{
public:
  TR5Level(const Game gameVersion, io::SDLReader&& reader)
      : Level{gameVersion, std::move(reader)}
  {
  }

  void loadFileData() override;
};
} // namespace loader
