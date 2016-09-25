#include "Base.h"
#include "Font.h"
#include "Game.h"
#include "MaterialParameter.h"
#include "Material.h"

#include <glm/gtc/matrix_transform.hpp>

#include <boost/log/trivial.hpp>

// Default font shaders
#define FONT_VSH "shaders/font.vert"
#define FONT_FSH "shaders/font.frag"


namespace gameplay
{
    static std::vector<Font*> __fontCache;

    static std::shared_ptr<ShaderProgram> __fontProgram = nullptr;


    Font::Font() :
                 _format(BITMAP)
                 , _style(PLAIN)
                 , _size(0)
                 , _spacing(0.0f)
                 , _glyphs()
                 , _texture(nullptr)
                 , _batch(nullptr)
                 , _cutoffParam(nullptr)
    {
    }


    Font::~Font()
    {
        // Remove this Font from the font cache.
        std::vector<Font*>::iterator itr = std::find(__fontCache.begin(), __fontCache.end(), this);
        if( itr != __fontCache.end() )
        {
            __fontCache.erase(itr);
        }

        SAFE_DELETE(_batch);
    }


    Font* Font::create(const char* family, Style style, unsigned int size, const std::vector<Font::Glyph>& glyphs, const std::shared_ptr<Texture>& texture, Font::Format format)
    {
        BOOST_ASSERT(family);
        BOOST_ASSERT(texture);

        // Create the effect for the font's sprite batch.
        if( __fontProgram == nullptr )
        {
            std::vector<std::string> defines;
            if( format == DISTANCE_FIELD )
                defines.emplace_back( "DISTANCE_FIELD" );
            __fontProgram = ShaderProgram::createFromFile(FONT_VSH, FONT_FSH, defines);
            if( __fontProgram == nullptr )
            {
                BOOST_LOG_TRIVIAL(warning) << "Failed to create effect for font.";
                return nullptr;
            }
        }

        // Create batch for the font.
        SpriteBatch* batch = SpriteBatch::create(texture, __fontProgram, 128);

        if( batch == nullptr )
        {
            BOOST_LOG_TRIVIAL(warning) << "Failed to create batch for font.";
            return nullptr;
        }

        // Add linear filtering for better font quality.
        auto sampler = batch->getSampler();
        sampler->setFilterMode(Texture::LINEAR_MIPMAP_LINEAR, Texture::LINEAR);
        sampler->setWrapMode(Texture::CLAMP, Texture::CLAMP);

        Font* font = new Font();
        font->_format = format;
        font->_family = family;
        font->_style = style;
        font->_size = size;
        font->_texture = texture;
        font->_batch = batch;

        font->_glyphs = glyphs;

        return font;
    }


    unsigned int Font::getSize(unsigned int index) const
    {
        BOOST_ASSERT(index <= _sizes.size());

        // index zero == this font
        return index == 0 ? _size : _sizes[index - 1]->_size;
    }


    unsigned int Font::getSizeCount() const
    {
        return _sizes.size() + 1; // +1 for "this" font
    }


    Font::Format Font::getFormat() const
    {
        return _format;
    }


    bool Font::isCharacterSupported(int character) const
    {
        // TODO: Update this once we support unicode fonts
        int glyphIndex = character - 32; // HACK for ASCII
        return (glyphIndex >= 0 && glyphIndex < (int)_glyphs.size());
    }


    void Font::lazyStart()
    {
        if( _batch->isStarted() )
            return; // already started

        // Update the projection matrix for our batch to match the current viewport
        const Rectangle& vp = Game::getInstance()->getViewport();
        if( !vp.isEmpty() )
        {
            Game::getInstance();
            glm::mat4 projectionMatrix = glm::ortho(vp.x, vp.width, vp.height, vp.y, 0.0f, 1.0f);
            _batch->setProjectionMatrix(projectionMatrix);
        }

        _batch->start();
    }


    void Font::finish()
    {
        // Finish any font batches that have been started
        if( _batch->isStarted() )
            _batch->finish();

        for( size_t i = 0, count = _sizes.size(); i < count; ++i )
        {
            SpriteBatch* batch = _sizes[i]->_batch;
            if( batch->isStarted() )
                batch->finish();
        }
    }


    Font* Font::findClosestSize(int size)
    {
        if( size == (int)_size )
            return this;

        int diff = abs(size - (int)_size);
        Font* closest = this;
        for( size_t i = 0, count = _sizes.size(); i < count; ++i )
        {
            auto f = _sizes[i];
            int d = abs(size - (int)f->_size);
            if( d < diff || (d == diff && f->_size > closest->_size) ) // prefer scaling down instead of up
            {
                diff = d;
                closest = f.get();
            }
        }

        return closest;
    }


