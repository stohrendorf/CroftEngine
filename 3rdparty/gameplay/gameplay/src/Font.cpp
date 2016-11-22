#include "Base.h"
#include "Font.h"
#include "Game.h"
#include "MaterialParameter.h"
#include "Image.h"

#include <glm/gtc/matrix_transform.hpp>

#include <boost/log/trivial.hpp>

#include FT_OUTLINE_H

namespace gameplay
{
    namespace
    {
        FT_Library freeTypeLib = nullptr;


        const char* getFreeTypeErrorMessage(FT_Error err)
        {
#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  case e: return s;
#define FT_ERROR_START_LIST     switch (err) {
#define FT_ERROR_END_LIST       }
#include FT_ERRORS_H
            return "(Unknown error)";
        }


        FT_Library loadFreeTypeLib()
        {
            if( freeTypeLib != nullptr )
                return freeTypeLib;

            auto error = FT_Init_FreeType(&freeTypeLib);
            if( error != FT_Err_Ok )
            {
                BOOST_LOG_TRIVIAL(fatal) << "Failed to load freetype library: " << getFreeTypeErrorMessage(error);
                BOOST_THROW_EXCEPTION(std::runtime_error("Failed to load freetype library"));
            }

            BOOST_ASSERT(freeTypeLib != nullptr);

            atexit([]()
                {
                    FT_Done_FreeType(freeTypeLib);
                    freeTypeLib = nullptr;
                });

            return freeTypeLib;
        }
    }


    Font::Font(const std::string& ttf, int size)
    {
        auto error = FT_New_Face(loadFreeTypeLib(), ttf.c_str(), 0, &m_face);
        if( error != FT_Err_Ok )
        {
            BOOST_LOG_TRIVIAL(fatal) << "Failed to load font face '" << ttf << "': " << getFreeTypeErrorMessage(error);
            BOOST_THROW_EXCEPTION(std::runtime_error("Failed to load font face"));
        }

        BOOST_ASSERT(m_face != nullptr);

        error = FT_Set_Char_Size(m_face, 0, size * 64, 0, 0);
        if(error != FT_Err_Ok)
        {
            BOOST_LOG_TRIVIAL(fatal) << "Failed to set char size for font '" << ttf << "': " << getFreeTypeErrorMessage(error);
            BOOST_THROW_EXCEPTION(std::runtime_error("Failed to set char size"));
        }

        error = FT_Set_Pixel_Sizes(m_face, 0, size);
        if(error != FT_Err_Ok)
        {
            BOOST_LOG_TRIVIAL(fatal) << "Failed to set pixel size for font '" << ttf << "': " << getFreeTypeErrorMessage(error);
            BOOST_THROW_EXCEPTION(std::runtime_error("Failed to set pixel size"));
        }
    }


    Font::~Font()
    {
        FT_Done_Face(m_face);
        m_face = nullptr;
    }

    void Font::renderCallback(int y, int count, const FT_Span_* spans, void* user)
    {
        Font* font = reinterpret_cast<Font*>(user);
        y = -y;

        auto color = font->m_currentColor;
        for(int i = 0; i < count; i++)
        {
            const FT_Span span = spans[i];
            color.a = span.coverage / 255.0f;

            font->m_targetImage->line(font->m_x0 + span.x, font->m_y0 + y, font->m_x0 + span.x + span.len - 1, font->m_y0 + y, color);
        }
    }

    void Font::drawText(const char* text, int x, int y, const glm::vec4& color)
    {
        BOOST_ASSERT(text);

        m_currentColor = color;

        m_x0 = x;
        m_y0 = y;

        while(const char chr = *text++)
        {
            /* load glyph image into the slot (erase previous one) */
            auto error = FT_Load_Char(m_face, chr, FT_LOAD_RENDER | FT_LOAD_TARGET_NORMAL);
            if(error)
            {
                BOOST_LOG_TRIVIAL(warning) << "Failed to load character '" << chr << "'";
                continue;
            }

            /* convert to an anti-aliased bitmap */
            error = FT_Render_Glyph(m_face->glyph, FT_RENDER_MODE_NORMAL);
            if(error)
            {
                BOOST_LOG_TRIVIAL(warning) << "Failed to render character '" << chr << "'";
                continue;
            }

            FT_Raster_Params params;

            params.target = nullptr;
            params.flags = FT_RASTER_FLAG_DIRECT | FT_RASTER_FLAG_AA;
            params.user = this;
            params.gray_spans = &Font::renderCallback;
            params.black_spans = nullptr;
            params.bit_set = nullptr;
            params.bit_test = nullptr;

            FT_Outline_Render(freeTypeLib,
                              &m_face->glyph->outline,
                              &params);

            m_x0 += m_face->glyph->advance.x / 64;
            m_y0 += m_face->glyph->advance.y / 64;
        }
    }


    void Font::drawText(const std::string& text, int x, int y, float red, float green, float blue, float alpha)
    {
        drawText(text.c_str(), x, y, glm::vec4(red, green, blue, alpha));
    }
}
