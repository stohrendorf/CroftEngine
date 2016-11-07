#include "Base.h"
#include "Image.h"
#include "Texture.h"
#include "FileSystem.h"


namespace gameplay
{
    static TextureHandle __currentTextureId = 0;


    Texture::Texture(unsigned int width, unsigned int height, const std::vector<glm::vec4>& data, bool generateMipmaps)
    {
        // Create the texture.
        TextureHandle textureId;
        GL_ASSERT(glGenTextures(1, &textureId));
        GL_ASSERT(glBindTexture(GL_TEXTURE_2D, textureId));
        GL_ASSERT(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
        if( generateMipmaps && !std::addressof(glGenerateMipmap) )
        GL_ASSERT(glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE));

        // Texture 2D
        GL_ASSERT(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, data.empty() ? nullptr : data.data()));

        // Set initial minification filter based on whether or not mipmaping was enabled.
        Filter minFilter = generateMipmaps ? NEAREST_MIPMAP_LINEAR : LINEAR;
        GL_ASSERT(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter));

        _handle = textureId;
        _width = width;
        _height = height;
        _minFilter = minFilter;
        if( generateMipmaps )
        {
            this->generateMipmaps();
        }

        // Restore the texture id
        GL_ASSERT(glBindTexture(GL_TEXTURE_2D, __currentTextureId));
    }


    Texture::~Texture()
    {
        if( _handle )
        {
            GL_ASSERT( glDeleteTextures(1, &_handle) );
            _handle = 0;
        }
    }


    Texture::Texture(const std::shared_ptr<Image>& image, bool generateMipmaps)
        : Texture{image->getWidth(), image->getHeight(), image->getData(), generateMipmaps}
    {
        BOOST_ASSERT( image );
    }


    void Texture::setData(const glm::vec4* data)
    {
        // Don't work with any compressed or cached textures
        BOOST_ASSERT( data );

        GL_ASSERT( glBindTexture(GL_TEXTURE_2D, _handle) );

        GL_ASSERT( glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _width, _height, GL_RGBA, GL_FLOAT, data) );

        if( _mipmapped )
        {
            generateMipmaps();
        }

        // Restore the texture id
        GL_ASSERT( glBindTexture(GL_TEXTURE_2D, __currentTextureId) );
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
            GL_ASSERT( glBindTexture(GL_TEXTURE_2D, _handle) );
            GL_ASSERT( glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST) );
            if( std::addressof(glGenerateMipmap) != nullptr )
            GL_ASSERT( glGenerateMipmap(GL_TEXTURE_2D) );

            _mipmapped = true;

            // Restore the texture id
            GL_ASSERT( glBindTexture(GL_TEXTURE_2D, __currentTextureId) );
        }
    }


    bool Texture::isMipmapped() const
    {
        return _mipmapped;
    }


    Texture::Sampler::Sampler(const std::shared_ptr<Texture>& texture)
        : _texture(texture)
    {
        BOOST_ASSERT( texture );
        _minFilter = texture->_minFilter;
        _magFilter = texture->_magFilter;
    }


    Texture::Sampler::~Sampler() = default;


    void Texture::Sampler::setWrapMode(Wrap wrapS, Wrap wrapT)
    {
        _wrapS = wrapS;
        _wrapT = wrapT;
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

        if( __currentTextureId != _texture->_handle )
        {
            GL_ASSERT( glBindTexture(GL_TEXTURE_2D, _texture->_handle) );
            __currentTextureId = _texture->_handle;
        }

        if( _texture->_minFilter != _minFilter )
        {
            _texture->_minFilter = _minFilter;
            GL_ASSERT( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLenum>(_minFilter)) );
        }

        if( _texture->_magFilter != _magFilter )
        {
            _texture->_magFilter = _magFilter;
            GL_ASSERT( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLenum>(_magFilter)) );
        }

        if( _texture->_wrapS != _wrapS )
        {
            _texture->_wrapS = _wrapS;
            GL_ASSERT( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLenum>(_wrapS)) );
        }

        if( _texture->_wrapT != _wrapT )
        {
            _texture->_wrapT = _wrapT;
            GL_ASSERT( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLenum>(_wrapT)) );
        }
    }
}