    void Font::drawText(const char* text, int x, int y, const glm::vec4& color, unsigned int size)
    {
        BOOST_ASSERT(_size);
        BOOST_ASSERT(text);

        if( size == 0 )
        {
            size = _size;
        }
        else
        {
            // Delegate to closest sized font
            Font* f = findClosestSize(size);
            if( f != this )
            {
                f->drawText(text, x, y, color, size);
                return;
            }
        }

        lazyStart();

        float scale = (float)size / _size;
        int spacing = (int)(size * _spacing);

        int xPos = x, yPos = y;

        size_t length = strlen(text);

        BOOST_ASSERT(_batch);
        for( size_t i = 0; i < length; ++i )
        {
            char c = text[i];

            // Draw this character.
            switch( c )
            {
                case ' ':
                    xPos += _glyphs[0].advance;
                    break;
                case '\r':
                case '\n':
                    yPos += size;
                    xPos = x;
                    break;
                case '\t':
                    xPos += _glyphs[0].advance * 4;
                    break;
                default:
                    int index = c - 32; // HACK for ASCII
                    if( index >= 0 && index < (int)_glyphs.size() )
                    {
                        Glyph& g = _glyphs[index];

                        if( getFormat() == DISTANCE_FIELD )
                        {
                            if( _cutoffParam == nullptr )
                                _cutoffParam = _batch->getMaterial()->getParameter("u_cutoff");
                            // TODO: Fix me so that smaller font are much smoother
                            _cutoffParam->setVector2(glm::vec2(1.0, 1.0));
                        }
                        _batch->draw(xPos + (int)(g.bearingX * scale), yPos, g.width * scale, size, g.uvs[0].x, g.uvs[0].y, g.uvs[1].x, g.uvs[1].y, color);
                        xPos += floor(g.advance * scale + spacing);
                        break;
                    }
                    break;
            }
        }
    }


    void Font::drawText(const std::string& text, int x, int y, float red, float green, float blue, float alpha, unsigned int size)
    {
        drawText(text.c_str(), x, y, glm::vec4(red, green, blue, alpha), size);
    }


    void Font::drawText(const std::string& text, const Rectangle& area, const glm::vec4& color, unsigned int size, Justify justify, bool wrap, const Rectangle& clip)
    {
        BOOST_ASSERT(_size);

        if( size == 0 )
        {
            size = _size;
        }
        else
        {
            // Delegate to closest sized font
            Font* f = findClosestSize(size);
            if( f != this )
            {
                f->drawText(text, area, color, size, justify, wrap, clip);
                return;
            }
        }

        lazyStart();

        float scale = (float)size / _size;
        int spacing = (int)(size * _spacing);
        int yPos = area.y;
        const float areaHeight = area.height - size;
        std::vector<int> xPositions;
        std::vector<unsigned int> lineLengths;

        getMeasurementInfo(text.c_str(), area, size, justify, wrap, &xPositions, &yPos, &lineLengths);

        // Now we have the info we need in order to render.
        int xPos = area.x;
        std::vector<int>::const_iterator xPositionsIt = xPositions.begin();
        if( xPositionsIt != xPositions.end() )
        {
            xPos = *xPositionsIt++;
        }

        const char* token = text.c_str();
        unsigned int currentLineLength = 0;
        std::vector<unsigned int>::const_iterator lineLengthsIt;

        while( token[0] != 0 )
        {
            // Handle delimiters until next token.
            if( !handleDelimiters(&token, size, area.x, &xPos, &yPos, &currentLineLength, &xPositionsIt, xPositions.end()) )
            {
                break;
            }

            bool truncated = false;
            unsigned int tokenLength = (unsigned int)strcspn(token, " \r\n\t");
            unsigned int tokenWidth = getTokenWidth(token, tokenLength, size, scale);

            // Wrap if necessary.
            if( wrap && (xPos + (int)tokenWidth > area.x + area.width) )
            {
                yPos += (int)size;
                currentLineLength = tokenLength;

                if( xPositionsIt != xPositions.end() )
                {
                    xPos = *xPositionsIt++;
                }
                else
                {
                    xPos = area.x;
                }
            }

            bool draw = true;
            if( yPos < static_cast<int>(area.y - size) )
            {
                // Skip drawing until line break or wrap.
                draw = false;
            }
            else if( yPos > area.y + areaHeight )
            {
                // Truncate below area's vertical limit.
                break;
            }

            BOOST_ASSERT(_batch);
            for( size_t i = 0; i < tokenLength; ++i )
            {
                char c = token[i];
                int glyphIndex = c - 32; // HACK for ASCII

                if( glyphIndex >= 0 && glyphIndex < (int)_glyphs.size() )
                {
                    Glyph& g = _glyphs[glyphIndex];

                    if( xPos + (int)(g.advance * scale) > area.x + area.width )
                    {
                        // Truncate this line and go on to the next one.
                        truncated = true;
                        break;
                    }
                    else if( xPos >= (int)area.x )
                    {
                        // Draw this character.
                        if( draw )
                        {
                            if( getFormat() == DISTANCE_FIELD )
                            {
                                if( _cutoffParam == nullptr )
                                    _cutoffParam = _batch->getMaterial()->getParameter("u_cutoff");
                                // TODO: Fix me so that smaller font are much smoother
                                _cutoffParam->setVector2(glm::vec2(1.0, 1.0));
                            }
                            if( clip != Rectangle(0, 0, 0, 0) )
                            {
                                _batch->draw(xPos + (int)(g.bearingX * scale), yPos, g.width * scale, size, g.uvs[0].x, g.uvs[0].y, g.uvs[1].x, g.uvs[1].y, color, clip);
                            }
                            else
                            {
                                _batch->draw(xPos + (int)(g.bearingX * scale), yPos, g.width * scale, size, g.uvs[0].x, g.uvs[0].y, g.uvs[1].x, g.uvs[1].y, color);
                            }
                        }
                    }
                    xPos += (int)(g.advance) * scale + spacing;
                }
            }

            if( !truncated )
            {
                token += tokenLength;
            }
            else
            {
                // Skip the rest of this line.
                size_t tokenLength = strcspn(token, "\n");

                if( tokenLength > 0 )
                {
                    // Get first token of next line.
                    token += tokenLength;
                }
            }
        }
    }


