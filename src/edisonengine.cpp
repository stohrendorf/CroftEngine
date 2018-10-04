#include "level/level.h"
#include "engine/laranode.h"
#include "loader/trx/trx.h"

#include "gl/framebuffer.h"
#include "gl/font.h"

#include "render/label.h"

#include "util/cimgwrapper.h"

#include "audio/tracktype.h"

#include <boost/range/adaptors.hpp>
#include <boost/filesystem/operations.hpp>

#include <type_traits>
#include <utility>

namespace
{
void drawText(const gsl::not_null<std::shared_ptr<gameplay::gl::Font>>& font, const int x, const int y,
              const std::string& txt,
              const gameplay::gl::RGBA8& col = {255, 255, 255, 255})
{
    font->drawText( txt, x, y, col.r, col.g, col.b, col.a );
}

void drawDebugInfo(const gsl::not_null<std::shared_ptr<gameplay::gl::Font>>& font,
                   const gsl::not_null<std::shared_ptr<level::Level>>& lvl,
                   const int fps)
{
    drawText( font, font->getTarget()->getWidth() - 40, font->getTarget()->getHeight() - 20, std::to_string( fps ) );

    if( lvl->m_lara != nullptr )
    {
        // position/rotation
        drawText( font, 10, 40, lvl->m_lara->m_state.position.room->node->getId() );

        drawText( font, 300, 20,
                  std::to_string( std::lround( lvl->m_lara->m_state.rotation.Y.toDegrees() ) ) + " deg" );
        drawText( font, 300, 40, "x=" + std::to_string( lvl->m_lara->m_state.position.position.X ) );
        drawText( font, 300, 60, "y=" + std::to_string( lvl->m_lara->m_state.position.position.Y ) );
        drawText( font, 300, 80, "z=" + std::to_string( lvl->m_lara->m_state.position.position.Z ) );

        // physics
        drawText( font, 300, 100, "grav " + std::to_string( lvl->m_lara->m_state.fallspeed ) );
        drawText( font, 300, 120, "fwd  " + std::to_string( lvl->m_lara->m_state.speed ) );

        // animation
        drawText( font, 10, 60,
                  std::string( "current/anim    " ) + toString( lvl->m_lara->getCurrentAnimState() ) );
        drawText( font, 10, 100,
                  std::string( "target          " ) + toString( lvl->m_lara->getGoalAnimState() ) );
        drawText( font, 10, 120,
                  std::string( "frame           " ) + std::to_string( lvl->m_lara->m_state.frame_number ) );
    }

    // triggers
    {
        int y = 180;
        for( const auto& item : lvl->m_itemNodes | boost::adaptors::map_values )
        {
            if( !item->m_isActive )
                continue;

            drawText( font, 10, y, item->getNode()->getId() );
            switch( item->m_state.triggerState )
            {
                case engine::items::TriggerState::Inactive:
                    drawText( font, 180, y, "inactive" );
                    break;
                case engine::items::TriggerState::Active:
                    drawText( font, 180, y, "active" );
                    break;
                case engine::items::TriggerState::Deactivated:
                    drawText( font, 180, y, "deactivated" );
                    break;
                case engine::items::TriggerState::Invisible:
                    drawText( font, 180, y, "invisible" );
                    break;
            }
            drawText( font, 260, y, std::to_string( item->m_state.timer ) );
            y += 20;
        }
    }

    if( lvl->m_lara == nullptr )
        return;

#ifndef NDEBUG
    // collision
    drawText( font, 400, 20, boost::lexical_cast<std::string>( "AxisColl: " )
                             + std::to_string( lvl->m_lara->lastUsedCollisionInfo.collisionType ) );
    drawText( font, 400, 40, boost::lexical_cast<std::string>( "Current floor:   " )
                             + std::to_string( lvl->m_lara->lastUsedCollisionInfo.mid.floor.y ) );
    drawText( font, 400, 60, boost::lexical_cast<std::string>( "Current ceiling: " )
                             + std::to_string( lvl->m_lara->lastUsedCollisionInfo.mid.ceiling.y ) );
    drawText( font, 400, 80, boost::lexical_cast<std::string>( "Front floor:     " )
                             + std::to_string( lvl->m_lara->lastUsedCollisionInfo.front.floor.y ) );
    drawText( font, 400, 100, boost::lexical_cast<std::string>( "Front ceiling:   " )
                              + std::to_string( lvl->m_lara->lastUsedCollisionInfo.front.ceiling.y ) );
    drawText( font, 400, 120, boost::lexical_cast<std::string>( "Front/L floor:   " )
                              + std::to_string( lvl->m_lara->lastUsedCollisionInfo.frontLeft.floor.y ) );
    drawText( font, 400, 140, boost::lexical_cast<std::string>( "Front/L ceiling: " )
                              + std::to_string( lvl->m_lara->lastUsedCollisionInfo.frontLeft.ceiling.y ) );
    drawText( font, 400, 160, boost::lexical_cast<std::string>( "Front/R floor:   " )
                              + std::to_string( lvl->m_lara->lastUsedCollisionInfo.frontRight.floor.y ) );
    drawText( font, 400, 180, boost::lexical_cast<std::string>( "Front/R ceiling: " )
                              + std::to_string( lvl->m_lara->lastUsedCollisionInfo.frontRight.ceiling.y ) );
    drawText( font, 400, 200, boost::lexical_cast<std::string>( "Need bottom:     " )
                              + std::to_string( lvl->m_lara->lastUsedCollisionInfo.badPositiveDistance ) );
    drawText( font, 400, 220, boost::lexical_cast<std::string>( "Need top:        " )
                              + std::to_string( lvl->m_lara->lastUsedCollisionInfo.badNegativeDistance ) );
    drawText( font, 400, 240, boost::lexical_cast<std::string>( "Need ceiling:    " )
                              + std::to_string( lvl->m_lara->lastUsedCollisionInfo.badCeilingDistance ) );
#endif

    // weapons
    drawText( font, 400, 280, std::string( "L.aiming    " ) + (lvl->m_lara->leftArm.aiming ? "true" : "false") );
    drawText( font, 400, 300,
              std::string( "L.aim       X=" ) + std::to_string( lvl->m_lara->leftArm.aimRotation.X.toDegrees() )
              + ", Y=" + std::to_string( lvl->m_lara->leftArm.aimRotation.Y.toDegrees() ) );
    drawText( font, 400, 320, std::string( "R.aiming    " ) + (lvl->m_lara->rightArm.aiming ? "true" : "false") );
    drawText( font, 400, 340,
              std::string( "R.aim       X=" ) + std::to_string( lvl->m_lara->rightArm.aimRotation.X.toDegrees() )
              + ", Y=" + std::to_string( lvl->m_lara->rightArm.aimRotation.Y.toDegrees() ) );
}
}

