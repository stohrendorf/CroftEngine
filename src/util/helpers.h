#pragma once

#include "core/angle.h"
#include "core/vec.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <gsl-lite.hpp>

namespace util
{
template<typename T>
T clamp(const T& v, const T& min, const T& max)
{
  if(v < min)
  {
    return min;
  }
  if(max < v)
  {
    return max;
  }
  return v;
}

template<typename T>
constexpr auto square(T value)
{
  return value * value;
}

constexpr int Rand15Max = 1 << 15;

inline int16_t rand15()
{
  return gsl::narrow_cast<int16_t>(std::rand() & (Rand15Max - 1));
}

template<typename T>
inline T rand15(T max)
{
  return max * rand15() / Rand15Max;
}

template<typename T, typename U>
inline auto rand15(qs::quantity<T, U> max)
{
  return max * U(rand15()) / U(Rand15Max);
}

inline int16_t rand15s()
{
  return static_cast<int16_t>(rand15() - Rand15Max / 2);
}

template<typename T, typename U>
inline auto rand15s(qs::quantity<T, U> max)
{
  return max * U(rand15s()) / U(Rand15Max);
}

template<typename T>
inline T rand15s(T max)
{
  return max * rand15s() / Rand15Max;
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
  return (len.retype_as<float>() * sin(rot)).retype_as<core::Length>();
}

inline core::Length cos(const core::Length& len, const core::Angle& rot)
{
  return (len.retype_as<float>() * cos(rot)).retype_as<core::Length>();
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
  const auto sin = core::sin(rot);
  const auto cos = core::cos(rot);
  return core::TRVec{(vec.Z.retype_as<float>() * sin + vec.X.retype_as<float>() * cos).retype_as<core::Length>(),
                     vec.Y,
                     (vec.Z.retype_as<float>() * cos - vec.X.retype_as<float>() * sin).retype_as<core::Length>()};
}
} // namespace util
