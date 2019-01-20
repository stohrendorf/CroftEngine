#pragma once

#include "core/magic.h"

#include "gsl-lite.hpp"

#include <boost/optional.hpp>
#include <yaml-cpp/yaml.h>

#include <bitset>
#include <vector>

namespace engine
{
namespace floordata
{
using FloorData = std::vector<uint16_t>;


//! @brief Native TR floor data functions
//! @ingroup native
enum class FloorDataChunkType : uint8_t
{
    PortalSector = 0x01,
    FloorSlant = 0x02,
    CeilingSlant = 0x03,
    CommandSequence = 0x04,
    Death = 0x05,
    Climb = 0x06,
    FloorTriangleNW = 0x07, //  [_\_]
    FloorTriangleNE = 0x08, //  [_/_]
    CeilingTriangleNW = 0x09, //  [_/_]
    CeilingTriangleNE = 0x0A, //  [_\_]
    FloorTriangleNWPortalSW = 0x0B, //  [P\_]
    FloorTriangleNWPortalNE = 0x0C, //  [_\P]
    FloorTriangleNEPortalSE = 0x0D, //  [_/P]
    FloorTriangleNEPortalNW = 0x0E, //  [P/_]
    CeilingTriangleNWPortalSW = 0x0F, //  [P\_]
    CeilingTriangleNWPortalNE = 0x10, //  [_\P]
    CeilingTriangleNEPortalNW = 0x11, //  [P/_]
    CeilingTriangleNEPortalSE = 0x12, //  [_/P]
    Monkey = 0x13,
    MinecartLeft = 0x14, // In TR3 only. Function changed in TR4+.
    MinecartRight = 0x15 // In TR3 only. Function changed in TR4+.
};

//! @brief Native trigger types.
//! @ingroup native
//! @see FloorDataChunkType::Always
//! @see CommandOpcode
enum class SequenceCondition
{
    LaraIsHere = 0x00, //!< If Lara is in sector, run (any case).
    LaraOnGround = 0x01, //!< If Lara is in sector, run (land case).
    ItemActivated = 0x02, //!< If item is activated, run, else stop.
    KeyUsed = 0x03, //!< If item is activated, run.
    ItemPickedUp = 0x04, //!< If item is picked up, run.
    ItemIsHere = 0x05, //!< If item is in sector, run, else stop.
    LaraOnGroundInverted = 0x06, //!< If Lara is in sector, stop (land case).
    LaraInCombatMode = 0x07, //!< If Lara is in combat state, run (any case).
    Dummy = 0x08, //!< If Lara is in sector, run (air case).
    AntiTrigger = 0x09, //!< TR2-5 only: If Lara is in sector, stop (any case).
    HeavySwitch = 0x0A, //!< TR3-5 only: If item is activated by item, run.
    HeavyAntiTrigger = 0x0B, //!< TR3-5 only: If item is activated by item, stop.
    Monkey = 0x0C, //!< TR3-5 only: If Lara is monkey-swinging, run.
    Skeleton = 0x0D, //!< TR5 only: Activated by skeleton only?
    TightRope = 0x0E, //!< TR5 only: If Lara is on tightrope, run.
    CrawlDuck = 0x0F, //!< TR5 only: If Lara is crawling, run.
    Climb = 0x10, //!< TR5 only: If Lara is climbing, run.
};

//! @brief Native trigger function types.
//! @ingroup native
//! @see FloorDataChunkType::Always
//! @see SequenceCondition
enum class CommandOpcode
{
    Activate = 0x00,
    SwitchCamera = 0x01,
    UnderwaterCurrent = 0x02,
    FlipMap = 0x03,
    FlipOn = 0x04,
    FlipOff = 0x05,
    LookAt = 0x06,
    EndLevel = 0x07,
    PlayTrack = 0x08,
    FlipEffect = 0x09,
    Secret = 0x0A,
    ClearBodies = 0x0B, // Unused in TR4
    FlyBy = 0x0C,
    CutScene = 0x0D
};


struct FloorDataChunk
{
    explicit FloorDataChunk(const FloorData::value_type fd)
            : isLast{extractIsLast( fd )}
            , sequenceCondition{extractSequenceCondition( fd )}
            , type{extractType( fd )}
    {
    }

    bool isLast;
    SequenceCondition sequenceCondition;
    FloorDataChunkType type;

    static FloorDataChunkType extractType(const FloorData::value_type data)
    {
        return gsl::narrow_cast<FloorDataChunkType>( data & 0xff );
    }

private:
    static SequenceCondition extractSequenceCondition(const FloorData::value_type data)
    {
        return gsl::narrow_cast<SequenceCondition>( (data & 0x3f00) >> 8 );
    }

    static constexpr bool extractIsLast(const FloorData::value_type data)
    {
        return (data & 0x8000) != 0;
    }
};


class ActivationState
{
public:
    static constexpr const uint16_t Oneshot = 0x100;
    static constexpr const uint16_t ActivationMask = 0x3e00;
    static constexpr const uint16_t InvertedActivation = 0x4000;
    static constexpr const uint16_t Locked = 0x8000;

