#include "level/level.h"
#include "engine/laranode.h"
#include "loader/trx/trx.h"

#include "LuaState.h"

#include "gl/framebuffer.h"
#include "gl/font.h"

#include <boost/range/adaptors.hpp>
#include <boost/filesystem/operations.hpp>

namespace
{
    void drawText(const std::unique_ptr<gameplay::gl::Font>& font, int x, int y, const std::string& txt, const gameplay::gl::RGBA8& col = {255,255,255,255})
    {
        font->drawText(txt, x, y, col.r, col.g, col.b, col.a);
    }


    void drawDebugInfo(const std::unique_ptr<gameplay::gl::Font>& font, gsl::not_null<level::Level*> lvl, int fps)
    {
        drawText(font, font->getTarget()->getWidth() - 40, font->getTarget()->getHeight() - 20, std::to_string(fps));

        // position/rotation
        drawText(font, 10, 40, lvl->m_lara->getCurrentRoom()->node->getId());

        drawText(font, 300, 20, std::to_string(std::lround(lvl->m_lara->getRotation().Y.toDegrees())) + " deg");
        drawText(font, 300, 40, "x=" + std::to_string(lvl->m_lara->getPosition().X));
        drawText(font, 300, 60, "y=" + std::to_string(lvl->m_lara->getPosition().Y));
        drawText(font, 300, 80, "z=" + std::to_string(lvl->m_lara->getPosition().Z));

        // physics
        drawText(font, 300, 100, "grav " + std::to_string(lvl->m_lara->getFallSpeed()));
        drawText(font, 300, 120, "fwd  " + std::to_string(lvl->m_lara->getHorizontalSpeed()));

        // animation
        drawText(font, 10, 60, std::string("current/anim    ") + loader::toString(lvl->m_lara->getCurrentAnimState()));
        drawText(font, 10, 100, std::string("target          ") + loader::toString(lvl->m_lara->getTargetState()));
        drawText(font, 10, 120, std::string("frame           ") + std::to_string(lvl->m_lara->getNode()->getCurrentFrame()));
        drawText(font, 10, 140, std::string("anim            ") + toString(static_cast<loader::AnimationId>(lvl->m_lara->getNode()->getAnimId())));

        // triggers
        {
            int y = 180;
            for( const std::shared_ptr<engine::items::ItemNode>& item : lvl->m_itemNodes | boost::adaptors::map_values )
            {
                if( !item->m_isActive )
                    continue;

                drawText(font, 10, y, item->getNode()->getId());
                switch(item->m_triggerState)
                {
                    case engine::items::TriggerState::Disabled:
                        drawText(font, 180, y, "disabled");
                        break;
                    case engine::items::TriggerState::Enabled:
                        drawText(font, 180, y, "enabled");
                        break;
                    case engine::items::TriggerState::Activated:
                        drawText(font, 180, y, "activated");
                        break;
                    case engine::items::TriggerState::Locked:
                        drawText(font, 180, y, "locked");
                        break;
                }
                drawText(font, 260, y, std::to_string(item->m_activationState.getTimeout()));
                y += 20;
            }
        }

#ifndef NDEBUG
        // collision
        drawText(font, 400, 20, boost::lexical_cast<std::string>("AxisColl: ") + std::to_string(lvl->m_lara->lastUsedCollisionInfo.collisionType));
        drawText(font, 400, 40, boost::lexical_cast<std::string>("Current floor:   ") + std::to_string(lvl->m_lara->lastUsedCollisionInfo.mid.floor.distance));
        drawText(font, 400, 60, boost::lexical_cast<std::string>("Current ceiling: ") + std::to_string(lvl->m_lara->lastUsedCollisionInfo.mid.ceiling.distance));
        drawText(font, 400, 80, boost::lexical_cast<std::string>("Front floor:     ") + std::to_string(lvl->m_lara->lastUsedCollisionInfo.front.floor.distance));
        drawText(font, 400, 100, boost::lexical_cast<std::string>("Front ceiling:   ") + std::to_string(lvl->m_lara->lastUsedCollisionInfo.front.ceiling.distance));
        drawText(font, 400, 120, boost::lexical_cast<std::string>("Front/L floor:   ") + std::to_string(lvl->m_lara->lastUsedCollisionInfo.frontLeft.floor.distance));
        drawText(font, 400, 140, boost::lexical_cast<std::string>("Front/L ceiling: ") + std::to_string(lvl->m_lara->lastUsedCollisionInfo.frontLeft.ceiling.distance));
        drawText(font, 400, 160, boost::lexical_cast<std::string>("Front/R floor:   ") + std::to_string(lvl->m_lara->lastUsedCollisionInfo.frontRight.floor.distance));
        drawText(font, 400, 180, boost::lexical_cast<std::string>("Front/R ceiling: ") + std::to_string(lvl->m_lara->lastUsedCollisionInfo.frontRight.ceiling.distance));
        drawText(font, 400, 200, boost::lexical_cast<std::string>("Need bottom:     ") + std::to_string(lvl->m_lara->lastUsedCollisionInfo.badPositiveDistance));
        drawText(font, 400, 220, boost::lexical_cast<std::string>("Need top:        ") + std::to_string(lvl->m_lara->lastUsedCollisionInfo.badNegativeDistance));
        drawText(font, 400, 240, boost::lexical_cast<std::string>("Need ceiling:    ") + std::to_string(lvl->m_lara->lastUsedCollisionInfo.badCeilingDistance));
#endif
    }
}