    void Font::measureText(const char* text, unsigned int size, unsigned int* width, unsigned int* height)
    {
        BOOST_ASSERT(_size);
        BOOST_ASSERT(text);
        BOOST_ASSERT(width);
        BOOST_ASSERT(height);

        if( size == 0 )
        {
            size = _size;
        }
        else
        {
            // Delegate to closest sized font
            Font* f = findClosestSize(size);
            if( f != this )
            {
                f->measureText(text, size, width, height);
                return;
            }
        }

        const size_t length = strlen(text);
        if( length == 0 )
        {
            *width = 0;
            *height = 0;
            return;
        }

        float scale = (float)size / _size;
        const char* token = text;

        *width = 0;
        *height = size;

        // Measure a line at a time.
        while( token[0] != 0 )
        {
            while( token[0] == '\n' )
            {
                *height += size;
                ++token;
            }

            unsigned int tokenLength = (unsigned int)strcspn(token, "\n");
            unsigned int tokenWidth = getTokenWidth(token, tokenLength, size, scale);
            if( tokenWidth > *width )
            {
                *width = tokenWidth;
            }

            token += tokenLength;
        }
    }


    void Font::measureText(const char* text, const Rectangle& clip, unsigned int size, Rectangle* out, Justify justify, bool wrap, bool ignoreClip)
    {
        BOOST_ASSERT(_size);
        BOOST_ASSERT(text);
        BOOST_ASSERT(out);

        if( size == 0 )
        {
            size = _size;
        }
        else
        {
            // Delegate to closest sized font
            Font* f = findClosestSize(size);
            if( f != this )
            {
                f->measureText(text, clip, size, out, justify, wrap, ignoreClip);
                return;
            }
        }

        if( strlen(text) == 0 )
        {
            out->set(0, 0, 0, 0);
            return;
        }

        float scale = (float)size / _size;
        Justify vAlign = static_cast<Justify>(justify & 0xF0);
        if( vAlign == 0 )
        {
            vAlign = ALIGN_TOP;
        }

        Justify hAlign = static_cast<Justify>(justify & 0x0F);
        if( hAlign == 0 )
        {
            hAlign = ALIGN_LEFT;
        }

        const char* token = text;
        std::vector<bool> emptyLines;
        std::vector<glm::vec2> lines;

        unsigned int lineWidth = 0;
        int yPos = clip.y + size;
        const float viewportHeight = clip.height;

        if( wrap )
        {
            unsigned int delimWidth = 0;
            bool reachedEOF = false;
            while( token[0] != 0 )
            {
                // Handle delimiters until next token.
                char delimiter = token[0];
                while( delimiter == ' ' ||
                    delimiter == '\t' ||
                    delimiter == '\r' ||
                    delimiter == '\n' ||
                    delimiter == 0 )
                {
                    switch( delimiter )
                    {
                        case ' ':
                            delimWidth += _glyphs[0].advance;
                            break;
                        case '\r':
                        case '\n':
                            // Add line-height to vertical cursor.
                            yPos += size;

                            if( lineWidth > 0 )
                            {
                                // Determine horizontal position and width.
                                int hWhitespace = clip.width - lineWidth;
                                int xPos = clip.x;
                                if( hAlign == ALIGN_HCENTER )
                                {
                                    xPos += hWhitespace / 2;
                                }
                                else if( hAlign == ALIGN_RIGHT )
                                {
                                    xPos += hWhitespace;
                                }

                                // Record this line's size.
                                emptyLines.push_back(false);
                                lines.push_back(glm::vec2(xPos, lineWidth));
                            }
                            else
                            {
                                // Record the existence of an empty line.
                                emptyLines.push_back(true);
                                lines.push_back(glm::vec2(std::numeric_limits<float>::max(), 0));
                            }

                            lineWidth = 0;
                            delimWidth = 0;
                            break;
                        case '\t':
                            delimWidth += _glyphs[0].advance * 4;
                            break;
                        case 0:
                            reachedEOF = true;
                            break;
                    }

                    if( reachedEOF )
                    {
                        break;
                    }

                    token++;
                    delimiter = token[0];
                }

                if( reachedEOF )
                {
                    break;
                }

                // Measure the next token.
                unsigned int tokenLength = (unsigned int)strcspn(token, " \r\n\t");
                unsigned int tokenWidth = getTokenWidth(token, tokenLength, size, scale);

                // Wrap if necessary.
                if( lineWidth + tokenWidth + delimWidth > clip.width )
                {
                    // Add line-height to vertical cursor.
                    yPos += size;

                    // Determine horizontal position and width.
                    int hWhitespace = clip.width - lineWidth;
                    int xPos = clip.x;
                    if( hAlign == ALIGN_HCENTER )
                    {
                        xPos += hWhitespace / 2;
                    }
                    else if( hAlign == ALIGN_RIGHT )
                    {
                        xPos += hWhitespace;
                    }

                    // Record this line's size.
                    emptyLines.push_back(false);
                    lines.push_back(glm::vec2(xPos, lineWidth));
                    lineWidth = 0;
                }
                else
                {
                    lineWidth += delimWidth;
                }

                delimWidth = 0;
                lineWidth += tokenWidth;
                token += tokenLength;
            }
        }
        else
        {
            // Measure a whole line at a time.
            int emptyLinesCount = 0;
            while( token[0] != 0 )
            {
                // Handle any number of consecutive newlines.
                bool nextLine = true;
                while( token[0] == '\n' )
                {
                    if( nextLine )
                    {
                        // Add line-height to vertical cursor.
                        yPos += size * (emptyLinesCount + 1);
                        nextLine = false;
                        emptyLinesCount = 0;
                        emptyLines.push_back(false);
                    }
                    else
                    {
                        // Record the existence of an empty line.
                        ++emptyLinesCount;
                        emptyLines.push_back(true);
                        lines.push_back(glm::vec2(std::numeric_limits<float>::max(), 0));
                    }

                    token++;
                }

                // Measure the next line.
                unsigned int tokenLength = (unsigned int)strcspn(token, "\n");
                lineWidth = getTokenWidth(token, tokenLength, size, scale);

                // Determine horizontal position and width.
                int xPos = clip.x;
                int hWhitespace = clip.width - lineWidth;
                if( hAlign == ALIGN_HCENTER )
                {
                    xPos += hWhitespace / 2;
                }
                else if( hAlign == ALIGN_RIGHT )
                {
                    xPos += hWhitespace;
                }

                // Record this line's size.
                lines.push_back(glm::vec2(xPos, lineWidth));

                token += tokenLength;
            }

            yPos += size;
        }

        if( wrap )
        {
            // Record the size of the last line.
            int hWhitespace = clip.width - lineWidth;
            int xPos = clip.x;
            if( hAlign == ALIGN_HCENTER )
            {
                xPos += hWhitespace / 2;
            }
            else if( hAlign == ALIGN_RIGHT )
            {
                xPos += hWhitespace;
            }

            lines.push_back(glm::vec2(xPos, lineWidth));
        }

        int x = std::numeric_limits<int>::max();
        int y = clip.y;
        unsigned int width = 0;
        int height = yPos - clip.y;

        // Calculate top of text without clipping.
        int vWhitespace = viewportHeight - height;
        if( vAlign == ALIGN_VCENTER )
        {
            y += vWhitespace / 2;
        }
        else if( vAlign == ALIGN_BOTTOM )
        {
            y += vWhitespace;
        }

        int clippedTop = 0;
        int clippedBottom = 0;
        if( !ignoreClip )
        {
            // Trim rect to fit text that would actually be drawn within the given clip.
            if( y >= clip.y )
            {
                // Text goes off the bottom of the clip.
                clippedBottom = (height - viewportHeight) / size + 1;
                if( clippedBottom > 0 )
                {
                    // Also need to crop empty lines above non-empty lines that have been clipped.
                    size_t emptyIndex = emptyLines.size() - clippedBottom;
                    while( emptyIndex < emptyLines.size() && emptyLines[emptyIndex] == true )
                    {
                        height -= size;
                        emptyIndex++;
                    }

                    height -= size * clippedBottom;
                }
                else
                {
                    clippedBottom = 0;
                }
            }
            else
            {
                // Text goes above the top of the clip.
                clippedTop = (clip.y - y) / size + 1;
                if( clippedTop < 0 )
                {
                    clippedTop = 0;
                }

                // Also need to crop empty lines below non-empty lines that have been clipped.
                size_t emptyIndex = clippedTop;
                while( emptyIndex < emptyLines.size() && emptyLines[emptyIndex] == true )
                {
                    y += size;
                    height -= size;
                    emptyIndex++;
                }

                if( vAlign == ALIGN_VCENTER )
                {
                    // In this case lines may be clipped off the bottom as well.
                    clippedBottom = (height - viewportHeight + vWhitespace / 2 + 0.01) / size + 1;
                    if( clippedBottom > 0 )
                    {
                        emptyIndex = emptyLines.size() - clippedBottom;
                        while( emptyIndex < emptyLines.size() && emptyLines[emptyIndex] == true )
                        {
                            height -= size;
                            emptyIndex++;
                        }

                        height -= size * clippedBottom;
                    }
                    else
                    {
                        clippedBottom = 0;
                    }
                }

                y = y + size * clippedTop;
                height = height - size * clippedTop;
            }
        }

        // Determine left-most x coordinate and largest width out of lines that have not been clipped.
        for( int i = clippedTop; i < (int)lines.size() - clippedBottom; ++i )
        {
            if( lines[i].x < x )
            {
                x = lines[i].x;
            }
            if( lines[i].y > width )
            {
                width = lines[i].y;
            }
        }

        if( !ignoreClip )
        {
            // Guarantee that the output rect will fit within the clip.
            out->x = (x >= clip.x) ? x : clip.x;
            out->y = (y >= clip.y) ? y : clip.y;
            out->width = (width <= clip.width) ? width : clip.width;
            out->height = (height <= viewportHeight) ? height : viewportHeight;
        }
        else
        {
            out->x = x;
            out->y = y;
            out->width = width;
            out->height = height;
        }
    }


