#pragma once

#include "core/angle.h"
#include "core/units.h"
#include "core/vec.h"
#include "qs/qs.h"

#include <cstdint>
#include <filesystem>
#include <glm/mat4x4.hpp>
#include <string>
#include <type_traits>

namespace util
{
template<typename T>
constexpr auto square(T value)
{
  return value * value;
}

constexpr int Rand15Max = 1u << 15u;

/**
 * Random number in range 0..32767.
 */
extern int16_t rand15();

template<typename T>
inline T rand15(T max)
{
  return static_cast<T>(static_cast<float>(max) * static_cast<float>(rand15()) / static_cast<float>(Rand15Max));
}

template<typename T, typename U>
inline auto rand15(qs::quantity<T, U> max)
{
  return (max.template cast<float>() * static_cast<float>(rand15()) / static_cast<float>(Rand15Max)).template cast<U>();
}

/**
 * Random number in range -16384..16383.
 */
extern int16_t rand15s();

/**
 * Random number in range -max/2..max/2
 */
template<typename T, typename U>
inline auto rand15s(qs::quantity<T, U> max)
{
  return (max.template cast<float>() * static_cast<float>(rand15s()) / static_cast<float>(Rand15Max))
    .template cast<U>();
}

template<typename T>
inline T rand15s(T max)
{
  return static_cast<T>(static_cast<float>(max) * static_cast<float>(rand15s()) / static_cast<float>(Rand15Max));
}

extern glm::mat4 mix(const glm::mat4& a, const glm::mat4& b, const float bias);

extern core::Length sin(const core::Length& len, const core::Angle& rot);

extern core::Length cos(const core::Length& len, const core::Angle& rot);

extern core::TRVec pitch(const core::Length& len, const core::Angle& rot, const core::Length& dy = 0_len);

inline core::TRVec yawPitch(const core::Length& len, const core::TRRotation& rot)
{
  const auto d = cos(len, rot.X);
  return core::TRVec{sin(d, rot.Y), -sin(len, rot.X), cos(d, rot.Y)};
}

extern core::TRVec pitch(const core::TRVec& vec, const core::Angle& rot);

template<typename T>
auto bits(T value, uint8_t shr, uint8_t n) -> std::enable_if_t<std::is_unsigned_v<T>, T>
{
  return static_cast<T>(value >> shr) & ((1u << n) - 1u);
}

extern std::string unescape(const std::string& escaped);
extern std::string escape(const std::string& escaped);

extern std::filesystem::path ensureFileExists(const std::filesystem::path& path);
} // namespace util
