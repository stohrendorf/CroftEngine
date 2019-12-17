#pragma once

#include "level.h"

namespace loader::file::level
{
class TR2Level : public Level
{
public:
  TR2Level(const Game gameVersion, io::SDLReader&& reader)
      : Level{gameVersion, std::move(reader)}
  {
  }

  void loadFileData() override;
};
} // namespace loader::file::level
