#include "loader/level.h"

int main()
{
#ifdef _MSC_VER
    const irr::video::E_DRIVER_TYPE driverType = irr::video::EDT_DIRECT3D9;
#else
    const irr::video::E_DRIVER_TYPE driverType = irr::video::EDT_OPENGL;
#endif

    irr::SIrrlichtCreationParameters driverParams;
    driverParams.AntiAlias = 255;
    driverParams.ZBufferBits = 24;
    driverParams.Stencilbuffer = true;
    driverParams.Vsync = false;
    driverParams.Bits = 24;
    driverParams.Fullscreen = false;
    driverParams.DriverType = driverType;
    driverParams.WindowSize = irr::core::dimension2d<irr::u32>(1024, 768);

    irr::IrrlichtDevice* device = irr::createDeviceEx( driverParams );
    if(!device)
        return EXIT_FAILURE;
    
    auto l = loader::Level::createLoader("data/tr1/data/LEVEL1.PHD", loader::Game::Unknown);
    BOOST_ASSERT(l != nullptr);
    auto driver = device->getVideoDriver();
    l->load(driver);
    l->toIrrlicht(device->getSceneManager(), device->getCursorControl());
    
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
        device->getSceneManager()->drawAll();
        device->getVideoDriver()->endScene();
        
        l->updateTriggers(lastTime / 1000.0f);
        
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