    void Font::getMeasurementInfo(const char* text, const Rectangle& area, unsigned int size, Justify justify, bool wrap,
                                  std::vector<int>* xPositions, int* yPosition, std::vector<unsigned int>* lineLengths)
    {
        BOOST_ASSERT(_size);
        BOOST_ASSERT(text);
        BOOST_ASSERT(yPosition);

        if( size == 0 )
            size = _size;

        float scale = (float)size / _size;

        Justify vAlign = static_cast<Justify>(justify & 0xF0);
        if( vAlign == 0 )
        {
            vAlign = ALIGN_TOP;
        }

        Justify hAlign = static_cast<Justify>(justify & 0x0F);
        if( hAlign == 0 )
        {
            hAlign = ALIGN_LEFT;
        }

        const char* token = text;
        const float areaHeight = area.height - size;

        // For alignments other than top-left, need to calculate the y position to begin drawing from
        // and the starting x position of each line.  For right-to-left text, need to determine
        // the number of characters on each line.
        if( vAlign != ALIGN_TOP || hAlign != ALIGN_LEFT )
        {
            int lineWidth = 0;
            int delimWidth = 0;

            if( wrap )
            {
                // Go a word at a time.
                bool reachedEOF = false;
                unsigned int lineLength = 0;
                while( token[0] != 0 )
                {
                    unsigned int tokenWidth = 0;

                    // Handle delimiters until next token.
                    char delimiter = token[0];
                    while( delimiter == ' ' ||
                        delimiter == '\t' ||
                        delimiter == '\r' ||
                        delimiter == '\n' ||
                        delimiter == 0 )
                    {
                        switch( delimiter )
                        {
                            case ' ':
                                delimWidth += _glyphs[0].advance;
                                lineLength++;
                                break;
                            case '\r':
                            case '\n':
                                *yPosition += size;

                                if( lineWidth > 0 )
                                {
                                    addLineInfo(area, lineWidth, lineLength, hAlign, xPositions, lineLengths);
                                }

                                lineWidth = 0;
                                lineLength = 0;
                                delimWidth = 0;
                                break;
                            case '\t':
                                delimWidth += _glyphs[0].advance * 4;
                                lineLength++;
                                break;
                            case 0:
                                reachedEOF = true;
                                break;
                        }

                        if( reachedEOF )
                        {
                            break;
                        }

                        token++;
                        delimiter = token[0];
                    }

                    if( reachedEOF || token == nullptr )
                    {
                        break;
                    }

                    unsigned int tokenLength = (unsigned int)strcspn(token, " \r\n\t");
                    tokenWidth += getTokenWidth(token, tokenLength, size, scale);

                    // Wrap if necessary.
                    if( lineWidth + tokenWidth + delimWidth > area.width )
                    {
                        *yPosition += size;

                        // Push position of current line.
                        if( lineLength )
                        {
                            addLineInfo(area, lineWidth, lineLength - 1, hAlign, xPositions, lineLengths);
                        }
                        else
                        {
                            addLineInfo(area, lineWidth, tokenLength, hAlign, xPositions, lineLengths);
                        }

                        // Move token to the next line.
                        lineWidth = 0;
                        lineLength = 0;
                        delimWidth = 0;
                    }
                    else
                    {
                        lineWidth += delimWidth;
                        delimWidth = 0;
                    }

                    lineWidth += tokenWidth;
                    lineLength += tokenLength;
                    token += tokenLength;
                }

                // Final calculation of vertical position.
                int textHeight = *yPosition - area.y;
                int vWhiteSpace = areaHeight - textHeight;
                if( vAlign == ALIGN_VCENTER )
                {
                    *yPosition = area.y + vWhiteSpace / 2;
                }
                else if( vAlign == ALIGN_BOTTOM )
                {
                    *yPosition = area.y + vWhiteSpace;
                }

                // Calculation of final horizontal position.
                addLineInfo(area, lineWidth, lineLength, hAlign, xPositions, lineLengths);
            }
            else
            {
                // Go a line at a time.
                while( token[0] != 0 )
                {
                    char delimiter = token[0];
                    while( delimiter == '\n' )
                    {
                        *yPosition += size;
                        ++token;
                        delimiter = token[0];
                    }

                    auto tokenLength = strcspn(token, "\n");
                    if( tokenLength == 0 )
                    {
                        tokenLength = strlen(token);
                    }

                    int lineWidth = getTokenWidth(token, tokenLength, size, scale);
                    addLineInfo(area, lineWidth, tokenLength, hAlign, xPositions, lineLengths);

                    token += tokenLength;
                }

                int textHeight = *yPosition - area.y;
                int vWhiteSpace = areaHeight - textHeight;
                if( vAlign == ALIGN_VCENTER )
                {
                    *yPosition = area.y + vWhiteSpace / 2;
                }
                else if( vAlign == ALIGN_BOTTOM )
                {
                    *yPosition = area.y + vWhiteSpace;
                }
            }

            if( vAlign == ALIGN_TOP )
            {
                *yPosition = area.y;
            }
        }
    }


