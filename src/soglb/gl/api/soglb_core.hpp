#pragma once

#include <cstdint>
#include <gsl-lite.hpp>
#include <type_traits>

struct __GLsync; // NOLINT(bugprone-reserved-identifier)

namespace gl::api
{
namespace core
{
using EnumType = uint32_t;
}

enum class DebugSource : core::EnumType;
enum class DebugType : core::EnumType;
enum class DebugSeverity : core::EnumType;

constexpr core::EnumType TextureUnitCount = 32;

namespace core
{
using SizeType = int32_t;
using ClampedFloat = float;
using ClampedDouble = double;
using Sync = __GLsync*;

struct Fixed
{
  int32_t value = 0;

  constexpr explicit operator int32_t() const noexcept
  {
    return value;
  }
};

struct Half
{
  // cppcheck-suppress unusedStructMember
  uint16_t __value = 0; // NOLINT(bugprone-reserved-identifier)
};

#ifdef __APPLE__
using Handle = void*;
#else
using Handle = uint32_t;
#endif

template<typename T>
class Bitfield final
{
  static_assert(std::is_same_v<std::underlying_type_t<T>, EnumType>, "Invalid bitfield template parameter");

  EnumType m_value{};

  // cppcheck-suppress noExplicitConstructor
  constexpr Bitfield(EnumType value) // NOLINT(google-explicit-constructor)
      : m_value{value}
  {
  }

public:
  // cppcheck-suppress noExplicitConstructor
  constexpr Bitfield(T value) noexcept // NOLINT(google-explicit-constructor)
      : m_value{static_cast<EnumType>(value)}
  {
  }

  constexpr Bitfield(const Bitfield<T>& rhs) noexcept
      : m_value{rhs.m_value}
  {
  }

  constexpr Bitfield() noexcept = default;

  [[nodiscard]] constexpr EnumType value() const noexcept
  {
    return m_value;
  }

  [[nodiscard]] constexpr bool isSet(T value) const noexcept
  {
    return (m_value & static_cast<EnumType>(value)) != 0;
  }

  constexpr Bitfield<T>& operator=(const Bitfield<T>& rhs) noexcept
  {
    m_value = rhs.m_value;
    return *this;
  }

  constexpr Bitfield<T> operator|(const Bitfield<T>& rhs) const noexcept
  {
    return Bitfield<T>{m_value | rhs.value()};
  }

  constexpr Bitfield<T>& operator|=(const Bitfield<T>& rhs) noexcept
  {
    m_value |= rhs.m_value;
    return *this;
  }

  constexpr Bitfield<T> operator|(T rhs) const noexcept
  {
    return Bitfield<T>{m_value | static_cast<EnumType>(rhs)};
  }

  constexpr Bitfield<T>& operator|=(T rhs) noexcept
  {
    m_value |= static_cast<EnumType>(rhs);
    return *this;
  }

  constexpr Bitfield<T> operator&(const Bitfield<T>& rhs) const noexcept
  {
    return Bitfield<T>{m_value & rhs.value()};
  }

  constexpr Bitfield<T>& operator&=(const Bitfield<T>& rhs) noexcept
  {
    m_value &= rhs.m_value;
    return *this;
  }
};

#ifdef _MSC_VER
#  define SOGLB_API __stdcall
#else
#  define SOGLB_API
#endif
using DebugProc = void(SOGLB_API*)(DebugSource source,
                                   DebugType type,
                                   uint32_t id,
                                   DebugSeverity severity,
                                   core::SizeType length,
                                   gsl::czstring message,
                                   const void* userParam);

} // namespace core

namespace detail
{
template<typename T>
class ConstAway final
{
public:
  explicit constexpr ConstAway(T* ptr)
      : m_ptr{ptr}
  {
  }

  constexpr operator T*() const noexcept // NOLINT(google-explicit-constructor)
  {
    return m_ptr;
  }

  template<typename U>
  [[deprecated]] constexpr operator U*() const noexcept // NOLINT(google-explicit-constructor)
  {
    return const_cast<U*>(m_ptr); // NOLINT(cppcoreguidelines-pro-type-const-cast)
  }

private:
  T* m_ptr;
};

template<typename T>
constexpr auto constAway(T* ptr)
{
  return ConstAway<T>(ptr);
}
} // namespace detail
} // namespace gl::api