class FullScreenFX
{
    std::shared_ptr<gameplay::gl::Texture> m_depthBuffer{nullptr};
    std::shared_ptr<gameplay::gl::Texture> m_colorBuffer{nullptr};
    const gsl::not_null<std::shared_ptr<gameplay::ShaderProgram>> m_shader;
    const gsl::not_null<std::shared_ptr<gameplay::Material>> m_material;

public:
    explicit FullScreenFX(const gameplay::Game& game,
                          gsl::not_null<std::shared_ptr<gameplay::ShaderProgram>> shader,
                          const GLint multisample = 0)
            : m_shader{std::move( shader )}
            , m_material{make_not_null_shared<gameplay::Material>( m_shader )}
            , m_fb{std::make_shared<gameplay::gl::FrameBuffer>()}
    {
        init( game, multisample );
    }

    void init(const gameplay::Game& game, const GLint multisample)
    {
        const auto vp = game.getViewport();

        m_colorBuffer = std::make_shared<gameplay::gl::Texture>(
                multisample > 0 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D );
        m_colorBuffer->image2D<gameplay::gl::RGBA8>(
                gsl::narrow<GLint>( vp.width ), gsl::narrow<GLint>( vp.height ), false, multisample );
        m_fb->attachTexture2D( GL_COLOR_ATTACHMENT0, *m_colorBuffer );

        m_depthBuffer = std::make_shared<gameplay::gl::Texture>(
                multisample > 0 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D );
        m_depthBuffer->depthImage2D( gsl::narrow<GLint>( vp.width ), gsl::narrow<GLint>( vp.height ), multisample );
        m_fb->attachTexture2D( GL_DEPTH_ATTACHMENT, *m_depthBuffer );

        BOOST_ASSERT( m_fb->isComplete() );

        m_mesh = gameplay::Mesh::createQuadFullscreen( gsl::narrow<float>( vp.width ), gsl::narrow<float>( vp.height ),
                                                       m_shader->getHandle() );
        m_material->getParameter( "u_depth" )->set( m_depthBuffer );
        m_material->getParameter( "u_projectionMatrix" )
                  ->set( glm::ortho( 0.0f, gsl::narrow<float>( vp.width ), gsl::narrow<float>( vp.height ), 0.0f, 0.0f,
                                     1.0f ) );
        m_material->getParameter( "u_projection" )
                  ->bind( game.getScene()->getActiveCamera().get(), &gameplay::Camera::getProjectionMatrix );
        m_material->getParameter( "u_texture" )->set( m_colorBuffer );

        m_mesh->getParts()[0]->setMaterial( m_material );

        m_model = std::make_shared<gameplay::Model>();
        m_model->addMesh( gsl::make_not_null( m_mesh ) );

        m_model->getRenderState().setDepthWrite( false );
        m_model->getRenderState().setDepthTest( false );
        m_model->getRenderState().setCullFace( false );

        m_colorBuffer->set( GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        m_colorBuffer->set( GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    }

    void bind() const
    {
        m_fb->bind();
    }

    void render(gameplay::RenderContext& context) const
    {
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        gameplay::gl::checkGlError();

        m_model->draw( context );
    }

    const gsl::not_null<std::shared_ptr<gameplay::Material>>& getMaterial() const
    {
        return m_material;
    }

private:
    std::shared_ptr<gameplay::gl::FrameBuffer> m_fb;
    std::shared_ptr<gameplay::Mesh> m_mesh;
    std::shared_ptr<gameplay::Model> m_model;
};


void update(const gsl::not_null<std::shared_ptr<level::Level>>& lvl, const bool godMode)
{
    for( const auto& item : lvl->m_itemNodes | boost::adaptors::map_values )
    {
        if( item.get() == lvl->m_lara ) // Lara is special and needs to be updated last
            continue;

        if( item->m_isActive )
            item->update();

        item->getNode()->setVisible( item->m_state.triggerState != engine::items::TriggerState::Invisible );
    }

    for( const auto& item : lvl->m_dynamicItems )
    {
        if( item->m_isActive )
            item->update();

        item->getNode()->setVisible( item->m_state.triggerState != engine::items::TriggerState::Invisible );
    }

    std::vector<gsl::not_null<std::shared_ptr<engine::Particle>>> particlesToKeep;
    for( const auto& particle : lvl->m_particles )
    {
        if( particle->update( *lvl ) )
        {
            particlesToKeep.emplace_back( particle );
            setParent( particle, particle->pos.room->node );
            particle->updateLight();
        }
        else
        {
            setParent( particle, nullptr );
        }
    }
    lvl->m_particles = std::move( particlesToKeep );

    if( godMode )
        lvl->m_lara->m_state.health = core::LaraHealth;

    if( lvl->m_lara != nullptr )
        lvl->m_lara->update();

    lvl->applyScheduledDeletions();
    lvl->animateUV();
}

sol::state createScriptEngine()
{
    sol::state engine;
    engine.open_libraries( sol::lib::base, sol::lib::math, sol::lib::package );
    engine["package"]["path"] = (boost::filesystem::path( "scripts" ) / "?.lua").string();
    engine["package"]["cpath"] = "";

    engine.set_usertype( core::Angle::userType() );
    engine.set_usertype( core::TRRotation::userType() );
    engine.set_usertype( core::TRVec::userType() );
    engine.set_usertype( engine::ai::CreatureInfo::userType() );
    engine.set_usertype( engine::items::ItemState::userType() );

    engine.new_enum( "ActivationState",
                     "INACTIVE", engine::items::TriggerState::Inactive,
                     "ACTIVE", engine::items::TriggerState::Active,
                     "DEACTIVATED", engine::items::TriggerState::Deactivated,
                     "INVISIBLE", engine::items::TriggerState::Invisible
    );

    engine.new_enum( "Mood",
                     "BORED", engine::ai::Mood::Bored,
                     "ATTACK", engine::ai::Mood::Attack,
                     "ESCAPE", engine::ai::Mood::Escape,
                     "STALK", engine::ai::Mood::Stalk
    );

    engine.new_enum( "TrackType",
                     "AMBIENT", audio::TrackType::Ambient,
                     "INTERCEPTION", audio::TrackType::Interception,
                     "AMBIENT_EFFECT", audio::TrackType::AmbientEffect,
                     "LARA_TALK", audio::TrackType::LaraTalk
    );

    {
        sol::table tbl = engine.create_table("TR1SoundId");
        for( const auto& entry : engine::EnumUtil<engine::TR1SoundId>::all() )
            tbl[entry.second] = static_cast<std::underlying_type_t<engine::TR1SoundId>>(entry.first);
    }

    return engine;
}

int main()
{
    auto game = std::make_unique<gameplay::Game>();
    game->initialize();
    game->getScene()->setActiveCamera(
            std::make_shared<gameplay::Camera>( glm::radians( 80.0f ), game->getAspectRatio(), 10.0f, 20480.0f ) );

    const util::CImgWrapper splashImage{"splash.png"};
    util::CImgWrapper splashImageScaled{splashImage};

    const auto scaleSplashImage = [&]() {
        splashImageScaled = splashImage;
        // scale splash image so that its aspect ratio is preserved, but the boundaries match
        const float splashScale = std::max(
                gsl::narrow<float>( game->getViewport().width ) / splashImageScaled.width(),
                gsl::narrow<float>( game->getViewport().height ) / splashImageScaled.height()
        );
        splashImageScaled.resize( splashImageScaled.width() * splashScale, splashImageScaled.height() * splashScale );
        // crop to boundaries
        const auto centerX = splashImageScaled.width() / 2;
        const auto centerY = splashImageScaled.height() / 2;
        splashImageScaled.crop(
                gsl::narrow<int>( centerX - game->getViewport().width / 2 ),
                gsl::narrow<int>( centerY - game->getViewport().height / 2 ),
                gsl::narrow<int>( centerX - game->getViewport().width / 2 + game->getViewport().width - 1 ),
                gsl::narrow<int>( centerY - game->getViewport().height / 2 + game->getViewport().height - 1 )
        );
        Expects( splashImageScaled.width() == game->getViewport().width );
        Expects( splashImageScaled.height() == game->getViewport().height );

        splashImageScaled.interleave();
    };

    scaleSplashImage();

    auto screenOverlay = make_not_null_shared<gameplay::ScreenOverlay>( game->getViewport() );

    auto abibasFont = make_not_null_shared<gameplay::gl::Font>( "abibas.ttf", 48 );
    abibasFont->setTarget( screenOverlay->getImage() );

    auto drawLoadingScreen = [&](const std::string& state) -> void {
        glfwPollEvents();
        if( game->updateWindowSize() )
        {
            game->getScene()->getActiveCamera()->setAspectRatio( game->getAspectRatio() );
            screenOverlay->init( game->getViewport() );
            abibasFont->setTarget( screenOverlay->getImage() );

            scaleSplashImage();
        }
        screenOverlay->getImage()->assign(
                reinterpret_cast<const gameplay::gl::RGBA8*>(splashImageScaled.data()),
                game->getViewport().width * game->getViewport().height
        );
        abibasFont->drawText( state, 40, gsl::narrow<int>( game->getViewport().height - 100 ), 255, 255, 255, 192 );

        gameplay::gl::FrameBuffer::unbindAll();

        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        gameplay::gl::checkGlError();

        game->clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, {0, 0, 0, 0}, 1 );
        gameplay::RenderContext context{};
        gameplay::Node dummyNode{""};
        context.setCurrentNode( &dummyNode );
        screenOverlay->draw( context );
        game->swapBuffers();
    };

    drawLoadingScreen( "Booting" );

    auto scriptEngine = createScriptEngine();

    try
    {
        scriptEngine.safe_script_file( "scripts/main.lua" );
    }
    catch( sol::error& e )
    {
        BOOST_LOG_TRIVIAL( fatal ) << "Failed to load main.lua: " << e.what();
        return EXIT_FAILURE;
    }

    const sol::optional<std::string> glidosPack = scriptEngine["getGlidosPack"]();

    std::unique_ptr<loader::trx::Glidos> glidos;
    if( glidosPack && boost::filesystem::is_directory( glidosPack.value() ) )
    {
        drawLoadingScreen( "Loading Glidos texture pack" );
        glidos = std::make_unique<loader::trx::Glidos>( glidosPack.value(), drawLoadingScreen );
    }

    sol::table levelInfo = scriptEngine["getLevelInfo"]();
    if( !levelInfo.get<std::string>( "video" ).empty() )
        BOOST_THROW_EXCEPTION( std::runtime_error( "Videos not yet supported" ) );

    const auto cutsceneName = levelInfo.get<std::string>( "cutscene" );

    const auto baseName = cutsceneName.empty() ? levelInfo.get<std::string>( "baseName" ) : cutsceneName;
    Expects( !baseName.empty() );
    sol::optional<uint32_t> trackToPlay = levelInfo["track"];
    const std::string levelName = levelInfo["name"];
    const bool useAlternativeLara = levelInfo.get_or( "useAlternativeLara", false );

    std::map<engine::TR1ItemId, size_t> initInv;

    if( sol::optional<sol::table> tbl = levelInfo["inventory"] )
    {
        for( const auto& kv : *tbl )
            initInv[engine::EnumUtil<engine::TR1ItemId>::fromString( kv.first.as<std::string>() )]
                    += kv.second.as<size_t>();
    }

    if( sol::optional<sol::table> tbl = scriptEngine["cheats"]["inventory"] )
    {
        for( const auto& kv : *tbl )
            initInv[engine::EnumUtil<engine::TR1ItemId>::fromString( kv.first.as<std::string>() )]
                    += kv.second.as<size_t>();
    }

    drawLoadingScreen( "Preparing to load " + baseName );

    auto lvl = gsl::make_not_null(
            level::Level::createLoader( "data/tr1/data/" + baseName + ".PHD", level::Game::Unknown,
                                        std::move( scriptEngine ) ) );

    drawLoadingScreen( "Loading " + baseName );

    lvl->loadFileData();

    for( size_t i = 0; i < lvl->m_textures.size(); ++i )
    {
        if( glidos != nullptr )
            drawLoadingScreen( "Upgrading texture " + std::to_string( i + 1 ) + " of "
                               + std::to_string( lvl->m_textures.size() ) );
        else
            drawLoadingScreen(
                    "Loading texture " + std::to_string( i + 1 ) + " of " + std::to_string( lvl->m_textures.size() ) );
        lvl->m_textures[i].toTexture( glidos.get(), drawLoadingScreen );
    }

    drawLoadingScreen( "Preparing the game" );

    lvl->setUpRendering( gsl::make_not_null( game.get() ) );

    if( useAlternativeLara )
    {
        lvl->useAlternativeLaraAppearance();
    }

    for( const auto& item : initInv )
        lvl->addInventoryItem( item.first, item.second );

    if( trackToPlay )
    {
        lvl->playStopCdTrack( trackToPlay.value(), false );
    }

    if( !cutsceneName.empty() )
    {
        lvl->m_cameraController
           ->setEyeRotation( 0_deg, core::Angle::fromDegrees( levelInfo.get<float>( "cameraRot" ) ) );
        auto pos = lvl->m_cameraController->getTRPosition().position;
        if( auto x = levelInfo["cameraPosX"] )
            pos.X = x;
        if( auto y = levelInfo["cameraPosY"] )
            pos.Y = y;
        if( auto z = levelInfo["cameraPosZ"] )
            pos.Z = z;

        lvl->m_cameraController->setPosition( pos );

        if( bool(levelInfo["flipRooms"]) )
            lvl->swapAllRooms();

        for( auto& room : lvl->m_rooms )
            room.node->setVisible( room.alternateRoom < 0 );

        if( bool(levelInfo["gunSwap"]) )
        {
            const auto& laraPistol = lvl->findAnimatedModelForType( engine::TR1ItemId::LaraPistolsAnim );
            Expects( laraPistol != nullptr );
            for( const auto& item : lvl->m_itemNodes | boost::adaptors::map_values )
            {
                if( item->m_state.type != engine::TR1ItemId::CutsceneActor1 )
                    continue;

                item->getNode()->getChild( 1 )->setDrawable( laraPistol->models[1].get() );
                item->getNode()->getChild( 4 )->setDrawable( laraPistol->models[4].get() );
            }
        }
    }

    FullScreenFX depthDarknessFx{*game,
                                 gsl::make_not_null(
                                         gameplay::ShaderProgram::createFromFile( "shaders/fx_darkness.vert",
                                                                                  "shaders/fx_darkness.frag",
                                                                                  {"LENS_DISTORTION"} ) )};
    depthDarknessFx.getMaterial()->getParameter( "aspect_ratio" )->bind(
            [&game](const gameplay::Node& /*node*/, gameplay::gl::Program::ActiveUniform& uniform) {
                uniform.set( game->getAspectRatio() );
            } );
    depthDarknessFx.getMaterial()->getParameter( "distortion_power" )->set( -1.0f );
    FullScreenFX depthDarknessWaterFx{*game,
                                      gsl::make_not_null(
                                              gameplay::ShaderProgram::createFromFile( "shaders/fx_darkness.vert",
                                                                                       "shaders/fx_darkness.frag",
                                                                                       {"WATER",
                                                                                        "LENS_DISTORTION"} ) )};
    depthDarknessWaterFx.getMaterial()->getParameter( "aspect_ratio" )->bind(
            [&game](const gameplay::Node& /*node*/, gameplay::gl::Program::ActiveUniform& uniform) {
                uniform.set( game->getAspectRatio() );
            } );
    depthDarknessWaterFx.getMaterial()->getParameter( "distortion_power" )->set( -2.0f );
    depthDarknessWaterFx.getMaterial()->getParameter( "u_time" )->bind(
            [&game](const gameplay::Node& /*node*/, gameplay::gl::Program::ActiveUniform& uniform) {
                const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>( game->getGameTime() );
                uniform.set( gsl::narrow_cast<float>( now.time_since_epoch().count() ) );
            }
    );

    static const auto frameDuration = std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::seconds( 1 ) )
                                      / core::FrameRate;