    using ActivationSet = std::bitset<5>;

    explicit ActivationState() = default;

    explicit ActivationState(const FloorData::value_type fd)
            : m_oneshot{(fd & Oneshot) != 0}
            , m_inverted{(fd & InvertedActivation) != 0}
            , m_locked{(fd & Locked) != 0}
            , m_activationSet{extractActivationSet( fd )}
    {
    }

    bool isOneshot() const noexcept
    {
        return m_oneshot;
    }

    void setOneshot(const bool oneshot) noexcept
    {
        m_oneshot = oneshot;
    }

    bool isInverted() const noexcept
    {
        return m_inverted;
    }

    bool isLocked() const noexcept
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

    const ActivationSet& getActivationSet() const noexcept
    {
        return m_activationSet;
    }

    bool isFullyActivated() const
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

    bool isInActivationSet(const size_t i) const
    {
        return m_activationSet.test( i );
    }

    static int16_t extractTimeout(const FloorData::value_type fd)
    {
        const auto seconds = gsl::narrow_cast<uint8_t>( fd & 0xff );
        if( seconds > 1 )
            return gsl::narrow<int16_t>( seconds * core::FrameRate );
        else
            return seconds;
    }

    YAML::Node save() const
    {
        YAML::Node node;
        node.SetStyle( YAML::EmitterStyle::Flow );
        node["oneshot"] = m_oneshot;
        node["inverted"] = m_inverted;
        node["locked"] = m_locked;
        for( size_t i = 0; i < m_activationSet.size(); ++i )
            node["activationSet"].push_back( m_activationSet[i] );
        return node;
    }

    void load(const YAML::Node& node)
    {
        if( !node["oneshot"].IsScalar() )
            BOOST_THROW_EXCEPTION( std::domain_error( "ActivationState::oneshot is not scalar" ) );
        if( !node["inverted"].IsScalar() )
            BOOST_THROW_EXCEPTION( std::domain_error( "ActivationState::inverted is not scalar" ) );
        if( !node["locked"].IsScalar() )
            BOOST_THROW_EXCEPTION( std::domain_error( "ActivationState::locked is not scalar" ) );
        if( !node["activationSet"].IsSequence() )
            BOOST_THROW_EXCEPTION( std::domain_error( "ActivationState::activationSet is not a sequence" ) );

        m_oneshot = node["oneshot"].as<bool>();
        m_inverted = node["inverted"].as<bool>();
        m_locked = node["locked"].as<bool>();
        for( size_t i = 0; i < m_activationSet.size(); ++i )
            m_activationSet[i] = node["activationSet"][i].as<bool>();
    }

private:
    static ActivationSet extractActivationSet(const FloorData::value_type fd)
    {
        const auto bits = gsl::narrow_cast<uint16_t>( (fd & ActivationMask) >> 9 );
        return ActivationSet{bits};
    }

    bool m_oneshot = false;
    bool m_inverted = false;
    bool m_locked = false;
    ActivationSet m_activationSet{};
};


struct CameraParameters
{
    explicit CameraParameters(const FloorData::value_type fd)
            : timeout{gsl::narrow_cast<uint8_t>( fd & 0xff )}
            , oneshot{(fd & 0x100) != 0}
            , isLast{(fd & 0x8000) != 0}
            , smoothness{gsl::narrow_cast<uint8_t>( (fd >> 8) & 0x3e )}
    {
    }

    const uint8_t timeout;
    const bool oneshot;
    const bool isLast;
    const uint8_t smoothness;
};


struct Command
{
    explicit Command(const FloorData::value_type fd)
            : isLast{extractIsLast( fd )}
            , opcode{extractOpcode( fd )}
            , parameter{extractParameter( fd )}
    {
    }

    mutable bool isLast;
    CommandOpcode opcode;
    uint16_t parameter;

private:
    static CommandOpcode extractOpcode(const FloorData::value_type data)
    {
        return gsl::narrow_cast<CommandOpcode>( (data >> 10) & 0x0f );
    }

    static constexpr uint16_t extractParameter(const FloorData::value_type data)
    {
        return static_cast<uint16_t>(data & 0x03ffu);
    }

    static constexpr bool extractIsLast(const FloorData::value_type data)
    {
        return (data & 0x8000u) != 0;
    }
};


inline boost::optional<uint8_t> getPortalTarget(const FloorData::value_type* fdData)
{
    if( fdData == nullptr )
        return {};

    FloorDataChunk chunk{fdData[0]};
    if( chunk.type == FloorDataChunkType::FloorSlant )
    {
        if( chunk.isLast )
            return {};
        fdData += 2;
        chunk = FloorDataChunk{fdData[0]};
    }
    if( chunk.type == FloorDataChunkType::CeilingSlant )
    {
        if( chunk.isLast )
            return {};
        fdData += 2;
        chunk = FloorDataChunk{fdData[0]};
    }
    if( chunk.type == FloorDataChunkType::PortalSector )
    {
        return gsl::narrow_cast<uint8_t>( fdData[1] );
    }

    return {};
}
}
}
