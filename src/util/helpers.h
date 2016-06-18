#pragma once

#include <boost/log/trivial.hpp>
#include <boost/property_tree/ptree.hpp>

#include <boost/version.hpp>
#if BOOST_VERSION >= 105800
#include <boost/type_index.hpp>
#endif

#include <irrlicht.h>

#include <cstdint>
#include <chrono>

#include "core/angle.h"

#define ENUM_TO_OSTREAM(name) \
    inline std::ostream& operator<<(std::ostream& str, name e) \
    { \
        return str << static_cast<int>(e); \
    } \

#define DISABLE_COPY(classname) \
    classname(const classname&) = delete; \
    classname& operator=(const classname&) = delete

namespace util
{
inline irr::f32 wrapAngle(const irr::f32 value)
{
    int i = static_cast<int>(value / 360.0);
    i = value < 0.0 ? i - 1 : i;
    return value - 360.0f * i;
}

template<typename T>
inline bool fuzzyZero(T value) noexcept
{
    return std::abs(value) <= std::numeric_limits<T>::epsilon();
}

template<typename T>
inline bool fuzzyEqual(T a, T b) noexcept
{
    return fuzzyZero(a - b);
}

template<typename T>
inline bool fuzzyOne(T value) noexcept
{
    return fuzzyEqual(value, static_cast<T>(1));
}

void writeTGAfile(const char *filename, const uint8_t *data, const int width, const int height, char invY);

using ClockType = std::chrono::high_resolution_clock;

using FloatDuration = float;
using Duration = std::chrono::duration<FloatDuration, std::chrono::nanoseconds::period>;
using Seconds = std::chrono::duration<FloatDuration, std::chrono::seconds::period>;
using MilliSeconds = std::chrono::duration<FloatDuration, std::chrono::milliseconds::period>;

using TimePoint = ClockType::time_point;

constexpr inline FloatDuration toSeconds(Duration d) noexcept
{
    return d.count() * static_cast<FloatDuration>(Duration::period::num) / static_cast<FloatDuration>(Duration::period::den);
}

constexpr inline util::Duration fromSeconds(FloatDuration d) noexcept
{
    return Duration(d * static_cast<FloatDuration>(Duration::period::den) / static_cast<FloatDuration>(Duration::period::num));
}

inline TimePoint now() noexcept
{
    return ClockType::now();
}

template<typename T>
inline T getSetting(boost::property_tree::ptree& ptree, const char* path, const T& defaultValue)
{
    T result = ptree.get<T>(path, defaultValue);
    ptree.put(path, defaultValue);
    return result;
}

inline boost::property_tree::ptree& getSettingChild(boost::property_tree::ptree& ptree, const char* path)
{
    if(!ptree.get_child_optional(path))
    {
        ptree.add_child(path, boost::property_tree::ptree());
    }

    return ptree.get_child(path);
}

class LifetimeTracker final
{
    DISABLE_COPY(LifetimeTracker);
private:
    const std::string m_objectName;
    const uintptr_t m_id;

public:
    explicit LifetimeTracker(const std::string& name, const void* ptr)
        : m_objectName(name)
        , m_id(reinterpret_cast<const uintptr_t>(ptr))
    {
        BOOST_LOG_TRIVIAL(debug) << "Initializing: " << m_objectName << " @ " << std::hex << m_id << std::dec;
    }
    explicit LifetimeTracker(std::string&& name, const void* ptr)
        : m_objectName(std::move(name))
        , m_id(reinterpret_cast<const uintptr_t>(ptr))
    {
        BOOST_LOG_TRIVIAL(debug) << "Initializing: " << m_objectName << " @ " << std::hex << m_id << std::dec;
    }
    ~LifetimeTracker()
    {
        BOOST_LOG_TRIVIAL(debug) << "Shut down complete: " << m_objectName << " @ " << std::hex << m_id << std::dec;
    }
};

#define TRACK_LIFETIME() \
    ::util::LifetimeTracker _S_lifetimeTracker{ boost::typeindex::type_id<decltype(*this)>().pretty_name(), this }


enum class Axis
{
    PosZ,
    PosX,
    NegZ,
    NegX
};

inline boost::optional<Axis> axisFromAngle(core::Angle angle, core::Angle margin)
{
    BOOST_ASSERT(margin.toAU() <= 0x2000); // 45 degrees
    if(angle.toAU() <= 0x0000 + margin.toAU() && angle.toAU() >= 0x0000 - margin.toAU())
        return Axis::PosZ;
    if(angle.toAU() <= 0x4000 + margin.toAU() && angle.toAU() >= 0x4000 - margin.toAU())
        return Axis::PosX;
    if(angle.toAU() <= -0x4000 + margin.toAU() && angle.toAU() >= -0x4000 - margin.toAU())
        return Axis::NegX;
    if(angle.toAU() >= 0x8000 - margin.toAU() || angle.toAU() <= -0x8000 + margin.toAU())
        return Axis::NegZ;

    return{};
}

inline boost::optional<core::Angle> alignRotation(core::Angle angle, core::Angle margin)
{
    auto axis = axisFromAngle(angle, margin);
    if(!axis)
        return{};

    switch(*axis)
    {
        case Axis::PosZ: return boost::optional<core::Angle>(0_deg);
        case Axis::PosX: return boost::optional<core::Angle>(90_deg);
        case Axis::NegZ: return boost::optional<core::Angle>(-180_deg);
        case Axis::NegX: return boost::optional<core::Angle>(-90_deg);
    }
    
    // silence compiler warning
    return{};
}

} // namespace util
