#pragma once

#include "core/magic.h"
#include "serialization/serialization_fwd.h"
#include "types.h"
#include "util/helpers.h"

#include <bitset>
#include <gsl/gsl-lite.hpp>
#include <optional>

namespace engine::floordata
{
struct FloorDataChunk
{
  explicit FloorDataChunk(const FloorDataValue fd)
      : isLast{extractIsLast(fd)}
      , sequenceCondition{extractSequenceCondition(fd)}
      , type{extractType(fd)}
  {
  }

  bool isLast;
  SequenceCondition sequenceCondition;
  FloorDataChunkType type;

  static FloorDataChunkType extractType(const FloorDataValue data)
  {
    return gsl::narrow_cast<FloorDataChunkType>(data.get() & 0xffu);
  }

private:
  static SequenceCondition extractSequenceCondition(const FloorDataValue data)
  {
    return gsl::narrow_cast<SequenceCondition>((data.get() & 0x3f00u) >> 8u);
  }

  static constexpr bool extractIsLast(const FloorDataValue data)
  {
    return (data.get() & 0x8000u) != 0;
  }
};

class ActivationState
{
public:
  static constexpr uint16_t TimeoutMask = 0x00ff;
  static constexpr uint16_t Oneshot = 0x0100;
  static constexpr uint16_t InvertedActivation = 0x4000;
  static constexpr uint16_t Locked = 0x8000;

  using ActivationSet = std::bitset<5>;

  explicit ActivationState() = default;

  explicit ActivationState(const FloorDataValue fd)
      : m_oneshot{(fd.get() & Oneshot) != 0}
      , m_inverted{(fd.get() & InvertedActivation) != 0}
      , m_locked{(fd.get() & Locked) != 0}
      , m_activationSet{extractActivationSet(fd)}
  {
    auto timeout = core::Seconds{static_cast<core::Seconds::type>(fd.get() & TimeoutMask)};
    if(timeout.get() == 1)
      m_timeout = 1_frame;
    else
      m_timeout = timeout * core::FrameRate;
  }

  [[nodiscard]] bool isOneshot() const noexcept
  {
    return m_oneshot;
  }

  void setOneshot(const bool oneshot) noexcept
  {
    m_oneshot = oneshot;
  }

  [[nodiscard]] bool isInverted() const noexcept
  {
    return m_inverted;
  }

  [[nodiscard]] bool isLocked() const noexcept
  {
    return m_locked;
  }

  void operator^=(const ActivationSet& rhs)
  {
    m_activationSet ^= rhs;
  }

  void operator|=(const ActivationSet& rhs)
  {
    m_activationSet |= rhs;
  }

  void operator&=(const ActivationSet& rhs)
  {
    m_activationSet &= rhs;
  }

  [[nodiscard]] const ActivationSet& getActivationSet() const noexcept
  {
    return m_activationSet;
  }

  [[nodiscard]] bool isFullyActivated() const
  {
    return m_activationSet.all();
  }

  void fullyActivate()
  {
    m_activationSet.set();
  }

  void fullyDeactivate()
  {
    m_activationSet.reset();
  }

  void setInverted(const bool inverted) noexcept
  {
    m_inverted = inverted;
  }

  void setLocked(const bool locked) noexcept
  {
    m_locked = locked;
  }

  [[nodiscard]] bool isInActivationSet(const size_t i) const
  {
    return m_activationSet.test(i);
  }

  [[nodiscard]] core::Frame getTimeout() const noexcept
  {
    return m_timeout;
  }

  void serialize(const serialization::Serializer<world::World>& ser);

  static ActivationState create(const serialization::Serializer<world::World>& ser)
  {
    ActivationState result{};
    result.serialize(ser);
    return result;
  }

private:
  static ActivationSet extractActivationSet(const FloorDataValue fd)
  {
    const auto bits = gsl::narrow_cast<uint16_t>(util::bits(fd.get(), 9, 5));
    return ActivationSet{bits};
  }

  bool m_oneshot = false;
  bool m_inverted = false;
  bool m_locked = false;
  ActivationSet m_activationSet{};
  core::Frame m_timeout = 0_frame;
};

struct CameraParameters
{
  explicit CameraParameters(const FloorDataValue fd)
      : timeout{core::Seconds{static_cast<core::Seconds::type>(int8_t(fd.get()))}}
      , oneshot{(fd.get() & 0x100u) != 0}
      , isLast{(fd.get() & 0x8000u) != 0}
      , smoothness{gsl::narrow_cast<uint8_t>(util::bits(fd.get(), 9, 5) * 2)}
  {
  }

  const core::Seconds timeout;
  const bool oneshot;
  const bool isLast;
  const uint8_t smoothness;
};

struct Command
{
  explicit Command(const FloorDataValue fd)
      : isLast{extractIsLast(fd)}
      , opcode{extractOpcode(fd)}
      , parameter{extractParameter(fd)}
  {
  }

  mutable bool isLast;
  CommandOpcode opcode;
  uint16_t parameter;

private:
  static CommandOpcode extractOpcode(const FloorDataValue data)
  {
    return gsl::narrow_cast<CommandOpcode>(util::bits(data.get(), 10, 4));
  }

  static constexpr uint16_t extractParameter(const FloorDataValue data)
  {
    return static_cast<uint16_t>(data.get() & 0x03ffu);
  }

  static constexpr bool extractIsLast(const FloorDataValue data)
  {
    return (data.get() & 0x8000u) != 0;
  }
};

inline std::optional<uint8_t> getBoundaryRoom(const FloorDataValue* fdData)
{
  if(fdData == nullptr)
    return {};

  FloorDataChunk chunk{fdData[0]};
  if(chunk.type == FloorDataChunkType::FloorSlant)
  {
    if(chunk.isLast)
      return {};
    fdData += 2;
    chunk = FloorDataChunk{fdData[0]};
  }
  if(chunk.type == FloorDataChunkType::CeilingSlant)
  {
    if(chunk.isLast)
      return {};
    fdData += 2;
    chunk = FloorDataChunk{fdData[0]};
  }
  if(chunk.type == FloorDataChunkType::BoundaryRoom)
  {
    return gsl::narrow_cast<uint8_t>(fdData[1].get());
  }

  return {};
}
} // namespace engine::floordata
