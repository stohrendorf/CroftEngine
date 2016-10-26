#pragma once

#include "SpriteBatch.h"
#include <glm/detail/type_mat.hpp>


namespace gameplay
{
    /**
     * Defines a font for text rendering.
     */
    class Font
    {
        friend class Text;

    public:

        /**
         * Defines the set of allowable alignments when drawing text.
         */
        enum Justify
        {
            ALIGN_LEFT = 0x01,
            ALIGN_HCENTER = 0x02,
            ALIGN_RIGHT = 0x04,
            ALIGN_TOP = 0x10,
            ALIGN_VCENTER = 0x20,
            ALIGN_BOTTOM = 0x40,
            ALIGN_TOP_LEFT = ALIGN_TOP | ALIGN_LEFT,
            ALIGN_VCENTER_LEFT = ALIGN_VCENTER | ALIGN_LEFT,
            ALIGN_BOTTOM_LEFT = ALIGN_BOTTOM | ALIGN_LEFT,
            ALIGN_TOP_HCENTER = ALIGN_TOP | ALIGN_HCENTER,
            ALIGN_VCENTER_HCENTER = ALIGN_VCENTER | ALIGN_HCENTER,
            ALIGN_BOTTOM_HCENTER = ALIGN_BOTTOM | ALIGN_HCENTER,
            ALIGN_TOP_RIGHT = ALIGN_TOP | ALIGN_RIGHT,
            ALIGN_VCENTER_RIGHT = ALIGN_VCENTER | ALIGN_RIGHT,
            ALIGN_BOTTOM_RIGHT = ALIGN_BOTTOM | ALIGN_RIGHT
        };

        /**
         * Determines if this font supports the specified character code.
         *
         * @param character The character code to check.
         * @return True if this Font supports (can draw) the specified character, false otherwise.
         */
        static bool isCharacterSupported(int character);

        /**
         * Draws the specified text in a solid color, with a scaling factor.
         *
         * @param text The text to draw.
         * @param x The viewport x position to draw text at.
         * @param y The viewport y position to draw text at.
         * @param color The color of text.
         * @param size The size to draw text (0 for default size).
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
         * @param size The size to draw text (0 for default size).
         */
        void drawText(const std::string& text, int x, int y, float red, float green, float blue, float alpha);

        /**
         * Finishes text batching for this font and renders all drawn text.
         */
        void finish();

        /**
         * Gets the sprite batch used to draw this Font.
         *
         * @param size The font size to be drawn.
         *
         * @return The SpriteBatch that most closely matches the requested font size.
         */
        const std::shared_ptr<SpriteBatch>& getSpriteBatch() const;

        Font(const std::shared_ptr<Texture>& texture, size_t cellSizeX, size_t cellSizeY);
        ~Font();

    private:

        Font(const Font& copy) = delete;

        Font& operator=(const Font&) = delete;

        void lazyStart();

        std::shared_ptr<Texture> _texture;
        std::shared_ptr<SpriteBatch> _batch;
        Rectangle _viewport;
        size_t _cellSizeX = 0;
        size_t _cellSizeY = 0;
    };
}
