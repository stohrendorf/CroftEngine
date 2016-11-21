#pragma once

#include <glm/glm.hpp>


namespace gameplay
{
    class Image;


    /**
     * Defines a standard texture.
     */
    class Texture
    {
    public:
        class Sampler;
        friend class Sampler;

        explicit Texture(unsigned int width, unsigned int height, const std::vector<glm::vec4>& data, bool generateMipmaps);

        //! Creates a depth texture
        explicit Texture(unsigned int width, unsigned int height);

        explicit Texture(const std::shared_ptr<Image>& image, bool generateMipmaps = false);
        virtual ~Texture();


        /**
         * Defines the set of supported texture filters.
         */
        enum Filter
        {
            NEAREST = GL_NEAREST,
            LINEAR = GL_LINEAR,
            NEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST,
            LINEAR_MIPMAP_NEAREST = GL_LINEAR_MIPMAP_NEAREST,
            NEAREST_MIPMAP_LINEAR = GL_NEAREST_MIPMAP_LINEAR,
            LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR
        };


        /**
         * Defines the set of supported texture wrapping modes.
         */
        enum Wrap
        {
            REPEAT = GL_REPEAT,
            CLAMP = GL_CLAMP_TO_EDGE
        };


        /**
         * Defines a face of a Texture of Type: cube.
         */
        enum CubeFace
        {
            POSITIVE_X,
            NEGATIVE_X,
            POSITIVE_Y,
            NEGATIVE_Y,
            POSITIVE_Z,
            NEGATIVE_Z
        };


        /**
         * Defines a texture sampler.
         *
         * A texture sampler is basically an instance of a texture that can be
         * used to sample a texture from a material. In addition to the texture
         * itself, a sampler stores per-instance texture state information, such
         * as wrap and filter modes.
         */
        class Sampler
        {
            friend class Texture;

        public:
            explicit Sampler(const std::shared_ptr<Texture>& texture);

            /**
             * Destructor.
             */
            virtual ~Sampler();

            /**
             * Sets the wrap mode for this sampler.
             *
             * @param wrapS The horizontal wrap mode.
             * @param wrapT The vertical wrap mode.
             * @param wrapR The depth wrap mode.
             */
            void setWrapMode(Wrap wrapS, Wrap wrapT);

            /**
             * Sets the texture filter modes for this sampler.
             *
             * @param minificationFilter The texture minification filter.
             * @param magnificationFilter The texture magnification filter.
             */
            void setFilterMode(Filter minificationFilter, Filter magnificationFilter);

            /**
             * Gets the texture for this sampler.
             *
             * @return The texture for this sampler.
             */
            const std::shared_ptr<Texture>& getTexture() const;

            /**
             * Binds the texture of this sampler to the renderer and applies the sampler state.
             */
            void bind();

        private:

            Sampler& operator=(const Sampler&) = delete;

            std::shared_ptr<Texture> _texture;
            Wrap _wrapS = REPEAT;
            Wrap _wrapT = REPEAT;
            Filter _minFilter;
            Filter _magFilter;
        };


        /**
         * Set texture data to replace current texture image.
         *
         * @param data Raw texture data (expected to be tightly packed). If the type parameter is set
         *   to TEXTURE_CUBE, then data is expected to be each face stored back contiguously within the
         *   array.
         */
        void setData(const glm::vec4* data);

        /**
         * Gets the texture width.
         *
         * @return The texture width.
         */
        unsigned int getWidth() const;

        /**
         * Gets the texture height.
         *
         * @return The texture height.
         */
        unsigned int getHeight() const;

        /**
         * Generates a full mipmap chain for this texture if it isn't already mipmapped.
         */
        void generateMipmaps();

        /**
         * Determines if this texture currently contains a full mipmap chain.
         *
         * @return True if this texture is currently mipmapped, false otherwise.
         */
        bool isMipmapped() const;

        /**
         * Returns the texture handle.
         *
         * @return The texture handle.
         */
        const TextureHandle& getHandle() const;

    private:

        Texture(const Texture& copy) = delete;

        Texture& operator=(const Texture&) = delete;

        static int getMaskByteIndex(unsigned int mask);

        TextureHandle _handle{};
        unsigned int _width = 0;
        unsigned int _height = 0;
        bool _mipmapped = false;
        Wrap _wrapS = REPEAT;
        Wrap _wrapT = REPEAT;
        Filter _minFilter = NEAREST_MIPMAP_LINEAR;
        Filter _magFilter = LINEAR;
    };
}
