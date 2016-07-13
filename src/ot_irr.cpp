#include "level/level.h"
#include <EffectHandler.h>
#include "engine/laracontroller.h"

#include <boost/range/adaptors.hpp>

namespace
{
    void drawText(irr::gui::IGUIEnvironment* env, int x, int y, const irr::core::stringw& txt, const irr::video::SColor& col = irr::video::SColor(255, 255, 255, 255))
    {
        auto fnt = env->getBuiltInFont();
        irr::core::recti r;
        r.UpperLeftCorner.X = x;
        r.UpperLeftCorner.Y = y;
        r.LowerRightCorner = r.UpperLeftCorner;
        const auto dim = fnt->getDimension(txt.c_str());
        r.LowerRightCorner.X += dim.Width;
        r.LowerRightCorner.Y += dim.Height;
        fnt->draw(txt, r, col);
    }
}

int main()
{
#if defined(_MSC_VER)
    const irr::video::E_DRIVER_TYPE driverType = irr::video::EDT_DIRECT3D9;
#else
    const irr::video::E_DRIVER_TYPE driverType = irr::video::EDT_OPENGL;
#endif

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

    struct LevelInfo
    {
        std::string name;
        int track;
    };

    LevelInfo levels[] = {
        {"LEVEL1", 57}, // 0
        {"LEVEL2", 57},
        {"LEVEL3A", 57},
        {"LEVEL3B", 57},
        {"LEVEL4", 59},
        {"LEVEL5", 59}, // 5
        {"LEVEL6", 59},
        {"LEVEL7A", 58},
        {"LEVEL7B", 58},
        {"LEVEL8A", 59},
        {"LEVEL8B", 59}, // 10
        {"LEVEL8C", 59},
        {"LEVEL10A", 58},
        {"LEVEL10B", 60},
        {"LEVEL10C", 60} // 14
    };

    const LevelInfo& lvlInfo = levels[14];

    auto lvl = level::Level::createLoader("data/tr1/data/" + lvlInfo.name + ".PHD", level::Game::Unknown);

    BOOST_ASSERT(lvl != nullptr);
    auto driver = device->getVideoDriver();
    lvl->load(driver);
    lvl->toIrrlicht(device);

    device->setWindowCaption(L"EdisonEngine");

    auto timer = device->getTimer();
    auto lastTime = timer->getTime();

    lvl->startTrack(lvlInfo.track);

    while(device->run())
    {
        lvl->m_audioDev.update();

        if(lvl->m_activeCDTrack > 0 && lvl->m_cdStream == nullptr)
            lvl->playStream(lvl->m_activeCDTrack);

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
        lvl->m_fx->update();
        lvl->drawBars(device->getVideoDriver());

        drawText(device->getGUIEnvironment(), 10, 40, lvl->m_lara->getCurrentRoom()->node->getName());
        drawText(device->getGUIEnvironment(), 100, 40, irr::core::stringw(std::lround(lvl->m_lara->getRotation().Y.toDegrees())));
        drawText(device->getGUIEnvironment(), 140, 20, irr::core::stringw(std::lround(lvl->m_lara->getPosition().X)));
        drawText(device->getGUIEnvironment(), 140, 30, irr::core::stringw(std::lround(lvl->m_lara->getPosition().Y)));
        drawText(device->getGUIEnvironment(), 140, 40, irr::core::stringw(std::lround(lvl->m_lara->getPosition().Z)));
        drawText(device->getGUIEnvironment(), 180, 40, irr::core::stringw(std::lround(lvl->m_lara->getFallSpeed().getCurrentValue())));
        drawText(device->getGUIEnvironment(), 10, 60, toString(lvl->m_lara->getCurrentAnimState()));
        drawText(device->getGUIEnvironment(), 100, 60, toString(lvl->m_lara->getTargetState()));
        drawText(device->getGUIEnvironment(), 10, 80, irr::core::stringw(lvl->m_lara->getCurrentFrame()));
        drawText(device->getGUIEnvironment(), 100, 80, toString(static_cast<loader::AnimationId>(lvl->m_lara->getCurrentAnimationId())));
        {
            int y = 100;
            for(const std::unique_ptr<engine::ItemController>& item : lvl->m_itemControllers | boost::adaptors::map_values)
            {
                if(!item->m_isActive)
                    continue;

                drawText(device->getGUIEnvironment(), 10, y, item->getName().c_str());
                if(item->m_flags2_02_toggledOn)
                    drawText(device->getGUIEnvironment(), 180, y, "toggled");
                if(item->m_flags2_04_ready)
                    drawText(device->getGUIEnvironment(), 220, y, "ready");
                drawText(device->getGUIEnvironment(), 260, y, irr::core::stringw(item->m_triggerTimeout));
                y += 20;
            }
        }

        device->getGUIEnvironment()->drawAll();

        device->getVideoDriver()->endScene();

        // update information about current frame-rate
        irr::core::stringw str(L"FPS: ");
        str.append(irr::core::stringw(driver->getFPS()));
        str += L" Tris: ";
        str.append(irr::core::stringw(driver->getPrimitiveCountDrawn()));
        device->setWindowCaption(str.c_str());
    }

    //device->drop();

    return EXIT_SUCCESS;
}
