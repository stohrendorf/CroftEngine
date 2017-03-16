#pragma once

#include "Image.h"
#include "gl/texture.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_CACHE_H


namespace gameplay
{
    /**
     * Defines a font for text rendering.
     */
    class Font
    {
    public:
        /**
         * Draws the specified text in a solid color, with a scaling factor.
         *
         * @param text The text to draw.
         * @param x The viewport x position to draw text at.
         * @param y The viewport y position to draw text at.
         * @param color The color of text.
         */
        void drawText(const char* text, int x, int y, const gl::RGBA8& color);

        /**
         * Draws the specified text in a solid color, with a scaling factor.
         *
         * @param text The text to draw.
         * @param x The viewport x position to draw text at.
         * @param y The viewport y position to draw text at.
         * @param red The red channel of the text color.
         * @param green The green channel of the text color.
         * @param blue The blue channel of the text color.
         * @param alpha The alpha channel of the text color.
         */
        void drawText(const std::string& text, int x, int y, float red, float green, float blue, float alpha);

        Font(const std::string& ttf, int size);

        ~Font();


        void setTarget(const std::shared_ptr<Image<gl::RGBA8>>& img)
        {
            m_targetImage = img;
        }


        const std::shared_ptr<Image<gl::RGBA8>>& getTarget() const
        {
            return m_targetImage;
        }


    private:

        Font(const Font& copy) = delete;

        Font& operator=(const Font&) = delete;

        FTC_Manager m_cache = nullptr;
        FTC_CMapCache m_cmapCache = nullptr;
        FTC_SBitCache m_sbitCache = nullptr;
        FTC_ImageTypeRec m_imgType;
        std::shared_ptr<Image<gl::RGBA8>> m_targetImage = nullptr;
        int m_x0 = 0;
        int m_y0 = 0;
        const std::string m_filename;
    };
}
