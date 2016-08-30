#include "level/level.h"
#include "engine/laracontroller.h"

#include <boost/range/adaptors.hpp>
#include <boost/lexical_cast.hpp>

namespace
{
    void drawText(gameplay::Font* font, int x, int y, const std::string& txt, const gameplay::Vector4& col = gameplay::Vector4::one())
    {
        font->drawText(txt, x, y, col.x, col.y, col.z, col.w);
    }

    void drawDebugInfo(gsl::not_null<gameplay::Font*> font, gsl::not_null<level::Level*> lvl)
    {
        // position/rotation
        drawText(font, 10, 40, lvl->m_lara->getCurrentRoom()->node->getId());
        drawText(font, 100, 40, boost::lexical_cast<std::string>(std::lround(lvl->m_lara->getRotation().Y.toDegrees())));
        drawText(font, 140, 20, boost::lexical_cast<std::string>(std::lround(lvl->m_lara->getPosition().X)));
        drawText(font, 140, 30, boost::lexical_cast<std::string>(std::lround(lvl->m_lara->getPosition().Y)));
        drawText(font, 140, 40, boost::lexical_cast<std::string>(std::lround(lvl->m_lara->getPosition().Z)));

        // physics
        drawText(font, 180, 40, boost::lexical_cast<std::string>(std::lround(lvl->m_lara->getFallSpeed().getCurrentValue())));

        // animation
        drawText(font, 10, 60, loader::toString(lvl->m_lara->getCurrentAnimState()));
        drawText(font, 100, 60, loader::toString(lvl->m_lara->getTargetState()));
        drawText(font, 10, 80, boost::lexical_cast<std::string>(lvl->m_lara->getCurrentFrame()));
        drawText(font, 100, 80, toString(static_cast<loader::AnimationId>(lvl->m_lara->getCurrentAnimationId())));

        // triggers
        {
            int y = 100;
            for(const std::unique_ptr<engine::ItemController>& item : lvl->m_itemControllers | boost::adaptors::map_values)
            {
                if(!item->m_isActive)
                    continue;

                drawText(font, 10, y, item->getName().c_str());
                if(item->m_flags2_02_toggledOn)
                    drawText(font, 180, y, "toggled");
                if(item->m_flags2_04_ready)
                    drawText(font, 220, y, "ready");
                drawText(font, 260, y, boost::lexical_cast<std::string>(item->m_triggerTimeout));
                y += 20;
            }
        }

        // collision
        drawText(font, 200, 20,  boost::lexical_cast<std::string>("AxisColl: ") + boost::lexical_cast<std::string>(lvl->m_lara->lastUsedCollisionInfo.axisCollisions));
        drawText(font, 200, 40,  boost::lexical_cast<std::string>("Current floor:   ") + boost::lexical_cast<std::string>(lvl->m_lara->lastUsedCollisionInfo.current.floor.distance));
        drawText(font, 200, 60,  boost::lexical_cast<std::string>("Current ceiling: ") + boost::lexical_cast<std::string>(lvl->m_lara->lastUsedCollisionInfo.current.ceiling.distance));
        drawText(font, 200, 80,  boost::lexical_cast<std::string>("Front floor:     ") + boost::lexical_cast<std::string>(lvl->m_lara->lastUsedCollisionInfo.front.floor.distance));
        drawText(font, 200, 100, boost::lexical_cast<std::string>("Front ceiling:   ") + boost::lexical_cast<std::string>(lvl->m_lara->lastUsedCollisionInfo.front.ceiling.distance));
        drawText(font, 200, 120, boost::lexical_cast<std::string>("Front/L floor:   ") + boost::lexical_cast<std::string>(lvl->m_lara->lastUsedCollisionInfo.frontLeft.floor.distance));
        drawText(font, 200, 140, boost::lexical_cast<std::string>("Front/L ceiling: ") + boost::lexical_cast<std::string>(lvl->m_lara->lastUsedCollisionInfo.frontLeft.ceiling.distance));
        drawText(font, 200, 160, boost::lexical_cast<std::string>("Front/R floor:   ") + boost::lexical_cast<std::string>(lvl->m_lara->lastUsedCollisionInfo.frontRight.floor.distance));
        drawText(font, 200, 180, boost::lexical_cast<std::string>("Front/R ceiling: ") + boost::lexical_cast<std::string>(lvl->m_lara->lastUsedCollisionInfo.frontRight.ceiling.distance));
        drawText(font, 200, 200, boost::lexical_cast<std::string>("Need bottom:     ") + boost::lexical_cast<std::string>(lvl->m_lara->lastUsedCollisionInfo.neededFloorDistanceBottom));
        drawText(font, 200, 220, boost::lexical_cast<std::string>("Need top:        ") + boost::lexical_cast<std::string>(lvl->m_lara->lastUsedCollisionInfo.neededFloorDistanceTop));
        drawText(font, 200, 240, boost::lexical_cast<std::string>("Need ceiling:    ") + boost::lexical_cast<std::string>(lvl->m_lara->lastUsedCollisionInfo.neededCeilingDistance));
    }
}

