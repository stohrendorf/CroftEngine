#include "level/level.h"
#include "engine/laracontroller.h"

#include <boost/range/adaptors.hpp>
#include <boost/lexical_cast.hpp>

namespace
{
    void drawText(gameplay::Font* font, int x, int y, const std::string& txt, const glm::vec4& col = {1,1,1,1})
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
        drawText(font, 10, 80, boost::lexical_cast<std::string>(lvl->m_lara->getCurrentFrame().count()));
        drawText(font, 100, 80, toString(static_cast<loader::AnimationId>(lvl->m_lara->getCurrentAnimationId())));

        // triggers
        {
            int y = 100;
            for(const std::unique_ptr<engine::ItemController>& item : lvl->m_itemControllers | boost::adaptors::map_values)
            {
                if(!item->m_isActive)
                    continue;

                drawText(font, 10, y, item->getName());
                if(item->m_flags2_02_toggledOn)
                    drawText(font, 180, y, "toggled");
                if(item->m_flags2_04_ready)
                    drawText(font, 220, y, "ready");
                drawText(font, 260, y, boost::lexical_cast<std::string>(item->m_triggerTimeout.count()));
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
    gameplay::Game* game = new gameplay::Game();
    std::unique_ptr<gameplay::Platform> platform{gameplay::Platform::create(game)};
    game->run();
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

    const LevelInfo& lvlInfo = levels[1];

    auto lvl = level::Level::createLoader("data/tr1/data/" + lvlInfo.filename + ".PHD", level::Game::Unknown);

    BOOST_ASSERT(lvl != nullptr);
    lvl->load();

    platform->boot();
    lvl->toIrrlicht(game);

    // device->setWindowCaption("EdisonEngine");

    if(lvlInfo.track > 0)
        lvl->playCdTrack(lvlInfo.track);

    auto lastTime = game->getAbsoluteTime();
    while(platform->loop())
    {
        lvl->m_audioDev.update();
        lvl->m_inputHandler->update();

        auto deltaTime = game->getAbsoluteTime() - lastTime;
        if(deltaTime <= std::chrono::microseconds::zero())
        {
            continue;
        }

        lastTime = game->getAbsoluteTime();

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

        lvl->drawBars(game);

        //drawDebugInfo(game, lvl.get());

        // update information about current frame-rate
        //std::string str = "FPS: ";
        //str += boost::lexical_cast<std::string>(driver->getFPS());
        //str += " Tris: ";
        //str += boost::lexical_cast<std::string>(driver->getPrimitiveCountDrawn());
        //device->setWindowCaption(str.c_str());

        platform->frame();
    }

    //device->drop();

    return EXIT_SUCCESS;
}
