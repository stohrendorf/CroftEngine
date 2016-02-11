#pragma once

#include "gui/common.h"
#include "util/helpers.h"

#include <irrlicht.h>

namespace engine
{
class Engine;
}

namespace gui
{
// These are the bars that are always exist in GUI.
// Scripted bars could be created and drawn separately later.

enum class BarType
{
    Health,     // TR 1-5
    Air,        // TR 1-5, alternate state - gas (TR5)
    Stamina,    // TR 3-5
    Warmth,     // TR 3 only
    Loading,
    Sentinel
};

// Bar color types.
// Each bar part basically has two colours - main and fade.

enum class BarColorType
{
    BaseMain,
    BaseFade,
    AltMain,
    AltFade,
    BackMain,
    BackFade,
    BorderMain,
    BorderFade
};

class ProgressBar
{
public:
    explicit ProgressBar(engine::Engine* engine);  // Bar constructor.

    void show(irr::f32 value);    // Main show bar procedure.
    void resize();

    void setColor(BarColorType colType, uint8_t R, uint8_t G, uint8_t B, uint8_t alpha);
    void setSize(irr::f32 width, irr::f32 height, irr::f32 borderSize);
    void setPosition(HorizontalAnchor anchor_X, irr::f32 offset_X, VerticalAnchor anchor_Y, irr::f32 offset_Y);
    void setValues(irr::f32 maxValue, irr::f32 warnValue);
    void setBlink(util::Duration interval);
    void setExtrude(bool enabled, uint8_t depth);
    void setAutoshow(bool enabled, util::Duration delay, bool fade, util::Duration fadeDelay);

    bool          m_forced = false;               // Forced flag is set when bar is strictly drawn.
    bool          m_visible = false;              // Is it visible or not.
    bool          m_alternate = false;            // Alternate state, in which bar changes color to AltColor.

    bool          m_invert = false;               // Invert decrease direction flag.
    bool          m_vertical = false;             // Change bar style to vertical.

private:
    void          recalculateSize();    // Recalculate size and position.
    void          recalculatePosition();

    engine::Engine* m_engine;

    float         m_x;                   // Horizontal position.
    float         m_y;                   // Vertical position.
    float         m_width;               // Real width.
    float         m_height;              // Real height.
    float         m_borderWidth;         // Real border size (horizontal).
    float         m_borderHeight;        // Real border size (vertical).

    HorizontalAnchor m_xAnchor;          // Horizontal anchoring: left, right or center.
    VerticalAnchor   m_yAnchor;          // Vertical anchoring: top, bottom or center.
    float         m_absXoffset;          // Absolute (resolution-independent) X offset.
    float         m_absYoffset;          // Absolute Y offset.
    float         m_absWidth;            // Absolute width.
    float         m_absHeight;           // Absolute height.
    float         m_absBorderSize;       // Absolute border size (horizontal).

    irr::video::SColor m_baseMainColor;    // Color at the min. of bar.
    irr::u32 m_baseMainColorAlpha;
    irr::video::SColor m_baseFadeColor;    // Color at the max. of bar.
    irr::u32 m_baseFadeColorAlpha;
    irr::video::SColor m_altMainColor;     // Alternate main color.
    irr::u32 m_altMainColorAlpha;
    irr::video::SColor m_altFadeColor;     // Alternate fade color.
    irr::u32 m_altFadeColorAlpha;
    irr::video::SColor m_backMainColor;    // Background main color.
    irr::u32 m_backMainColorAlpha;
    irr::video::SColor m_backFadeColor;    // Background fade color.
    irr::u32 m_backFadeColorAlpha;
    irr::video::SColor m_borderMainColor;  // Border main color.
    irr::u32 m_borderMainColorAlpha;
    irr::video::SColor m_borderFadeColor;  // Border fade color.
    irr::u32 m_borderFadeColorAlpha;

    bool m_extrude;             // Extrude effect.
    irr::video::SColor m_extrudeDepth;     // Extrude effect depth.
    irr::u32 m_extrudeDepthAlpha;

    irr::f32 m_maxValue;            // Maximum possible value.
    irr::f32 m_warnValue;           // Warning value, at which bar begins to blink.
    irr::f32 m_lastValue;           // Last value back-up for autoshow on change event.

    bool m_blink;               // Warning state (blink) flag.
    util::Duration m_blinkInterval;       // Blink interval (speed).
    util::Duration m_blinkCnt;            // Blink counter.

    bool m_autoShow;            // Autoshow on change flag.
    util::Duration m_autoShowDelay;       // How long bar will stay on-screen in AutoShow mode.
    util::Duration m_autoShowCnt;         // Auto-show counter.
    bool m_autoShowFade;        // Fade flag.
    util::Duration m_autoShowFadeDelay;   // Fade length.
    util::Duration m_autoShowFadeLength;     // Fade progress counter.

    irr::f32 m_rangeUnit;           // Range unit used to set base bar size.
    irr::f32 m_baseSize;            // Base bar size.
    irr::f32 m_baseRatio;           // Max. / actual value ratio.
};
} // namespace gui
