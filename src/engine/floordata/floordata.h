#pragma once

#include "types.h"
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
struct FloorDataChunk
{
    explicit FloorDataChunk(const FloorDataValue fd)
            : isLast{extractIsLast( fd )}
            , sequenceCondition{extractSequenceCondition( fd )}
            , type{extractType( fd )}
    {
    }

    bool isLast;
    SequenceCondition sequenceCondition;
    FloorDataChunkType type;

    static FloorDataChunkType extractType(const FloorDataValue data)
    {
        return gsl::narrow_cast<FloorDataChunkType>( data.get() & 0xff );
    }

private:
    static SequenceCondition extractSequenceCondition(const FloorDataValue data)
    {
        return gsl::narrow_cast<SequenceCondition>( (data.get() & 0x3f00) >> 8 );
    }

    static constexpr bool extractIsLast(const FloorDataValue data)
    {
        return (data.get() & 0x8000) != 0;
    }
};


class ActivationState
{
public:
    static constexpr const uint16_t TimeoutMask = 0x00ff;
    static constexpr const uint16_t Oneshot = 0x0100;
    static constexpr const uint16_t ActivationMask = 0x3e00;
    static constexpr const uint16_t InvertedActivation = 0x4000;
    static constexpr const uint16_t Locked = 0x8000;

    using ActivationSet = std::bitset<5>;

    explicit ActivationState() = default;

    explicit ActivationState(const FloorDataValue fd)
            : m_oneshot{(fd.get() & Oneshot) != 0}
            , m_inverted{(fd.get() & InvertedActivation) != 0}
            , m_locked{(fd.get() & Locked) != 0}
            , m_activationSet{extractActivationSet( fd )}
            , m_timeout{(fd.get() & TimeoutMask) * core::FrameRate}
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

    core::Frame getTimeout() const noexcept
    {
        return m_timeout;
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
    static ActivationSet extractActivationSet(const FloorDataValue fd)
    {
        const auto bits = gsl::narrow_cast<uint16_t>( (fd.get() & ActivationMask) >> 9 );
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
            : timeout{gsl::narrow_cast<uint8_t>( fd.get() & 0xff )}
            , oneshot{(fd.get() & 0x100) != 0}
            , isLast{(fd.get() & 0x8000) != 0}
            , smoothness{gsl::narrow_cast<uint8_t>( (fd.get() >> 8) & 0x3e )}
    {
    }

    const uint8_t timeout;
    const bool oneshot;
    const bool isLast;
    const uint8_t smoothness;
};


struct Command
{
    explicit Command(const FloorDataValue fd)
            : isLast{extractIsLast( fd )}
            , opcode{extractOpcode( fd )}
            , parameter{extractParameter( fd )}
    {
    }

    mutable bool isLast;
    CommandOpcode opcode;
    uint16_t parameter;

private:
    static CommandOpcode extractOpcode(const FloorDataValue data)
    {
        return gsl::narrow_cast<CommandOpcode>( (data.get() >> 10) & 0x0f );
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


inline boost::optional<uint8_t> getPortalTarget(const FloorDataValue* fdData)
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
        return gsl::narrow_cast<uint8_t>( fdData[1].get() );
    }

    return {};
}
}
}