    bool showDebugInfo = false;
    bool showDebugInfoToggled = false;

    auto font = make_not_null_shared<gameplay::gl::Font>( "DroidSansMono.ttf", 12 );
    font->setTarget( screenOverlay->getImage() );

    auto nextFrameTime = std::chrono::high_resolution_clock::now() + frameDuration;
    while( !game->windowShouldClose() )
    {
        screenOverlay->getImage()->fill( {0, 0, 0, 0} );

        if( !levelName.empty() )
        {
            render::Label tmp{0, -50, levelName};
            tmp.alignX = render::Label::Alignment::Center;
            tmp.alignY = render::Label::Alignment::Bottom;
            tmp.outline = true;
            tmp.addBackground( 0, 0, 0, 0 );
            tmp.draw( *screenOverlay->getImage(), *lvl );
        }

        lvl->m_audioDev.update();
        lvl->m_inputHandler->update();

        if( lvl->m_inputHandler->getInputState().debug )
        {
            if( !showDebugInfoToggled )
            {
                showDebugInfoToggled = true;
                showDebugInfo = !showDebugInfo;
            }
        }
        else
        {
            showDebugInfoToggled = false;
        }

        {
            // frame rate throttling
            // TODO this assumes that the frame rate capacity (the processing power so to speak)
            // is faster than 30 FPS.
            std::this_thread::sleep_until( nextFrameTime );
            nextFrameTime += frameDuration;
        }

        update( lvl, bool( lvl->m_scriptEngine["cheats"]["godMode"] ) );

        if( game->updateWindowSize() )
        {
            game->getScene()->getActiveCamera()->setAspectRatio( game->getAspectRatio() );
            depthDarknessFx.init( *game, 0 );
            depthDarknessWaterFx.init( *game, 0 );
            screenOverlay->init( game->getViewport() );
            font->setTarget( screenOverlay->getImage() );
        }

        if( cutsceneName.empty() )
        {
            lvl->m_cameraController->update();
        }
        else
        {
            if( ++lvl->m_cameraController->m_cinematicFrame >= lvl->m_cinematicFrames.size() )
                break;

            lvl->m_cameraController
               ->updateCinematic( lvl->m_cinematicFrames[lvl->m_cameraController->m_cinematicFrame], false );
        }
        lvl->doGlobalEffect();

        lvl->m_audioDev.setListenerTransform( lvl->m_cameraController->getPosition(),
                                              lvl->m_cameraController->getFrontVector(),
                                              lvl->m_cameraController->getUpVector() );

        if( lvl->m_lara != nullptr )
            lvl->drawBars( gsl::make_not_null( game.get() ), screenOverlay->getImage() );

        if( lvl->m_cameraController->getCurrentRoom()->isWaterRoom() )
            depthDarknessWaterFx.bind();
        else
            depthDarknessFx.bind();
        game->render();

        gameplay::RenderContext context{};
        gameplay::Node dummyNode{""};
        context.setCurrentNode( &dummyNode );

        gameplay::gl::FrameBuffer::unbindAll();

        if( lvl->m_cameraController->getCurrentRoom()->isWaterRoom() )
            depthDarknessWaterFx.render( context );
        else
            depthDarknessFx.render( context );

        if( showDebugInfo )
        {
            drawDebugInfo( font, lvl, game->getFrameRate() );

            for( const std::shared_ptr<engine::items::ItemNode>& ctrl : lvl->m_itemNodes | boost::adaptors::map_values )
            {
                const auto vertex = glm::vec3{game->getScene()->getActiveCamera()->getViewMatrix()
                                              * glm::vec4( ctrl->getNode()->getTranslationWorld(), 1 )};

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

                if( std::abs( projVertex.x ) > 1 || std::abs( projVertex.y ) > 1 )
                    continue;

                projVertex.x = (projVertex.x / 2 + 0.5f) * game->getViewport().width;
                projVertex.y = (1 - (projVertex.y / 2 + 0.5f)) * game->getViewport().height;

                font->drawText( ctrl->getNode()->getId().c_str(), projVertex.x, projVertex.y,
                                gameplay::gl::RGBA8{255} );
            }
        }

        screenOverlay->draw( context );

        game->swapBuffers();

        if( lvl->m_inputHandler->getInputState().save )
        {
            scaleSplashImage();
            abibasFont->setTarget( screenOverlay->getImage() );
            drawLoadingScreen( "Saving..." );

            BOOST_LOG_TRIVIAL( info ) << "Save";
            std::ofstream file{"quicksave.yaml", std::ios::out | std::ios::trunc};
            Expects( file.is_open() );
            file << lvl->save();

            nextFrameTime = std::chrono::high_resolution_clock::now() + frameDuration;
        }
        else if( lvl->m_inputHandler->getInputState().load )
        {
            scaleSplashImage();
            abibasFont->setTarget( screenOverlay->getImage() );
            drawLoadingScreen( "Loading..." );

            BOOST_LOG_TRIVIAL( info ) << "Load";
            lvl->load( YAML::LoadFile( "quicksave.yaml" ) );

            nextFrameTime = std::chrono::high_resolution_clock::now() + frameDuration;
        }
    }

    levelInfo = sol::table();

    return EXIT_SUCCESS;
}
