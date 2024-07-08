#pragma once

#include <cstdint>

namespace loader::file::level
{
enum class Game : uint8_t
{
  TR1,
  TR1Demo,
  TR1UnfinishedBusiness,
  TR2,
  TR2Demo,
  TR2Gold,
  TR3,
  TR3Gold,
  TR4,
  TR4Demo,
  TR5,
  Unknown
};

enum class Engine : uint8_t
{
  TR1,
  TR2,
  TR3,
  TR4,
  TR5,
  Unknown
};

inline Engine gameToEngine(const Game game)
{
  switch(game)
  {
  case Game::TR1:
  case Game::TR1Demo:
  case Game::TR1UnfinishedBusiness:
    return Engine::TR1;
  case Game::TR2:
  case Game::TR2Demo:
  case Game::TR2Gold:
    return Engine::TR2;
  case Game::TR3:
  case Game::TR3Gold:
    return Engine::TR3;
  case Game::TR4:
  case Game::TR4Demo:
    return Engine::TR4;
  case Game::TR5:
    return Engine::TR5;
  default:
    return Engine::Unknown;
  }
}
} // namespace loader::file::level
