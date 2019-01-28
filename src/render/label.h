#include "gameplay.h"
#include "loader/color.h"
#include "loader/datatypes.h"
#include "util/cimgwrapper.h"

#include <cstdint>
#include <string>

namespace level
{
class Level;
}

namespace render
{
namespace
{
constexpr const int FontBaseScale = 0x10000;
}

class CachedFont
{
    std::vector<util::CImgWrapper> m_images;
    const int m_scaleX;
    const int m_scaleY;

    static util::CImgWrapper extractChar(const loader::Sprite& sprite, const int scaleX, const int scaleY)
    {
        BOOST_ASSERT( sprite.image != nullptr );

        const auto dstW = std::lround( (sprite.t1.x - sprite.t0.x) * 256 * scaleX / FontBaseScale );
        const auto dstH = std::lround( (sprite.t1.y - sprite.t0.y) * 256 * scaleY / FontBaseScale );

        util::CImgWrapper src{
                reinterpret_cast<const uint8_t*>(sprite.image->getData().data()),
                sprite.image->getWidth(),
                sprite.image->getHeight(),
                true
        };
        src.crop(
                gsl::narrow_cast<int>( sprite.t0.x * sprite.image->getWidth() ),
                gsl::narrow_cast<int>( sprite.t0.y * sprite.image->getHeight() ),
                gsl::narrow_cast<int>( sprite.t1.x * sprite.image->getWidth() - 1 ),
                gsl::narrow_cast<int>( sprite.t1.y * sprite.image->getHeight() - 1 )
        );
        src.resize( dstW, dstH );

        return src;
    }

public:
    explicit CachedFont(const loader::SpriteSequence& sequence,
                        const int scaleX = FontBaseScale,
                        const int scaleY = FontBaseScale)
            : m_scaleX{scaleX}
            , m_scaleY{scaleY}
    {
        for( const auto& spr : sequence.sprites )
        {
            m_images.emplace_back( extractChar( spr, scaleX, scaleY ) );
        }
    }

    const util::CImgWrapper& get(size_t n) const
    {
        return m_images.at( n );
    }

    void draw(size_t n, const int x, const int y, gameplay::gl::Image<gameplay::gl::RGBA8>& img)
    {
        auto& src = m_images.at( n );

        for( int dy = 0; dy < src.height(); ++dy )
        {
            for( int dx = 0; dx < src.width(); ++dx )
            {
                img.set( x + dx, y + dy, src( dx, dy ), true );
            }
        }
    }

    int getScaleX() const noexcept
    {
        return m_scaleX;
    }

    int getScaleY() const noexcept
    {
        return m_scaleY;
    }
};


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
    int scaleX = FontBaseScale;
    int scaleY = FontBaseScale;
    std::string text;

    explicit Label(int16_t xpos, int16_t ypos, const std::string& string)
            : posX{xpos}
            , posY{ypos}
            , text{string, 0, std::min( std::string::size_type( 64 ), string.size() )}
    {
    }

    void draw(CachedFont& font, gameplay::gl::Image<gameplay::gl::RGBA8>& img, const level::Level& level) const;

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
            this->blinkTime = blinkTime;
            timeout = blinkTime;
        }
    }
};
}