int main()
{
    gameplay::Game* game = gameplay::Game::getInstance();
    std::unique_ptr<gameplay::Platform> platform{gameplay::Platform::create(game)};
#if 0
    irr::SIrrlichtCreationParameters driverParams;
    driverParams.AntiAlias = 255;
    driverParams.ZBufferBits = 32;
    driverParams.Stencilbuffer = true;
    driverParams.Vsync = false;
    driverParams.Bits = 24;
    driverParams.Fullscreen = false;
    driverParams.DriverType = driverType;
    driverParams.WindowSize = irr::core::dimension2d<irr::u32>(1024, 768);

    irr::IrrlichtDevice* device = irr::createDeviceEx( driverParams );
    if(!device)
        return EXIT_FAILURE;
#endif

    struct LevelInfo
    {
        std::string filename;
        std::string title;
        int track;
        int secrets;
    };

    LevelInfo levels[] = {
        {"GYM", "Lara's Home", 0, 0},
        {"LEVEL1", "Caves", 57, 3}, // 1
        {"LEVEL2", "City of Vilcabamba", 57, 3},
        {"LEVEL3A", "Lost Valley", 57, 5},
        {"LEVEL3B", "Tomb of Qualopec", 57, 3},
        {"LEVEL4", "St. Francis' Folly", 59, 4},
        {"LEVEL5", "Colosseum", 59, 3}, // 6
        {"LEVEL6", "Palace Midas", 59, 3},
        {"LEVEL7A", "The Cistern", 58, 3},
        {"LEVEL7B", "Tomb of Tihocan", 58, 2},
        {"LEVEL8A", "City of Khamoon", 59, 3},
        {"LEVEL8B", "Obelisk of Khamoon", 59, 3}, // 11
        {"LEVEL8C", "Sanctuary of the Scion", 59, 1},
        {"LEVEL10A", "Natla's Mines", 58, 3},
        {"LEVEL10B", "Atlantis", 60, 3},
        {"LEVEL10C", "The Great Pyramid", 60, 3} // 15
    };

    const LevelInfo& lvlInfo = levels[0];

    auto lvl = level::Level::createLoader("data/tr1/data/" + lvlInfo.filename + ".PHD", level::Game::Unknown);

    BOOST_ASSERT(lvl != nullptr);
    lvl->load(game);
    lvl->toIrrlicht(game);

    // device->setWindowCaption("EdisonEngine");

    auto lastTime = game->getAbsoluteTime();

    if(lvlInfo.track > 0)
        lvl->playCdTrack(lvlInfo.track);

    while(!game->loop())
    {
        lvl->m_audioDev.update();

        //if(lvl->m_activeCDTrack > 0 && lvl->m_cdStream == nullptr)
        //    lvl->playStream(lvl->m_activeCDTrack);

        if(!device->isWindowActive())
        {
            lastTime = timer->getTime();
            device->yield();
            continue;
        }

        lvl->m_inputHandler->update();

        auto deltaTime = timer->getTime() - lastTime;
        if(deltaTime <= 0)
        {
            device->yield();
            continue;
        }

        lastTime = timer->getTime();

        for(const std::unique_ptr<engine::ItemController>& ctrl : lvl->m_itemControllers | boost::adaptors::map_values)
        {
            if(ctrl.get() == lvl->m_lara) // Lara is special and needs to be updated last
                continue;

            ctrl->update(deltaTime);
        }

        lvl->m_lara->update(deltaTime);
        lvl->m_cameraController->update(deltaTime);

        lvl->m_audioDev.setListenerTransform(lvl->m_cameraController->getPosition(),
                                             lvl->m_cameraController->getFrontVector(),
                                             lvl->m_cameraController->getUpVector());

        device->getVideoDriver()->beginScene(true, true);
        //device->getSceneManager()->drawAll();
        lvl->drawBars(device->getVideoDriver());

        drawDebugInfo(device, lvl.get());

        device->getVideoDriver()->endScene();

        // update information about current frame-rate
        std::string str = "FPS: ";
        str += boost::lexical_cast<std::string>(driver->getFPS());
        str += " Tris: ";
        str += boost::lexical_cast<std::string>(driver->getPrimitiveCountDrawn());
        device->setWindowCaption(str.c_str());
    }

    //device->drop();

    return EXIT_SUCCESS;
}
