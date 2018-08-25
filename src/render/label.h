#include "gameplay.h"
#include "loader/color.h"
#include "loader/datatypes.h"

#include <cstdint>
#include <string>

namespace level
{
class Level;
}

namespace render
{
struct Label
{
    enum class Alignment
    {
        None, Center, Right, Bottom = Right
    };

    bool blink = false;
    Alignment alignX = Alignment::None;
    Alignment alignY = Alignment::None;
    bool fillBackground = false;
    bool outline = false;
    int16_t posX = 0;
    int16_t posY = 0;
    int16_t letterSpacing = 1;
    int16_t wordSpacing = 6;
    int16_t blinkTime = 0;
    mutable int16_t timeout = 0;
    int16_t bgndSizeX = 0;
    int16_t bgndSizeY = 0;
    int16_t bgndOffX = 0;
    int16_t bgndOffY = 0;
    int scaleX = 0x10000;
    int scaleY = 0x10000;
    std::string text;

    explicit Label(int16_t xpos, int16_t ypos, const std::string& string)
            : posX{xpos}
            , posY{ypos}
            , text{string, 0, std::min( std::string::size_type( 64 ), string.size() )}
    {
    }

    void draw(gameplay::gl::Image<gameplay::gl::RGBA8>& img, const level::Level& level) const;

    int calcWidth() const;

    void addBackground(int16_t xsize, int16_t ysize, int16_t xoff, int16_t yoff)
    {
        bgndSizeX = xsize;
        bgndSizeY = ysize;
        bgndOffX = xoff;
        bgndOffY = yoff;
        fillBackground = true;
    }

    void removeBackground()
    {
        fillBackground = false;
    }

    void flashText(bool blink, int16_t blinkTime)
    {
        this->blink = blink;
        if( blink )
        {
            blinkTime = blinkTime;
            timeout = blinkTime;
        }
    }
};
}
