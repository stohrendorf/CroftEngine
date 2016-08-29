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
        , _format(UNKNOWN)
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


    Texture* Texture::create(Image* image, bool generateMipmaps)
    {
        GP_ASSERT( image );

        switch( image->getFormat() )
        {
            case Image::RGB:
                return create(Texture::RGB, image->getWidth(), image->getHeight(), image->getData(), generateMipmaps);
            case Image::RGBA:
                return create(Texture::RGBA, image->getWidth(), image->getHeight(), image->getData(), generateMipmaps);
            default:
                GP_ERROR("Unsupported image format (%d).", image->getFormat());
                return nullptr;
        }
    }


    Texture* Texture::create(Format format, unsigned int width, unsigned int height, const unsigned char* data, bool generateMipmaps, Texture::Type type)
    {
        GP_ASSERT( type == Texture::TEXTURE_2D || type == Texture::TEXTURE_CUBE );

        GLenum target = static_cast<GLenum>(type);

        // Create the texture.
        GLuint textureId;
        GL_ASSERT( glGenTextures(1, &textureId) );
        GL_ASSERT( glBindTexture(target, textureId) );
        GL_ASSERT( glPixelStorei(GL_UNPACK_ALIGNMENT, 1) );
#ifndef OPENGL_ES
        // glGenerateMipmap is new in OpenGL 3.0. For OpenGL 2.0 we must fallback to use glTexParameteri
        // with GL_GENERATE_MIPMAP prior to actual texture creation (glTexImage2D)
        if( generateMipmaps && !std::addressof(glGenerateMipmap) )
        GL_ASSERT( glTexParameteri(target, GL_GENERATE_MIPMAP, GL_TRUE) );
#endif

        // Load the texture
        if( type == Texture::TEXTURE_2D )
        {
            // Texture 2D
            GL_ASSERT( glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLenum>(format), width, height, 0, static_cast<GLenum>(format), GL_UNSIGNED_BYTE, data) );
        }
        else
        {
            // Get texture size
            unsigned int textureSize = width * height;
            switch( format )
            {
                case Texture::RGB:
                    textureSize *= 3;
                    break;
                case Texture::RGBA:
                    textureSize *= 4;
                    break;
                case Texture::ALPHA:
                    break;
                case Texture::UNKNOWN:
                    if( data )
                    {
                        glDeleteTextures(1, &textureId);
                        GP_ERROR("Failed to determine texture size because format is UNKNOWN.");
                        return nullptr;
                    }
                    break;
            }
            // Texture Cube
            for( unsigned int i = 0; i < 6; i++ )
            {
                const unsigned char* texturePtr = (data == nullptr) ? nullptr : &data[i * textureSize];
                GL_ASSERT( glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, static_cast<GLenum>(format), width, height, 0, static_cast<GLenum>(format), GL_UNSIGNED_BYTE, texturePtr) );
            }
        }

        // Set initial minification filter based on whether or not mipmaping was enabled.
        Filter minFilter = generateMipmaps ? NEAREST_MIPMAP_LINEAR : LINEAR;
        GL_ASSERT( glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter) );

        Texture* texture = new Texture();
        texture->_handle = textureId;
        texture->_format = format;
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


    Texture* Texture::create(TextureHandle handle, int width, int height, Format format)
    {
        GP_ASSERT( handle );

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
        texture->_format = format;
        texture->_width = width;
        texture->_height = height;

        return texture;
    }


    void Texture::setData(const unsigned char* data)
    {
        // Don't work with any compressed or cached textures
        GP_ASSERT( data );
        GP_ASSERT( (!_compressed) );
        GP_ASSERT( (!_cached) );

        GL_ASSERT( glBindTexture(static_cast<GLenum>(_type), _handle) );

        if( _type == Texture::TEXTURE_2D )
        {
            GL_ASSERT( glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _width, _height, static_cast<GLenum>(_format), GL_UNSIGNED_BYTE, data) );
        }
        else
        {
            // Get texture size
            unsigned int textureSize = _width * _height;
            switch( _format )
            {
                case Texture::RGB:
                    textureSize *= 3;
                    break;
                case Texture::RGBA:
                    textureSize *= 4;
                    break;
                case Texture::ALPHA:
                    break;
            }
            // Texture Cube
            for( unsigned int i = 0; i < 6; i++ )
            {
                GL_ASSERT( glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0, 0, _width, _height, static_cast<GLenum>(_format), GL_UNSIGNED_BYTE, &data[i * textureSize]) );
            }
        }

        if( _mipmapped )
        {
            generateMipmaps();
        }

        // Restore the texture id
        GL_ASSERT( glBindTexture(static_cast<GLenum>(__currentTextureType), __currentTextureId) );
    }


    // Computes the size of a PVRTC data chunk for a mipmap level of the given size.
    static unsigned int computePVRTCDataSize(int width, int height, int bpp)
    {
        int blockSize;
        int widthBlocks;
        int heightBlocks;

        if( bpp == 4 )
        {
            blockSize = 4 * 4; // Pixel by pixel block size for 4bpp
            widthBlocks = std::max(width >> 2, 2);
            heightBlocks = std::max(height >> 2, 2);
        }
        else
        {
            blockSize = 8 * 4; // Pixel by pixel block size for 2bpp
            widthBlocks = std::max(width >> 3, 2);
            heightBlocks = std::max(height >> 2, 2);
        }

        return widthBlocks * heightBlocks * ((blockSize * bpp) >> 3);
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


    Texture::Format Texture::getFormat() const
    {
        return _format;
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
            if( std::addressof(glGenerateMipmap) )
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


    Texture::Sampler::Sampler(Texture* texture)
        : _texture(texture)
        , _wrapS(Texture::REPEAT)
        , _wrapT(Texture::REPEAT)
        , _wrapR(Texture::REPEAT)
    {
        GP_ASSERT( texture );
        _minFilter = texture->_minFilter;
        _magFilter = texture->_magFilter;
    }


    Texture::Sampler::~Sampler()
    {
        SAFE_RELEASE(_texture);
    }


    Texture::Sampler* Texture::Sampler::create(Texture* texture)
    {
        GP_ASSERT( texture );
        GP_ASSERT( texture->_type == Texture::TEXTURE_2D || texture->_type == Texture::TEXTURE_CUBE );
        texture->addRef();
        return new Sampler(texture);
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


    Texture* Texture::Sampler::getTexture() const
    {
        return _texture;
    }


    void Texture::Sampler::bind()
    {
        GP_ASSERT( _texture );

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

#if defined(GL_TEXTURE_WRAP_R) // OpenGL ES 3.x and up, OpenGL 1.2 and up
        if( _texture->_wrapR != _wrapR )
        {
            _texture->_wrapR = _wrapR;
            if( target == GL_TEXTURE_CUBE_MAP ) // We don't want to run this on something that we know will fail
            GL_ASSERT( glTexParameteri(target, GL_TEXTURE_WRAP_R, static_cast<GLenum>(_wrapR)) );
        }
#endif
    }
}
