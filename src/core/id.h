#pragma once

#include "serialization/serialization.h"
#include "tpl_helper.h"

#include <cstdint>
#include <functional>

namespace engine
{
enum class TR1ItemId;
enum class TR1SoundEffect;
} // namespace engine

namespace core
{
/**
 * @brief A simple wrapper for specifying something without arithmetic support.
 * @tparam StorageType ID type
 * @tparam Tag Tag for avoiding mixing different IDs with the same storage type
 */
template<typename StorageType, typename Tag, typename... Enums>
struct Id
{
  static_assert(tpl::is_all_enum_v<Enums...>, "Compatible types must be enums");

  using type = StorageType;
  using tag = Tag;

  constexpr explicit Id(type value)
      : m_value{value}
  {
  }

  template<typename T>
  // cppcheck-suppress noExplicitConstructor
  constexpr Id(T value) // NOLINT(google-explicit-constructor)
      : m_value{static_cast<type>(value)}
  {
    static_assert(sizeof...(Enums) == 0 || tpl::is_one_of_v<T, Enums...>, "Incompatible type");
  }

  constexpr auto& operator=(type value)
  {
    m_value = value;
    return *this;
  }

  template<typename T>
  constexpr auto& operator=(T value)
  {
    static_assert(sizeof...(Enums) == 0 || tpl::is_one_of_v<T, Enums...>, "Incompatible type");
    m_value = static_cast<type>(value);
    return *this;
  }

  constexpr explicit operator type() const
  {
    return m_value;
  }

  [[nodiscard]] constexpr type get() const
  {
    return m_value;
  }

  template<typename T>
  [[nodiscard]] constexpr T get_as() const
  {
    static_assert(sizeof...(Enums) == 0 || tpl::is_one_of_v<T, Enums...>, "Incompatible target type");
    return static_cast<T>(m_value);
  }

  [[nodiscard]] constexpr bool operator<(const Id<type, tag, Enums...> r) const
  {
    return get() < r.get();
  }

  [[nodiscard]] constexpr bool operator==(const Id<type, tag, Enums...> r) const
  {
    return get() == r.get();
  }

  [[nodiscard]] constexpr bool operator!=(const Id<type, tag, Enums...> r) const
  {
    return get() != r.get();
  }

  template<typename T>
  [[nodiscard]] constexpr bool operator==(const T r) const
  {
    return get_as<T>() == r;
  }

  template<typename T>
  [[nodiscard]] constexpr bool operator!=(const T r) const
  {
    return get_as<T>() != r;
  }

  template<typename T>
  [[nodiscard]] constexpr bool operator<(const T r) const
  {
    return get_as<T>() < r;
  }

  template<typename T>
  [[nodiscard]] constexpr bool operator<=(const T r) const
  {
    return get_as<T>() <= r;
  }

  template<typename T>
  [[nodiscard]] constexpr bool operator>(const T r) const
  {
    return get_as<T>() > r;
  }

  template<typename T>
  [[nodiscard]] constexpr bool operator>=(const T r) const
  {
    return get_as<T>() >= r;
  }

  void serialize(const serialization::Serializer& ser)
  {
    ser.tag("id");
    serialization::access::callSerialize(m_value, ser);
  }

  [[nodiscard]] static Id<StorageType, Tag, Enums...> create(const serialization::Serializer& ser)
  {
    ser.tag("id");
    return Id<StorageType, Tag, Enums...>{serialization::create(serialization::TypeId<StorageType>{}, ser)};
  }

private:
  StorageType m_value;
};

#define DECLARE_ID_PAREN_WRAPPER(value) value
#define DECLARE_ID(name, type) \
  struct name##_generated_tag  \
  {                            \
  };                           \
  using name = ::core::Id<DECLARE_ID_PAREN_WRAPPER(type), name##_generated_tag>

#define DECLARE_ID_E(name, type, ...) \
  struct name##_generated_tag         \
  {                                   \
  };                                  \
  using name = ::core::Id<DECLARE_ID_PAREN_WRAPPER(type), name##_generated_tag, __VA_ARGS__>

DECLARE_ID(RoomId8, uint8_t);
DECLARE_ID(RoomId16, uint16_t);
DECLARE_ID(RoomIdI16, int16_t);
DECLARE_ID(RoomId32, uint32_t);
DECLARE_ID(RoomGroupId, uint8_t);
DECLARE_ID(AnimStateId, uint16_t);
DECLARE_ID(TextureTileId, uint16_t);
DECLARE_ID(TextureId, uint16_t);
DECLARE_ID(MeshId, uint32_t);
DECLARE_ID(StaticMeshId, uint32_t);
DECLARE_ID(SampleId, uint32_t);
DECLARE_ID(BoxId, int16_t);
DECLARE_ID(SpriteInstanceId, uint16_t);
DECLARE_ID(ItemId, uint16_t);

DECLARE_ID_E(TypeId, uint16_t, engine::TR1ItemId);
DECLARE_ID_E(SoundEffectId, uint16_t, engine::TR1SoundEffect);

[[nodiscard]] inline constexpr AnimStateId operator"" _as(unsigned long long value)
{
  return AnimStateId{static_cast<AnimStateId::type>(value)};
}
} // namespace core

using core::operator""_as;

template<typename StorageType, typename Tag, typename... Enums>
struct std::hash<core::Id<StorageType, Tag, Enums...>>
{
  [[nodiscard]] constexpr size_t operator()(const core::Id<StorageType, Tag, Enums...>& v) const
  {
    return hash<StorageType>{}(v.get());
  }
};
