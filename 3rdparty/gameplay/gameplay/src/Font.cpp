#include "Base.h"
#include "Font.h"
#include "Game.h"
#include "MaterialParameter.h"

#include <glm/gtc/matrix_transform.hpp>

#include <boost/log/trivial.hpp>

// Default font shaders
#define FONT_VSH "shaders/font.vert"
#define FONT_FSH "shaders/font.frag"


namespace gameplay
{
    static std::shared_ptr<ShaderProgram> __fontProgram = nullptr;


    Font::Font(const std::shared_ptr<Texture>& texture, size_t cellSizeX, size_t cellSizeY)
        : _texture{texture}
        , _batch{nullptr}
        , _cellSizeX{cellSizeX}
        , _cellSizeY{cellSizeY}
    {
        BOOST_ASSERT(texture);

        // Create the effect for the font's sprite batch.
        if( __fontProgram == nullptr )
        {
            __fontProgram = ShaderProgram::createFromFile(FONT_VSH, FONT_FSH, {});
            if( __fontProgram == nullptr )
            {
                BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create effect for font."));
            }
        }

        // Create batch for the font.
        auto batch = SpriteBatch::create(texture, __fontProgram);

        if( batch == nullptr )
        {
            BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create batch for font."));
        }

        // Add linear filtering for better font quality.
        auto sampler = batch->getSampler();
        sampler->setFilterMode(Texture::LINEAR_MIPMAP_LINEAR, Texture::LINEAR);
        sampler->setWrapMode(Texture::CLAMP, Texture::CLAMP);

        _batch = batch;
    }


    Font::~Font() = default;


    bool Font::isCharacterSupported(int character)
    {
        return character >= 0x20 && character <= 0x7f;
    }


    void Font::lazyStart()
    {
        if( _batch->isStarted() )
            return; // already started

        // Update the projection matrix for our batch to match the current viewport
        const Rectangle& vp = Game::getInstance()->getViewport();
        if( !vp.isEmpty() )
        {
            glm::mat4 projectionMatrix = glm::ortho(vp.x, vp.width, vp.height, vp.y, 0.0f, 1.0f);
            _batch->setProjectionMatrix(projectionMatrix);
        }

        _batch->start();
    }


    void Font::finish()
    {
        // Finish any font batches that have been started
        if( _batch->isStarted() )
            _batch->finishAndDraw();
    }


    void Font::drawText(const char* text, int x, int y, const glm::vec4& color)
    {
        BOOST_ASSERT(text);

        lazyStart();

        int xPos = x, yPos = y;

        size_t length = strlen(text);
        const auto csx = static_cast<float>(_cellSizeX) / _texture->getWidth() * 10;
        const auto csy = static_cast<float>(_cellSizeY) / _texture->getHeight() * 10;

        BOOST_ASSERT(_batch);
        for( size_t i = 0; i < length; ++i )
        {
            char c = text[i];

            // Draw this character.
            switch( c )
            {
                case ' ':
                    xPos += _cellSizeX;
                    break;
                case '\r':
                case '\n':
                    yPos += _cellSizeY;
                    xPos = x;
                    break;
                case '\t':
                    xPos += _cellSizeX * 4;
                    break;
                default:
                    if( c >= 0x20 && c <= 0x7f )
                    {
                        const auto charsPerRow = _texture->getWidth() / _cellSizeX;
                        const auto cx = float(c % charsPerRow) * csx;
                        const auto cy = float(c / charsPerRow) * csy;

                        _batch->draw(xPos, yPos, _cellSizeX, _cellSizeY, cx, cy, cx + csx, cy + csy, color);
                        xPos += _cellSizeX;
                        break;
                    }
                    break;
            }
        }
    }


    void Font::drawText(const std::string& text, int x, int y, float red, float green, float blue, float alpha)
    {
        drawText(text.c_str(), x, y, glm::vec4(red, green, blue, alpha));
    }


    const std::shared_ptr<SpriteBatch>& Font::getSpriteBatch() const
    {
        return _batch;
    }
}
