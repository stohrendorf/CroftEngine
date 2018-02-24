#include "level/level.h"
#include "engine/laranode.h"
#include "loader/trx/trx.h"

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
        drawText(font, 10, 40, lvl->m_lara->m_state.position.room->node->getId());

        drawText(font, 300, 20, std::to_string(std::lround(lvl->m_lara->m_state.rotation.Y.toDegrees())) + " deg");
        drawText(font, 300, 40, "x=" + std::to_string(lvl->m_lara->m_state.position.position.X));
        drawText(font, 300, 60, "y=" + std::to_string(lvl->m_lara->m_state.position.position.Y));
        drawText(font, 300, 80, "z=" + std::to_string(lvl->m_lara->m_state.position.position.Z));

        // physics
        drawText(font, 300, 100, "grav " + std::to_string(lvl->m_lara->m_state.fallspeed));
        drawText(font, 300, 120, "fwd  " + std::to_string(lvl->m_lara->m_state.speed));

        // animation
        drawText(font, 10, 60, std::string("current/anim    ") + loader::toString(lvl->m_lara->getCurrentAnimState()));
        drawText(font, 10, 100, std::string("target          ") + loader::toString(lvl->m_lara->getTargetState()));
        drawText(font, 10, 120, std::string("frame           ") + std::to_string(lvl->m_lara->m_state.frame_number));
        drawText(font, 10, 140, std::string("anim            ") + toString(static_cast<loader::AnimationId>(lvl->m_lara->m_state.anim_number)));

        // triggers
        {
            int y = 180;
            for( const std::shared_ptr<engine::items::ItemNode>& item : lvl->m_itemNodes | boost::adaptors::map_values )
            {
                if( !item->m_isActive )
                    continue;

                drawText(font, 10, y, item->getNode()->getId());
                switch(item->m_state.triggerState)
                {
                    case engine::items::TriggerState::Inactive:
                        drawText(font, 180, y, "inactive");
                        break;
                    case engine::items::TriggerState::Active:
                        drawText(font, 180, y, "active");
                        break;
                    case engine::items::TriggerState::Deactivated:
                        drawText(font, 180, y, "deactivated");
                        break;
                    case engine::items::TriggerState::Locked:
                        drawText(font, 180, y, "locked");
                        break;
                }
                drawText(font, 260, y, std::to_string(item->m_state.timer));
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

        // weapons
        drawText(font, 400, 280, std::string("L.aiming    ") + (lvl->m_lara->leftArm.aiming?"true":"false"));
        drawText(font, 400, 300, std::string("L.aim       X=") + std::to_string(lvl->m_lara->leftArm.aimRotation.X.toDegrees()) + ", Y=" + std::to_string(lvl->m_lara->leftArm.aimRotation.Y.toDegrees()));
        drawText(font, 400, 320, std::string("R.aiming    ") + (lvl->m_lara->rightArm.aiming?"true":"false"));
        drawText(font, 400, 340, std::string("R.aim       X=") + std::to_string(lvl->m_lara->rightArm.aimRotation.X.toDegrees()) + ", Y=" + std::to_string(lvl->m_lara->rightArm.aimRotation.Y.toDegrees()));
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

        if(ctrl->m_isActive)
            ctrl->update();
    }

    for( const std::shared_ptr<engine::items::ItemNode>& ctrl : lvl->m_dynamicItems )
    {
        if (ctrl->m_isActive)
            ctrl->update();
    }

    if (godMode)
        lvl->m_lara->m_state.health = core::LaraHealth;

    lvl->m_lara->update();

    lvl->applyScheduledDeletions();
}


sol::state createScriptEngine()
{
    sol::state engine;
    engine.open_libraries(sol::lib::base, sol::lib::math, sol::lib::package);
    engine["package"]["path"] = (boost::filesystem::path("scripts") / "?.lua").string();
    engine["package"]["cpath"] = "";

    engine.set_usertype(core::Angle::userType());
    engine.set_usertype(core::TRRotation::userType());
    engine.set_usertype(core::TRCoordinates::userType());
    engine.set_usertype(engine::ai::CreatureInfo::userType());
    engine.set_usertype(engine::items::ItemState::userType());

    engine["ActivationState"] = engine.create_table_with(
        "DISABLED", engine::items::TriggerState::Inactive,
        "ENABLED", engine::items::TriggerState::Active,
        "ACTIVATED", engine::items::TriggerState::Deactivated,
        "LOCKED", engine::items::TriggerState::Locked
    );

    engine["Mood"] = engine.create_table_with(
        "BORED", engine::ai::Mood::Bored,
        "ATTACK", engine::ai::Mood::Attack,
        "ESCAPE", engine::ai::Mood::Escape,
        "STALK", engine::ai::Mood::Stalk
    );

    return engine;
}


int main()
{
    auto* game = new gameplay::Game();
    game->run();

    auto scriptEngine = createScriptEngine();

    try
    {
        scriptEngine.safe_script_file("scripts/main.lua");
    }
    catch(sol::error& e)
    {
        BOOST_LOG_TRIVIAL(fatal) << "Failed to load main.lua: " << e.what();
        return EXIT_FAILURE;
    }

    const sol::optional<std::string> glidosPack = scriptEngine["getGlidosPack"]();

    std::unique_ptr<loader::trx::Glidos> glidos;
    if(glidosPack && boost::filesystem::is_regular_file(glidosPack.value()))
    {
        glidos = std::make_unique<loader::trx::Glidos>(glidosPack.value());
#ifndef NDEBUG
        glidos->dump();
#endif
    }

    sol::table levelInfo = scriptEngine["getLevelInfo"]();
    const auto baseName = levelInfo.get<std::string>("baseName");
    sol::optional<uint32_t> trackToPlay = levelInfo["track"];
    const bool useAlternativeLara = levelInfo.get_or("useAlternativeLara", false);
    levelInfo = sol::table(); // do not keep a reference to the engine

    auto lvl = level::Level::createLoader("data/tr1/data/" + baseName + ".PHD", level::Game::Unknown, std::move(scriptEngine));

    BOOST_ASSERT(lvl != nullptr);
    lvl->loadFileData();

    lvl->setUpRendering(game, "assets/tr1", baseName, glidos);

    if(useAlternativeLara)
    {
        lvl->useAlternativeLaraAppearance();
    }

    // device->setWindowCaption("EdisonEngine");

    if( trackToPlay )
    {
        lvl->playCdTrack(trackToPlay.value());
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

        update(lvl, bool(lvl->m_scriptEngine["cheats"]["godMode"]));

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
