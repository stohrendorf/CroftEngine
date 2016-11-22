#include "Base.h"

#include "ScreenOverlay.h"

#include "Game.h"
#include "Image.h"
#include "Material.h"
#include "MaterialParameter.h"
#include "RenderState.h"

#include <glm/gtc/matrix_transform.hpp>
#include <boost/log/trivial.hpp>


namespace gameplay
{
    namespace
    {
        std::shared_ptr<ShaderProgram> screenOverlayProgram = nullptr;
    }


    ScreenOverlay::ScreenOverlay(Game* game)
        : _game{game}
    {
        BOOST_ASSERT(game != nullptr);

        if( screenOverlayProgram == nullptr )
        {
            screenOverlayProgram = ShaderProgram::createFromFile("shaders/screenoverlay.vert", "shaders/screenoverlay.frag", {});
            if( screenOverlayProgram == nullptr )
            {
                BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create effect for screen overlay."));
            }
        }

        resize();
    }


    void ScreenOverlay::resize()
    {
        // Update the projection matrix for our batch to match the current viewport
        const Rectangle& vp = _game->getViewport();

        if(vp.isEmpty())
        {
            BOOST_THROW_EXCEPTION(std::runtime_error("Cannot create screen overlay because the viewport is empty"));
        }

        _image = std::make_shared<Image>(vp.width, vp.height);
        _image->fill({0,0,0,0});

        _texture = std::make_shared<Texture>(_image, false);

        _batch = std::make_shared<SpriteBatch>(_game, _texture, screenOverlayProgram);

        if( !vp.isEmpty() )
        {
            glm::mat4 projectionMatrix = glm::ortho(vp.x, vp.width, vp.height, vp.y, 0.0f, 1.0f);
            _batch->setProjectionMatrix(projectionMatrix);
        }

        _batch->getSampler()->setWrapMode(Texture::CLAMP, Texture::CLAMP);

        _batch->start();
        _batch->draw(0, 0, vp.width, vp.height, 0, 0, 1, 1, glm::vec4{1,1,1,1});
    }


    ScreenOverlay::~ScreenOverlay() = default;
}
