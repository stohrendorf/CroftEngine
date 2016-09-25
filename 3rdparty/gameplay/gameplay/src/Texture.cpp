#include "Base.h"
#include "Image.h"
#include "Texture.h"
#include "FileSystem.h"

namespace gameplay
{
    static std::vector<Texture*> __textureCache;
    static TextureHandle __currentTextureId = 0;
    static Texture::Type __currentTextureType = Texture::TEXTURE_2D;


    Texture::Texture()
        : _handle(0)
        , _type(static_cast<Texture::Type>(0))
        , _width(0)
        , _height(0)
        , _mipmapped(false)
        , _cached(false)
        , _compressed(false)
        , _wrapS(Texture::REPEAT)
        , _wrapT(Texture::REPEAT)
        , _wrapR(Texture::REPEAT)
        , _minFilter(Texture::NEAREST_MIPMAP_LINEAR)
        , _magFilter(Texture::LINEAR)
    {
    }


    Texture::~Texture()
    {
        if( _handle )
        {
            GL_ASSERT( glDeleteTextures(1, &_handle) );
            _handle = 0;
        }

        // Remove ourself from the texture cache.
        if( _cached )
        {
            std::vector<Texture*>::iterator itr = std::find(__textureCache.begin(), __textureCache.end(), this);
            if( itr != __textureCache.end() )
            {
                __textureCache.erase(itr);
            }
        }
    }


    std::shared_ptr<Texture> Texture::create(const std::shared_ptr<Image>& image, bool generateMipmaps)
    {
        BOOST_ASSERT( image );

        return create(image->getWidth(), image->getHeight(), image->getData(), generateMipmaps);
    }


