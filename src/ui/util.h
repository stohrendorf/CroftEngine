#pragma once

#include <glm/fwd.hpp>
#include <memory>
#include <string>

namespace ui
{
struct Label;

static constexpr char SpriteSelector = '\x1f';
static constexpr char SzLigSprite = '\x4a';
static constexpr char ArrowUpSprite = '\x50';
static constexpr char ArrowDownSprite = '\x51';
static constexpr char GamepadEmptyButtonSprite = '\x5c';
static constexpr char GamepadTriangleSprite = '\x5d';
static constexpr char GamepadCircleSprite = '\x5e';
static constexpr char GamepadCrossSprite = '\x5f';
static constexpr char GamepadSquareSprite = '\x60';
static constexpr char GamepadL1Sprite = '\x61';
static constexpr char GamepadR1Sprite = '\x62';
static constexpr char GamepadL2Sprite = '\x63';
static constexpr char GamepadR2Sprite = '\x64';
static constexpr char NoteSprite = '\x65';
static constexpr char SpeakerSprite = '\x66';
static constexpr char ShotgunAmmoSprite = '\x67';
static constexpr char MagnumsAmmoSprite = '\x68';
static constexpr char UzisAmmoSprite = '\x69';
static constexpr char GamepadDPadDownSprite = '\x6a';
static constexpr char GamepadDPadUpSprite = '\x6b';
static constexpr char GamepadDPadLeftSprite = '\x6c';
static constexpr char GamepadDPadRightSprite = '\x6d';

inline std::string getSpriteSelector(char sprite)
{
  return std::string{SpriteSelector} + sprite;
}

static constexpr char NumberSign = '@';
static constexpr char SzLig = '=';
static constexpr char SquareBracketLeft = '<';
static constexpr char SquareBracketRight = '>';
static constexpr char SmallNumber0 = '\x01';

static constexpr char ArrowUp = '[';
static constexpr char ArrowDown = ']';

static constexpr char UmlautDots = '~';
static constexpr char Acute1 = '(';
static constexpr char Acute2 = ')';
static constexpr char Gravis = '$';

extern std::unique_ptr<Label> createFrame(const glm::ivec2& position, const glm::ivec2& size);
extern std::unique_ptr<Label>
  createHeading(const std::string& heading, const glm::ivec2& position, const glm::ivec2& size);
} // namespace ui
