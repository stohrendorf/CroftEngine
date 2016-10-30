#pragma once

#include "SpriteBatch.h"

#include <ft2build.h>
#include FT_FREETYPE_H

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
        void drawText(const char* text, int x, int y, const glm::vec4& color);

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

        void setTarget(const std::shared_ptr<Image>& img)
        {
            m_targetImage = img;
        }

    private:

        Font(const Font& copy) = delete;
        Font& operator=(const Font&) = delete;

        FT_Face m_face = nullptr;
        std::shared_ptr<Image> m_targetImage = nullptr;
        glm::vec4 m_currentColor;
        int m_x0 = 0;
        int m_y0 = 0;

        static void renderCallback(int y, int count, const FT_Span_ *spans, void *user);
    };
}
