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
#ifdef _MSC_VER
    const irr::video::E_DRIVER_TYPE driverType = irr::video::EDT_DIRECT3D9;
#else
    const irr::video::E_DRIVER_TYPE driverType = irr::video::EDT_OPENGL;
#endif

    irr::SIrrlichtCreationParameters driverParams;
    driverParams.AntiAlias = 255;
    driverParams.ZBufferBits = 16;
    driverParams.Stencilbuffer = true;
    driverParams.Vsync = false;
    driverParams.Bits = 24;
    driverParams.Fullscreen = false;
    driverParams.DriverType = driverType;
    driverParams.WindowSize = irr::core::dimension2d<irr::u32>(1680, 1050);

    irr::IrrlichtDevice* device = irr::createDeviceEx( driverParams );
    if(!device)
        return EXIT_FAILURE;
    
    auto l = level::Level::createLoader("data/tr1/data/LEVEL3B.PHD", level::Game::Unknown);
    BOOST_ASSERT(l != nullptr);
    auto driver = device->getVideoDriver();
    l->load(driver);
    l->toIrrlicht(device);
    
    device->setWindowCaption(L"EdisonEngine");
    
    auto timer = device->getTimer();
    auto lastTime = timer->getTime();
    
    while(device->run())
    {
        if(!device->isWindowActive())
        {
            device->yield();
            continue;
        }
        auto currentTime = timer->getTime()-lastTime;
        if(currentTime <= 0)
        {
            device->yield();
            continue;
        }
        
        lastTime = timer->getTime();
        
        device->getVideoDriver()->beginScene(true, true);
        //device->getSceneManager()->drawAll();
        l->m_fx->update();
        l->drawBars(device->getVideoDriver());

        drawText(device->getGUIEnvironment(), 10, 40, l->m_lara->getCurrentRoom()->node->getName());
        drawText(device->getGUIEnvironment(), 10, 60, toString(l->m_lara->getCurrentAnimState()));
        drawText(device->getGUIEnvironment(), 10, 80, irr::core::stringw(l->m_lara->getCurrentAnimationId()));
        drawText(device->getGUIEnvironment(), 30, 80, irr::core::stringw(l->m_lara->getCurrentFrame()));
        {
            int y = 100;
            for(const std::unique_ptr<engine::ItemController>& item : l->m_itemControllers | boost::adaptors::map_values)
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
