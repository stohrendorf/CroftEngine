#pragma once

#include <cstdint>

namespace engine
{
enum class LevelLoopResult : uint8_t
{
  /// Advance to the next level in the sequence.
  NextLevel,
  /// Return to the title screen.
  TitleLevel,
  /// Go to Lara's Home (Gym).
  LaraHomeLevel,
  /// Quit the application.
  ExitGame,
  /// Load a savegame from a slot.
  RequestLoad,
  /// Jump to a specific level in the sequence.
  RequestLevel,
  /// Restart the current level.
  RestartLevel
};
} // namespace engine