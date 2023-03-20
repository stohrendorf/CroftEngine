#pragma once

#include <memory>

namespace audio
{
class Voice;
}

namespace engine::world
{
struct StaticSoundEffect
{
  std::weak_ptr<audio::Voice> voice;
  bool playIfSwapped;
  bool playIfNotSwapped;
};
} // namespace engine::world
