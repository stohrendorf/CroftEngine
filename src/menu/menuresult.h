#pragma once

#include <cstdint>

namespace menu
{
enum class MenuResult : uint8_t
{
  /// No action requested.
  None,
  /// Menu closed by the user.
  Closed,
  /// Request to exit to the title screen.
  ExitToTitle,
  /// Request to exit the application.
  ExitGame,
  /// Request to start a new game (Title -> Level 0, or reset current game).
  NewGame,
  /// Request to restart the current level.
  RestartLevel,
  /// Request to visit Lara's Home (Gym).
  LaraHome,
  /// Request to load a savegame from a slot (see MenuDisplay::requestLoad).
  RequestLoad,
  /// Request to jump to a specific level (see MenuDisplay::requestLevelSequenceIndex).
  RequestLevel
};
} // namespace menu