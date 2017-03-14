#pragma once

#include "Drawable.h"
#include "Game.h"
#include "Image.h"
#include "Mesh.h"

#include "gl/texture.h"

#include <memory>


namespace gameplay
{
    class ScreenOverlay : public Drawable
    {
    public:
        explicit ScreenOverlay(Game* game);
        ~ScreenOverlay();

        void resize();


        void draw(RenderContext& context) override;


        const std::shared_ptr<Image<gl::PixelRGBA_U8>>& getImage() const
        {
            return _image;
        }


    private:

        ScreenOverlay(const ScreenOverlay& copy) = delete;
        ScreenOverlay& operator=(const ScreenOverlay&) = delete;

        std::shared_ptr<Image<gl::PixelRGBA_U8>> _image{nullptr};
        std::shared_ptr<gl::Texture> _texture{nullptr};
        std::shared_ptr<Mesh> _mesh{nullptr};
        std::shared_ptr<Model> _model{nullptr};

        Game* _game;
    };
}
