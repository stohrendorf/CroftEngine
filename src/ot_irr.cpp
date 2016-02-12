#include "loader/level.h"

int main()
{
    irr::IrrlichtDevice* device = irr::createDevice( irr::video::EDT_OPENGL, irr::core::dimension2d<irr::u32>(1024, 768), 16, false, false, false, 0);    
    if(!device)
        return EXIT_FAILURE;
    
    auto l = loader::Level::createLoader("data/tr1/data/LEVEL1.PHD", loader::Game::Unknown);
    l->load(device->getVideoDriver());
    l->toIrrlicht(device->getSceneManager());
    
    device->setWindowCaption(L"IrrOT");
    
    while(device->run())
    {
        if(!device->isWindowActive())
            continue;
        device->getVideoDriver()->beginScene(true, true);
        device->getSceneManager()->drawAll();
        device->getVideoDriver()->endScene();
    }
    
    device->drop();
    
    return EXIT_SUCCESS;
}
