#pragma once

#include "fontmanager.h"
#include "gui/common.h"
#include "util/helpers.h"

#include <boost/format.hpp>

namespace gui
{
struct TextLine
{
    std::string text;

    FontType  fontType = gui::FontType::Primary;
    FontStyle fontStyle = gui::FontStyle::MenuTitle;

    irr::core::vector2df position{ 10, 10 };
    mutable irr::core::vector2df offset;

    HorizontalAnchor Xanchor = gui::HorizontalAnchor::Right;
    VerticalAnchor   Yanchor = gui::VerticalAnchor::Bottom;

    mutable irr::core::vector2df topLeft;
    mutable irr::core::vector2df bottomRight;

    bool show = true;

    void move(float scaleFactor);
};

class TextLineManager
{
    TRACK_LIFETIME();

    engine::Engine* m_engine;
    std::list<const TextLine*> m_baseLines;
    std::list<TextLine> m_tempLines;

public:
    explicit TextLineManager(engine::Engine* engine);

    void add(const TextLine *line)
    {
        m_baseLines.push_back(line);
    }

    void erase(const TextLine* line)
    {
        m_baseLines.erase(std::find(m_baseLines.begin(), m_baseLines.end(), line));
    }

    void renderLine(const TextLine& line);
    void renderStrings();

    /**
     * Draws text using a FontType::Secondary.
     */
    TextLine* drawText(irr::f32 x, irr::f32 y, const std::string& str);

    TextLine* drawText(irr::f32 x, irr::f32 y, const boost::format& str)
    {
        return drawText(x, y, str.str());
    }

    void resizeTextLines();
};
} // namespace gui
