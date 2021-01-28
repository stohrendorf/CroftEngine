#pragma once

#include "level.h"

namespace loader::file::level
{
class TR1Level : public Level
{
public:
  TR1Level(const Game gameVersion, io::SDLReader&& reader, std::filesystem::path filename)
      : Level{gameVersion, std::move(reader), std::move(filename)}
  {
  }

  void loadFileData() override;
};
} // namespace loader::file::level