    float Font::getCharacterSpacing() const
    {
        return _spacing;
    }


    void Font::setCharacterSpacing(float spacing)
    {
        _spacing = spacing;
    }


    int Font::getIndexAtLocation(const char* text, const Rectangle& area, unsigned int size, const glm::vec2& inLocation, glm::vec2* outLocation,
                                 Justify justify, bool wrap)
    {
        return getIndexOrLocation(text, area, size, inLocation, outLocation, -1, justify, wrap);
    }


    void Font::getLocationAtIndex(const char* text, const Rectangle& clip, unsigned int size, glm::vec2* outLocation, const unsigned int destIndex,
                                  Justify justify, bool wrap)
    {
        getIndexOrLocation(text, clip, size, *outLocation, outLocation, (const int)destIndex, justify, wrap);
    }


    int Font::getIndexOrLocation(const char* text, const Rectangle& area, unsigned int size, const glm::vec2& inLocation, glm::vec2* outLocation,
                                 const int destIndex, Justify justify, bool wrap)
    {
        BOOST_ASSERT(_size);
        BOOST_ASSERT(text);
        BOOST_ASSERT(outLocation);

        if( size == 0 )
        {
            size = _size;
        }
        else
        {
            // Delegate to closest sized font
            Font* f = findClosestSize(size);
            if( f != this )
            {
                return f->getIndexOrLocation(text, area, size, inLocation, outLocation, destIndex, justify, wrap);
            }
        }

        unsigned int charIndex = 0;

        // Essentially need to measure text until we reach inLocation.
        float scale = (float)size / _size;
        int spacing = (int)(size * _spacing);
        int yPos = area.y;
        const float areaHeight = area.height - size;
        std::vector<int> xPositions;
        std::vector<unsigned int> lineLengths;

        getMeasurementInfo(text, area, size, justify, wrap, &xPositions, &yPos, &lineLengths);

        int xPos = area.x;
        std::vector<int>::const_iterator xPositionsIt = xPositions.begin();
        if( xPositionsIt != xPositions.end() )
        {
            xPos = *xPositionsIt++;
        }

        const char* token = text;

        unsigned int currentLineLength = 0;

        while( token[0] != 0 )
        {
            // Handle delimiters until next token.
            unsigned int delimLength = 0;
            int result;
            if( destIndex == -1 )
            {
                result = handleDelimiters(&token, size, area.x, &xPos, &yPos, &delimLength, &xPositionsIt, xPositions.end(), &charIndex, &inLocation);
            }
            else
            {
                result = handleDelimiters(&token, size, area.x, &xPos, &yPos, &delimLength, &xPositionsIt, xPositions.end(), &charIndex, nullptr, charIndex, destIndex);
            }

            currentLineLength += delimLength;
            if( result == 0 || result == 2 )
            {
                outLocation->x = xPos;
                outLocation->y = yPos;
                return charIndex;
            }

            if( destIndex == (int)charIndex ||
            (destIndex == -1 &&
                inLocation.x >= xPos && inLocation.x < xPos + spacing &&
                inLocation.y >= yPos && inLocation.y < yPos + size) )
            {
                outLocation->x = xPos;
                outLocation->y = yPos;
                return charIndex;
            }

            bool truncated = false;
            unsigned int tokenLength = (unsigned int)strcspn(token, " \r\n\t");
            unsigned int tokenWidth = getTokenWidth(token, tokenLength, size, scale);

            // Wrap if necessary.
            if( wrap && (xPos + (int)tokenWidth > area.x + area.width) )
            {
                yPos += size;
                currentLineLength = tokenLength;

                if( xPositionsIt != xPositions.end() )
                {
                    xPos = *xPositionsIt++;
                }
                else
                {
                    xPos = area.x;
                }
            }

            if( yPos > area.y + areaHeight )
            {
                // Truncate below area's vertical limit.
                break;
            }

            for( size_t i = 0; i < tokenLength; ++i )
            {
                char c = token[i];
                int glyphIndex = c - 32; // HACK for ASCII

                if( glyphIndex >= 0 && glyphIndex < (int)_glyphs.size() )
                {
                    Glyph& g = _glyphs[glyphIndex];

                    if( xPos + (int)(g.advance * scale) > area.x + area.width )
                    {
                        // Truncate this line and go on to the next one.
                        truncated = true;
                        break;
                    }

                    // Check against inLocation.
                    if( destIndex == (int)charIndex ||
                    (destIndex == -1 &&
                        inLocation.x >= xPos && inLocation.x < floor(xPos + g.width * scale + spacing) &&
                        inLocation.y >= yPos && inLocation.y < yPos + size) )
                    {
                        outLocation->x = xPos;
                        outLocation->y = yPos;
                        return charIndex;
                    }

                    xPos += floor(g.advance * scale + spacing);
                    charIndex++;
                }
            }

            if( !truncated )
            {
                token += tokenLength;
            }
            else
            {
                // Skip the rest of this line.
                unsigned int tokenLength = (unsigned int)strcspn(token, "\n");

                if( tokenLength > 0 )
                {
                    // Get first token of next line.
                    token += tokenLength;
                    charIndex += tokenLength;
                }
            }
        }

        if( destIndex == (int)charIndex ||
        (destIndex == -1 &&
            inLocation.x >= xPos && inLocation.x < xPos + spacing &&
            inLocation.y >= yPos && inLocation.y < yPos + size) )
        {
            outLocation->x = xPos;
            outLocation->y = yPos;
            return charIndex;
        }

        return -1;
    }


