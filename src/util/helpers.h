#pragma once

#include "core/angle.h"
#include "core/units.h"
#include "core/vec.h"
#include "qs/qs.h"

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <gsl/gsl-lite.hpp>
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

inline int16_t rand15()
{
  // NOLINTNEXTLINE(cert-msc50-cpp)
  return gsl::narrow_cast<int16_t>(std::rand() % Rand15Max);
}

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

inline int16_t rand15s()
{
  return static_cast<int16_t>(rand15() - Rand15Max / 2);
}

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

inline glm::mat4 mix(const glm::mat4& a, const glm::mat4& b, const float bias)
{
  glm::mat4 result{0.0f};
  const auto ap = value_ptr(a);
  const auto bp = value_ptr(b);
  const auto rp = value_ptr(result);
  for(int i = 0; i < 16; ++i)
    rp[i] = ap[i] * (1 - bias) + bp[i] * bias;
  return result;
}

inline core::Length sin(const core::Length& len, const core::Angle& rot)
{
  return (len.cast<float>() * sin(rot)).cast<core::Length>();
}

inline core::Length cos(const core::Length& len, const core::Angle& rot)
{
  return (len.cast<float>() * cos(rot)).cast<core::Length>();
}

inline core::TRVec pitch(const core::Length& len, const core::Angle& rot, const core::Length& dy = 0_len)
{
  return core::TRVec{sin(len, rot), dy, cos(len, rot)};
}

inline core::TRVec yawPitch(const core::Length& len, const core::TRRotation& rot)
{
  const auto d = cos(len, rot.X);
  return core::TRVec{sin(d, rot.Y), -sin(len, rot.X), cos(d, rot.Y)};
}

inline core::TRVec pitch(const core::TRVec& vec, const core::Angle& rot)
{
  const auto s = sin(rot);
  const auto c = cos(rot);
  return core::TRVec{(vec.Z.cast<float>() * s + vec.X.cast<float>() * c).cast<core::Length>(),
                     vec.Y,
                     (vec.Z.cast<float>() * c - vec.X.cast<float>() * s).cast<core::Length>()};
}

template<typename T>
auto bits(T value, uint8_t shr, uint8_t n) -> std::enable_if_t<std::is_unsigned_v<T>, T>
{
  return static_cast<T>(value >> shr) & ((1u << n) - 1u);
}

extern std::string unescape(const std::string& escaped);

extern std::filesystem::path ensureFileExists(const std::filesystem::path& path);
} // namespace util