class FullScreenFX
{
    std::shared_ptr<gameplay::gl::Texture> m_depthBuffer{nullptr};
    std::shared_ptr<gameplay::gl::Texture> m_colorBuffer{nullptr};

public:
    explicit FullScreenFX(const gsl::not_null<gameplay::Game*>& game, const gsl::not_null<std::shared_ptr<gameplay::ShaderProgram>>& shader, GLint multisample = 0)
        : m_fb{std::make_shared<gameplay::gl::FrameBuffer>()}
    {
        auto vp = game->getViewport();

        m_colorBuffer = std::make_shared<gameplay::gl::Texture>(multisample > 0 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D);
        m_colorBuffer->image2D<gameplay::gl::RGBA8>(vp.width, vp.height, false, multisample);
        m_fb->attachTexture2D(GL_COLOR_ATTACHMENT0, *m_colorBuffer);

        m_depthBuffer = std::make_shared<gameplay::gl::Texture>(multisample > 0 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D);
        m_depthBuffer->depthImage2D(vp.width, vp.height, multisample);
        m_fb->attachTexture2D(GL_DEPTH_ATTACHMENT, *m_depthBuffer);

        BOOST_ASSERT(m_fb->isComplete());

        m_mesh = gameplay::Mesh::createQuadFullscreen(vp.width, vp.height, shader->getHandle());
        auto part = m_mesh->getPart(0);
        part->setMaterial(std::make_shared<gameplay::Material>(shader));

        if(auto tmp = part->getMaterial()->getParameter("u_depth"))
            tmp->set(m_depthBuffer);
        part->getMaterial()->getParameter("u_projectionMatrix")->set(glm::ortho(vp.x, vp.width, vp.height, vp.y, 0.0f, 1.0f));
        part->getMaterial()->getParameter("u_projection")->bind(game->getScene()->getActiveCamera().get(), &gameplay::Camera::getProjectionMatrix);
        if(auto tmp = part->getMaterial()->getParameter("u_texture"))
            tmp->set(m_colorBuffer);

        m_model = std::make_shared<gameplay::Model>();
        m_model->addMesh(m_mesh);

        m_colorBuffer->set(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        m_colorBuffer->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }


    void bind()
    {
        m_fb->bind();
    }


    void render(gameplay::RenderContext& context) const
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        gameplay::gl::checkGlError();

        m_model->draw(context);
    }


    const std::shared_ptr<gameplay::Material>& getMaterial() const
    {
        return m_mesh->getPart(0)->getMaterial();
    }

private:
    std::shared_ptr<gameplay::gl::FrameBuffer> m_fb;
    std::shared_ptr<gameplay::Mesh> m_mesh;
    std::shared_ptr<gameplay::Model> m_model;
};


void update(const std::unique_ptr<level::Level>& lvl, bool godMode)
{
    for( const std::shared_ptr<engine::items::ItemNode>& ctrl : lvl->m_itemNodes | boost::adaptors::map_values )
    {
        if( ctrl.get() == lvl->m_lara ) // Lara is special and needs to be updated last
            continue;

        ctrl->update();
    }

    for( engine::items::ItemNode* ctrl : lvl->m_dynamicItems )
    {
        ctrl->update();
    }

    if (godMode)
        lvl->m_lara->setHealth(core::LaraHealth);

    lvl->m_lara->update();

    lvl->applyScheduledDeletions();
}