    unsigned int Font::getTokenWidth(const char* token, unsigned int length, unsigned int size, float scale) const
    {
        BOOST_ASSERT(token);

        if( size == 0 )
            size = _size;

        int spacing = (int)(size * _spacing);

        // Calculate width of word or line.
        unsigned int tokenWidth = 0;
        for( unsigned int i = 0; i < length; ++i )
        {
            char c = token[i];
            switch( c )
            {
                case ' ':
                    tokenWidth += _glyphs[0].advance;
                    break;
                case '\t':
                    tokenWidth += _glyphs[0].advance * 4;
                    break;
                default:
                    int glyphIndex = c - 32;
                    if( glyphIndex >= 0 && glyphIndex < (int)_glyphs.size() )
                    {
                        const Glyph& g = _glyphs[glyphIndex];
                        tokenWidth += floor(g.advance * scale + spacing);
                    }
                    break;
            }
        }

        return tokenWidth;
    }


    unsigned int Font::getReversedTokenLength(const char* token, const char* bufStart)
    {
        BOOST_ASSERT(token);
        BOOST_ASSERT(bufStart);

        const char* cursor = token;
        char c = cursor[0];
        unsigned int length = 0;

        while( cursor != bufStart && c != ' ' && c != '\r' && c != '\n' && c != '\t' )
        {
            length++;
            cursor--;
            c = cursor[0];
        }

        if( cursor == bufStart )
        {
            length++;
        }

        return length;
    }


