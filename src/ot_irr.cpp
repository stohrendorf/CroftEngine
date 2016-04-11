#include "loader/level.h"

int main()
{
    irr::IrrlichtDevice* device = irr::createDevice( irr::video::EDT_OPENGL, irr::core::dimension2d<irr::u32>(1024, 768), 10, false, false, false, nullptr);    
    if(!device)
        return EXIT_FAILURE;
    
    auto l = loader::Level::createLoader("data/tr1/data/LEVEL1.PHD", loader::Game::Unknown);
    BOOST_ASSERT(l != nullptr);
    auto driver = device->getVideoDriver();
    l->load(driver);
    l->toIrrlicht(device->getSceneManager(), device->getCursorControl());
    
    device->setWindowCaption(L"IrrOT");
    
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
