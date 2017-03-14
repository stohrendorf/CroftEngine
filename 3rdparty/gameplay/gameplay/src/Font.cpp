#include "Base.h"
#include "Font.h"
#include "Game.h"
#include "MaterialParameter.h"
#include "Image.h"

#include <glm/gtc/matrix_transform.hpp>

#include <gsl/gsl>

#include <boost/log/trivial.hpp>

#include FT_OUTLINE_H


namespace gameplay
{
    namespace
    {
        FT_Library freeTypeLib = nullptr;
        int _dummyFaceId = 0;


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


    FT_Error ftcFaceRequester(FTC_FaceID face_id,
                              FT_Library library,
                              FT_Pointer req_data,
                              FT_Face* aface)
    {
        Expects(face_id == &_dummyFaceId);
        auto error = FT_New_Face(library, static_cast<const char*>(req_data), 0, aface);
        if( error != FT_Err_Ok )
        {
            BOOST_LOG_TRIVIAL(fatal) << "Failed to load font " << static_cast<const char*>(req_data) << ": " << getFreeTypeErrorMessage(error);
            BOOST_THROW_EXCEPTION(std::runtime_error("Failed to load font"));
        }
        return FT_Err_Ok;
    }


    Font::Font(const std::string& ttf, int size)
        : m_filename{ttf}
    {
        auto error = FTC_Manager_New(loadFreeTypeLib(), 0, 0, 0, &ftcFaceRequester, const_cast<char*>(m_filename.c_str()), &m_cache);
        if( error != FT_Err_Ok )
        {
            BOOST_LOG_TRIVIAL(fatal) << "Failed to create cache manager: " << getFreeTypeErrorMessage(error);
            BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create cache manager"));
        }
        BOOST_ASSERT(m_cache != nullptr);

        error = FTC_CMapCache_New(m_cache, &m_cmapCache);
        if( error != FT_Err_Ok )
        {
            BOOST_LOG_TRIVIAL(fatal) << "Failed to create cmap cache: " << getFreeTypeErrorMessage(error);
            BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create cmap cache"));
        }
        BOOST_ASSERT(m_cmapCache != nullptr);

        error = FTC_SBitCache_New(m_cache, &m_sbitCache);
        if( error != FT_Err_Ok )
        {
            BOOST_LOG_TRIVIAL(fatal) << "Failed to create cmap cache: " << getFreeTypeErrorMessage(error);
            BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create cmap cache"));
        }
        BOOST_ASSERT(m_sbitCache != nullptr);

        m_imgType.face_id = &_dummyFaceId;
        m_imgType.width = size;
        m_imgType.height = size;
        m_imgType.flags = FT_LOAD_DEFAULT | FT_LOAD_RENDER;
    }


    Font::~Font()
    {
        FTC_Manager_Done(m_cache);
        m_cache = nullptr;
    }


    void Font::drawText(const char* text, int x, int y, const gl::PixelRGBA_U8& color)
    {
        BOOST_ASSERT(text);

        auto currentColor = color;

        m_x0 = x;
        m_y0 = y;

        while( const char chr = *text++ )
        {
            auto glyphIndex = FTC_CMapCache_Lookup(m_cmapCache, &_dummyFaceId, -1, chr);
            if( glyphIndex <= 0 )
            {
                BOOST_LOG_TRIVIAL(warning) << "Failed to load character '" << chr << "'";
                continue;
            }

            FTC_SBit sbit = nullptr;
            FTC_Node node = nullptr;
            auto error = FTC_SBitCache_Lookup(m_sbitCache, &m_imgType, glyphIndex, &sbit, &node);
            if( error != FT_Err_Ok )
            {
                BOOST_LOG_TRIVIAL(warning) << "Failed to load from sbit cache: " << getFreeTypeErrorMessage(error);
                FTC_Node_Unref(node, m_cache);
                continue;
            }

            {
                for( int y = 0, i = 0; y < sbit->height; y++ )
                {
                    for( int x = 0; x < sbit->width; x++ , i++ )
                    {
                        currentColor.a = sbit->buffer[i];

                        m_targetImage->at(m_x0 + x + sbit->left, m_y0 + y - sbit->top) = currentColor;
                    }
                }
            }

            m_x0 += sbit->xadvance;
            m_y0 += sbit->yadvance;

            FTC_Node_Unref(node, m_cache);
        }
    }


    void Font::drawText(const std::string& text, int x, int y, float red, float green, float blue, float alpha)
    {
        drawText(text.c_str(), x, y, gl::PixelRGBA_U8(red * 255, green * 255, blue * 255, alpha * 255));
    }
}
