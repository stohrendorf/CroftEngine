#pragma once

#include "gl/pixel.h"
#include "ext/image.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_CACHE_H


namespace gameplay
{
    namespace ext
    {
        class Font
        {
        public:
            void drawText(const char* text, int x, int y, const gl::RGBA8& color);

            void drawText(const std::string& text, int x, int y, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);

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
}
