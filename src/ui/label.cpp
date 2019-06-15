#include "label.h"

#include "util/cimgwrapper.h"

namespace ui
{

namespace
{
const std::array<const int, 110> charWidths{
    14, 11, 11, 11, 11, 11, 11, 13, 8, 11, 12, 11, 13, 13, 12, 11,
    12, 12, 11, 12, 13, 13, 13, 12, 12, 11, 9, 9, 9, 9, 9, 9,
    9, 9, 5, 9, 9, 5, 12, 10, 9, 9, 9, 8, 9, 8, 9, 9,
    11, 9, 9, 9, 12, 8, 10, 10, 10, 10, 10, 9, 10, 10, 5, 5,
    5, 11, 9, 10, 8, 6, 6, 7, 7, 3, 11, 8, 13, 16, 9, 4,
    12, 12, 7, 5, 7, 7, 7, 7, 7, 7, 7, 7, 16, 14, 14, 14,
    16, 16, 16, 16, 16, 12, 14, 8, 8, 8, 8, 8, 8, 8
};

const std::array<const uint8_t, 98> charToSprite{
    0, 64, 66, 78, 77, 74, 78, 79, 69, 70, 92, 72, 63, 71, 62, 68,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 73, 73, 66, 74, 75, 65,
    0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 80, 76, 81, 97, 98,
    77, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 100, 101, 102, 67, 0,
    0, 0
};

void drawLine(render::gl::Image<render::gl::SRGBA8>& img,
              const int x0, const int y0, const int width, const int height,
              const loader::file::ByteColor& color)
{
    img.line( x0, y0, x0 + width, y0 + height, color.toTextureColor() );
}

void drawOutline(render::gl::Image<render::gl::SRGBA8>& img,
                 const int x, const int y, const int width, const int height,
                 const loader::file::Palette& palette)
{
    // top
    drawLine( img, x, y - 1, width + 1, 0, palette.colors[15] );
    drawLine( img, x, y, width, 0, palette.colors[31] );
    //right
    drawLine( img, x + width, y - 1, 0, height + 1, palette.colors[15] );
    drawLine( img, x + width + 1, y - 1, 0, height + 2, palette.colors[31] );
    // bottom
    drawLine( img, x + width, y + height, -width, 0, palette.colors[15] );
    drawLine( img, x + width, y + height + 1, -width - 1, 0, palette.colors[31] );
    // left
    drawLine( img, x - 1, y + height, 0, -height - 1, palette.colors[15] );
    drawLine( img, x, y + height, 0, -height, palette.colors[31] );
}
}

int Label::calcWidth() const
{
    int width = 0;

    for( uint8_t chr : text )
    {
        const auto origChar = chr;
        if( chr > 129 || (chr > 10 && chr < 32) )
            continue;

        if( chr == ' ' )
        {
            width += (wordSpacing * scaleX) / 0x10000;
            continue;
        }

        if( chr < 11 )
            chr += 81;
        else if( chr < 16 )
            chr += 91;
        else
            chr = charToSprite[chr - ' '];

        if( origChar == '(' || origChar == ')' || origChar == '$' || origChar == '~' )
            continue;

        width += (charWidths[chr] + letterSpacing) * scaleX / 0x10000;
    }

    width -= letterSpacing;
    return width;
}

void Label::draw(CachedFont& font,
                 render::gl::Image<render::gl::SRGBA8>& img,
                 const loader::file::Palette& palette) const
{
    Expects( font.getScaleX() == scaleX );
    Expects( font.getScaleY() == scaleY );

    if( blink )
    {
        --timeout;
        if( timeout <= -blinkTime )
        {
            timeout = blinkTime;
        }
        else if( timeout < 0 )
        {
            return;
        }
    }

    auto y = posY;
    auto x = posX;
    const auto textWidth = calcWidth();

    if( alignX == Alignment::Center )
    {
        x += (img.getWidth() - textWidth) / 2;
    }
    else if( alignX == Alignment::Right )
    {
        x += img.getWidth() - textWidth;
    }

    if( alignY == Alignment::Center )
    {
        y += img.getHeight() / 2;
    }
    else if( alignY == Alignment::Bottom )
    {
        y += img.getHeight();
    }

    auto bgndX = bgndOffX + x - 2;
    const auto bgndY = bgndOffY + y + 1;

    int bgndWidth = 0;
    int bgndHeight = 0;
    if( fillBackground || outline )
    {
        if( bgndSizeX != 0 )
        {
            bgndWidth = bgndSizeX + 4;
            bgndX += (textWidth - bgndSizeX) / 2;
        }
        else
        {
            bgndWidth = textWidth + 4;
        }

        if( bgndSizeY != 0 )
        {
            bgndHeight = bgndSizeY;
        }
        else
        {
            bgndHeight = 16;
        }
    }

    if( fillBackground )
    {
        for( int dy = 0; dy < bgndHeight; ++dy )
            img.line( bgndX, bgndY + dy, bgndX + bgndWidth - 1, bgndY + dy, { 0, 0, 0, 192 }, true );
    }

    for( uint8_t chr : text )
    {
        const auto origChar = chr;
        if( chr > 129 || (chr > 10 && chr < 32) )
            continue;

        if( chr == ' ' )
        {
            x += (wordSpacing * scaleX) / 0x10000;
            continue;
        }

        if( chr <= 10 )
            chr += 81;
        else if( chr <= 15 )
            chr += 91;
        else
            chr = charToSprite[chr - ' '];

        font.draw( chr, x, y, img );

        if( origChar == '(' || origChar == ')' || origChar == '$' || origChar == '~' )
            continue;

        x += (charWidths[chr] + letterSpacing) * scaleX / 0x10000;
    }

    if( outline )
    {
        drawOutline( img, bgndX, bgndY, bgndWidth, bgndHeight, palette );
    }
}
}