int main()
{
    auto* game = new gameplay::Game();
    game->run();

    lua::State mainScript;
    mainScript.doFile("scripts/main.lua");
    lua::Value levelInfo = mainScript["getLevelInfo"].call();

    auto lvl = level::Level::createLoader("data/tr1/data/" + levelInfo["baseName"].toString() + ".PHD", level::Game::Unknown);

    BOOST_ASSERT(lvl != nullptr);
    lvl->loadFileData();

    const auto glidosPack = mainScript["getGlidosPack"].call();

    std::unique_ptr<loader::trx::Glidos> glidos;
    if(!glidosPack.isNil() && boost::filesystem::is_regular_file(glidosPack.toString()))
    {
        glidos = std::make_unique<loader::trx::Glidos>(glidosPack.toString());
        glidos->dump();
    }

    lvl->setUpRendering(game, "assets/tr1", levelInfo["baseName"].toString(), glidos);

    if( !levelInfo["swapRooms"].isNil() && levelInfo["swapRooms"].toBool() )
    {
        lvl->useAlternativeLaraAppearance();
    }

    // device->setWindowCaption("EdisonEngine");

    if( !levelInfo["track"].isNil() )
    {
        lvl->playCdTrack(levelInfo["track"].toUInt());
    }

    auto screenOverlay = std::make_unique<gameplay::ScreenOverlay>(game->getViewport());
    auto font = std::make_unique<gameplay::gl::Font>("DroidSansMono.ttf", 12);
    font->setTarget(screenOverlay->getImage());

    FullScreenFX depthDarknessFx{game, gameplay::ShaderProgram::createFromFile("shaders/fx_darkness.vert", "shaders/fx_darkness.frag", {"LENS_DISTORTION"}), gsl::narrow<GLint>(game->getMultiSampling())};
    depthDarknessFx.getMaterial()->getParameter("aspect_ratio")->set(game->getAspectRatio());
    depthDarknessFx.getMaterial()->getParameter("distortion_power")->set(-1.0f);
    FullScreenFX depthDarknessWaterFx{game, gameplay::ShaderProgram::createFromFile("shaders/fx_darkness.vert", "shaders/fx_darkness.frag", {"WATER", "LENS_DISTORTION"}), gsl::narrow<GLint>(game->getMultiSampling())};
    depthDarknessWaterFx.getMaterial()->getParameter("aspect_ratio")->set(game->getAspectRatio());
    depthDarknessWaterFx.getMaterial()->getParameter("distortion_power")->set(-2.0f);
    depthDarknessWaterFx.getMaterial()->getParameter("u_time")->bind(
                            [game](const gameplay::Node& /*node*/, gameplay::gl::Program::ActiveUniform& uniform)
                            {
                                const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(game->getGameTime());
                                uniform.set(gsl::narrow_cast<float>(now.time_since_epoch().count()));
                            }
                        );

    static const auto frameTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::seconds(1)) / core::FrameRate;

    bool showDebugInfo = false;
    bool showDebugInfoToggled = false;

    auto lastTime = game->getGameTime();
    glEnable(GL_FRAMEBUFFER_SRGB);
    gameplay::gl::checkGlError();
    while( game->loop() )
    {
        screenOverlay->getImage()->fill({0,0,0,0});

        lvl->m_audioDev.update();
        lvl->m_inputHandler->update();

        if(lvl->m_inputHandler->getInputState().debug)
        {
            if(!showDebugInfoToggled)
            {
                showDebugInfoToggled = true;
                showDebugInfo = !showDebugInfo;
            }
        }
        else
        {
            showDebugInfoToggled = false;
        }

        auto deltaTime = std::chrono::duration_cast<std::chrono::microseconds>( game->getGameTime() - lastTime );
        if( deltaTime < frameTime )
        {
            std::this_thread::sleep_for(frameTime - deltaTime);
        }

        lastTime = game->getGameTime();

        update(lvl, mainScript["cheats"]["godMode"].toBool());

        lvl->m_cameraController->update();

        lvl->m_audioDev.setListenerTransform(lvl->m_cameraController->getPosition(),
                                             lvl->m_cameraController->getFrontVector(),
                                             lvl->m_cameraController->getUpVector());

        lvl->drawBars(game, screenOverlay->getImage());

#define WITH_POSTFX

#ifdef WITH_POSTFX
        if(lvl->m_cameraController->getCurrentRoom()->isWaterRoom())
            depthDarknessWaterFx.bind();
        else
            depthDarknessFx.bind();
#else
        gameplay::gl::FrameBuffer::unbindAll();
#endif
        game->frame();

        gameplay::RenderContext context{false};
        gameplay::Node dummyNode{""};
        context.setCurrentNode(&dummyNode);

#ifdef WITH_POSTFX
        gameplay::gl::FrameBuffer::unbindAll();

        if(lvl->m_cameraController->getCurrentRoom()->isWaterRoom())
            depthDarknessWaterFx.render(context);
        else
            depthDarknessFx.render(context);
#endif

        if(showDebugInfo)
            drawDebugInfo(font, lvl.get(), game->getFrameRate());

        for( const std::shared_ptr<engine::items::ItemNode>& ctrl : lvl->m_itemNodes | boost::adaptors::map_values )
        {
            auto vertex = glm::vec3(game->getScene()->getActiveCamera()->getViewMatrix() * glm::vec4(ctrl->getNode()->getTranslationWorld(), 1));

            if( vertex.z > -game->getScene()->getActiveCamera()->getNearPlane() )
            {
                continue;
            }
            else if( vertex.z < -game->getScene()->getActiveCamera()->getFarPlane() )
            {
                continue;
            }

            glm::vec4 projVertex{vertex, 1};
            projVertex = game->getScene()->getActiveCamera()->getProjectionMatrix() * projVertex;
            projVertex /= projVertex.w;

            if(std::abs(projVertex.x) > 1 || std::abs(projVertex.y) > 1)
                continue;

            projVertex.x = (projVertex.x / 2 + 0.5f) * game->getViewport().width;
            projVertex.y = (1 - (projVertex.y / 2 + 0.5f)) * game->getViewport().height;

            if (showDebugInfo)
                font->drawText(ctrl->getNode()->getId().c_str(), projVertex.x, projVertex.y, gameplay::gl::RGBA8{255});
        }

        screenOverlay->draw(context);

        game->swapBuffers();
    }

    //device->drop();

    return EXIT_SUCCESS;
}