    int Font::handleDelimiters(const char** token, const unsigned int size, const int areaX, int* xPos, int* yPos, unsigned int* lineLength,
                               std::vector<int>::const_iterator* xPositionsIt, std::vector<int>::const_iterator xPositionsEnd, unsigned int* charIndex,
                               const glm::vec2* stopAtPosition, const int currentIndex, const int destIndex)
    {
        BOOST_ASSERT(token);
        BOOST_ASSERT(*token);
        BOOST_ASSERT(xPos);
        BOOST_ASSERT(yPos);
        BOOST_ASSERT(lineLength);
        BOOST_ASSERT(xPositionsIt);

        char delimiter = *token[0];
        bool nextLine = true;
        while( delimiter == ' ' ||
            delimiter == '\t' ||
            delimiter == '\r' ||
            delimiter == '\n' ||
            delimiter == 0 )
        {
            if( (stopAtPosition &&
                    stopAtPosition->x >= *xPos && stopAtPosition->x < *xPos + ((int)size >> 1) &&
                    stopAtPosition->y >= *yPos && stopAtPosition->y < *yPos + (int)size) ||
                (currentIndex >= 0 && destIndex >= 0 && currentIndex + (int)*lineLength == destIndex) )
            {
                // Success + stopAtPosition was reached.
                return 2;
            }

            switch( delimiter )
            {
                case ' ':
                    *xPos += _glyphs[0].advance;
                    (*lineLength)++;
                    if( charIndex )
                    {
                        (*charIndex)++;
                    }
                    break;
                case '\r':
                case '\n':
                    *yPos += size;

                    // Only use next xPos for first newline character (in case of multiple consecutive newlines).
                    if( nextLine )
                    {
                        if( *xPositionsIt != xPositionsEnd )
                        {
                            *xPos = **xPositionsIt;
                            (*xPositionsIt)++;
                        }
                        else
                        {
                            *xPos = areaX;
                        }
                        nextLine = false;
                        *lineLength = 0;
                        if( charIndex )
                        {
                            (*charIndex)++;
                        }
                    }
                    break;
                case '\t':
                    *xPos += _glyphs[0].advance * 4;
                    (*lineLength)++;
                    if( charIndex )
                    {
                        (*charIndex)++;
                    }
                    break;
                case 0:
                    // EOF reached.
                    return 0;
            }

            ++*token;
            delimiter = *token[0];
        }

        // Success.
        return 1;
    }


    void Font::addLineInfo(const Rectangle& area, int lineWidth, int lineLength, Justify hAlign,
                           std::vector<int>* xPositions, std::vector<unsigned int>* lineLengths)
    {
        int hWhitespace = area.width - lineWidth;
        if( hAlign == ALIGN_HCENTER )
        {
            BOOST_ASSERT(xPositions);
            (*xPositions).push_back(area.x + hWhitespace / 2);
        }
        else if( hAlign == ALIGN_RIGHT )
        {
            BOOST_ASSERT(xPositions);
            (*xPositions).push_back(area.x + hWhitespace);
        }
    }


    SpriteBatch* Font::getSpriteBatch(unsigned int size) const
    {
        if( size == 0 )
            return _batch;

        // Find the closest sized child font
        return const_cast<Font*>(this)->findClosestSize(size)->_batch;
    }
}
