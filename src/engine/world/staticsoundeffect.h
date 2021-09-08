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
  const std::weak_ptr<audio::Voice> voice;
  const bool playIfSwapped;
  const bool playIfNotSwapped;
};
} // namespace engine::world