    std::shared_ptr<Texture> Texture::create(unsigned int width, unsigned int height, const std::vector<glm::vec4>& data, bool generateMipmaps, Texture::Type type)
    {
        BOOST_ASSERT( type == Texture::TEXTURE_2D || type == Texture::TEXTURE_CUBE );

        GLenum target = static_cast<GLenum>(type);

        // Create the texture.
        GLuint textureId;
        GL_ASSERT( glGenTextures(1, &textureId) );
        GL_ASSERT( glBindTexture(target, textureId) );
        GL_ASSERT( glPixelStorei(GL_UNPACK_ALIGNMENT, 1) );
        if( generateMipmaps && !std::addressof(glGenerateMipmap) )
        GL_ASSERT( glTexParameteri(target, GL_GENERATE_MIPMAP, GL_TRUE) );

        // Load the texture
        if( type == Texture::TEXTURE_2D )
        {
            // Texture 2D
            GL_ASSERT( glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, data.empty() ? nullptr : data.data()) );
        }
        else
        {
            // Texture Cube
            for( unsigned int i = 0; i < 6; i++ )
            {
                const glm::vec4* texturePtr = data.empty() ? nullptr : &data[i];
                GL_ASSERT( glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, texturePtr) );
            }
        }

        // Set initial minification filter based on whether or not mipmaping was enabled.
        Filter minFilter = generateMipmaps ? NEAREST_MIPMAP_LINEAR : LINEAR;
        GL_ASSERT( glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter) );

        auto texture = std::make_shared<Texture>();
        texture->_handle = textureId;
        texture->_type = type;
        texture->_width = width;
        texture->_height = height;
        texture->_minFilter = minFilter;
        if( generateMipmaps )
        {
            texture->generateMipmaps();
        }

        // Restore the texture id
        GL_ASSERT( glBindTexture(static_cast<GLenum>(__currentTextureType), __currentTextureId) );

        return texture;
    }


    Texture* Texture::create(TextureHandle handle, int width, int height)
    {
        BOOST_ASSERT( handle );

        Texture* texture = new Texture();
        if( glIsTexture(handle) )
        {
            // There is no real way to query for texture type, but an error will be returned if a cube texture is bound to a 2D texture... so check for that
            glBindTexture(GL_TEXTURE_CUBE_MAP, handle);
            if( glGetError() == GL_NO_ERROR )
            {
                texture->_type = TEXTURE_CUBE;
            }
            else
            {
                // For now, it's either or. But if 3D textures and others are added, it might be useful to simply test a bunch of bindings and seeing which one doesn't error out
                texture->_type = TEXTURE_2D;
            }

            // Restore the texture id
            GL_ASSERT( glBindTexture(static_cast<GLenum>(__currentTextureType), __currentTextureId) );
        }
        texture->_handle = handle;
        texture->_width = width;
        texture->_height = height;

        return texture;
    }


    void Texture::setData(const glm::vec4* data)
    {
        // Don't work with any compressed or cached textures
        BOOST_ASSERT( data );
        BOOST_ASSERT( (!_compressed) );
        BOOST_ASSERT( (!_cached) );

        GL_ASSERT( glBindTexture(static_cast<GLenum>(_type), _handle) );

        if( _type == Texture::TEXTURE_2D )
        {
            GL_ASSERT( glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _width, _height, GL_RGBA, GL_FLOAT, data) );
        }
        else
        {
            // Texture Cube
            for( unsigned int i = 0; i < 6; i++ )
            {
                GL_ASSERT( glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0, 0, _width, _height, GL_RGBA, GL_FLOAT, &data[i]) );
            }
        }

        if( _mipmapped )
        {
            generateMipmaps();
        }

        // Restore the texture id
        GL_ASSERT( glBindTexture(static_cast<GLenum>(__currentTextureType), __currentTextureId) );
    }


    int Texture::getMaskByteIndex(unsigned int mask)
    {
        switch( mask )
        {
            case 0xff000000:
                return 3;
            case 0x00ff0000:
                return 2;
            case 0x0000ff00:
                return 1;
            case 0x000000ff:
                return 0;
            default:
                return -1; // no or invalid mask
        }
    }


    Texture::Type Texture::getType() const
    {
        return _type;
    }


    unsigned int Texture::getWidth() const
    {
        return _width;
    }


    unsigned int Texture::getHeight() const
    {
        return _height;
    }


    TextureHandle Texture::getHandle() const
    {
        return _handle;
    }


    void Texture::generateMipmaps()
    {
        if( !_mipmapped )
        {
            GLenum target = static_cast<GLenum>(_type);
            GL_ASSERT( glBindTexture(target, _handle) );
            GL_ASSERT( glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST) );
            if( std::addressof(glGenerateMipmap) != nullptr )
                GL_ASSERT( glGenerateMipmap(target) );

            _mipmapped = true;

            // Restore the texture id
            GL_ASSERT( glBindTexture(static_cast<GLenum>(__currentTextureType), __currentTextureId) );
        }
    }


    bool Texture::isMipmapped() const
    {
        return _mipmapped;
    }


    bool Texture::isCompressed() const
    {
        return _compressed;
    }


    Texture::Sampler::Sampler(const std::shared_ptr<Texture>& texture)
        : _texture(texture)
        , _wrapS(Texture::REPEAT)
        , _wrapT(Texture::REPEAT)
        , _wrapR(Texture::REPEAT)
    {
        BOOST_ASSERT( texture );
        _minFilter = texture->_minFilter;
        _magFilter = texture->_magFilter;
    }


    Texture::Sampler::~Sampler() = default;


    std::shared_ptr<Texture::Sampler> Texture::Sampler::create(const std::shared_ptr<Texture>& texture)
    {
        BOOST_ASSERT( texture );
        BOOST_ASSERT( texture->_type == Texture::TEXTURE_2D || texture->_type == Texture::TEXTURE_CUBE );
        return std::make_shared<Sampler>(texture);
    }


    void Texture::Sampler::setWrapMode(Wrap wrapS, Wrap wrapT, Wrap wrapR)
    {
        _wrapS = wrapS;
        _wrapT = wrapT;
        _wrapR = wrapR;
    }


    void Texture::Sampler::setFilterMode(Filter minificationFilter, Filter magnificationFilter)
    {
        _minFilter = minificationFilter;
        _magFilter = magnificationFilter;
    }


    const std::shared_ptr<Texture>& Texture::Sampler::getTexture() const
    {
        return _texture;
    }


    void Texture::Sampler::bind()
    {
        BOOST_ASSERT( _texture );

        GLenum target = static_cast<GLenum>(_texture->_type);
        if( __currentTextureId != _texture->_handle )
        {
            GL_ASSERT( glBindTexture(target, _texture->_handle) );
            __currentTextureId = _texture->_handle;
            __currentTextureType = _texture->_type;
        }

        if( _texture->_minFilter != _minFilter )
        {
            _texture->_minFilter = _minFilter;
            GL_ASSERT( glTexParameteri(target, GL_TEXTURE_MIN_FILTER, static_cast<GLenum>(_minFilter)) );
        }

        if( _texture->_magFilter != _magFilter )
        {
            _texture->_magFilter = _magFilter;
            GL_ASSERT( glTexParameteri(target, GL_TEXTURE_MAG_FILTER, static_cast<GLenum>(_magFilter)) );
        }

        if( _texture->_wrapS != _wrapS )
        {
            _texture->_wrapS = _wrapS;
            GL_ASSERT( glTexParameteri(target, GL_TEXTURE_WRAP_S, static_cast<GLenum>(_wrapS)) );
        }

        if( _texture->_wrapT != _wrapT )
        {
            _texture->_wrapT = _wrapT;
            GL_ASSERT( glTexParameteri(target, GL_TEXTURE_WRAP_T, static_cast<GLenum>(_wrapT)) );
        }

        if( _texture->_wrapR != _wrapR )
        {
            _texture->_wrapR = _wrapR;
            if( target == GL_TEXTURE_CUBE_MAP ) // We don't want to run this on something that we know will fail
            GL_ASSERT( glTexParameteri(target, GL_TEXTURE_WRAP_R, static_cast<GLenum>(_wrapR)) );
        }
    }
}
