#pragma once

#include "SpriteBatch.h"
#include "Image.h"

#include <memory>


namespace gameplay
{
    class ScreenOverlay
    {
    public:
        ScreenOverlay();
        ~ScreenOverlay();

        void resize();


        void draw()
        {
            _texture->setData(_image->getData().data());
            _batch->finishAndDraw();
        }


        const std::shared_ptr<Image>& getImage() const
        {
            return _image;
        }


    private:

        ScreenOverlay(const ScreenOverlay& copy) = delete;
        ScreenOverlay& operator=(const ScreenOverlay&) = delete;

        std::shared_ptr<Image> _image{nullptr};
        std::shared_ptr<Texture> _texture{nullptr};
        std::shared_ptr<SpriteBatch> _batch{nullptr};
    };
}
