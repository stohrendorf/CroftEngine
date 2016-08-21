#include "Platform.h"


namespace gameplay
{
    inline Game::State Game::getState() const
    {
        return _state;
    }


    inline bool Game::isInitialized() const
    {
        return _initialized;
    }


    inline unsigned int Game::getFrameRate() const
    {
        return _frameRate;
    }


    inline unsigned int Game::getWidth() const
    {
        return _width;
    }


    inline unsigned int Game::getHeight() const
    {
        return _height;
    }


    inline float Game::getAspectRatio() const
    {
        return static_cast<float>(_width) / static_cast<float>(_height);
    }


    inline const Rectangle& Game::getViewport() const
    {
        return _viewport;
    }


    inline AnimationController* Game::getAnimationController() const
    {
        return _animationController;
    }


    inline AIController* Game::getAIController() const
    {
        return _aiController;
    }


    template<class T>
    void Game::renderOnce(T* instance, void (T::*method)(void*), void* cookie)
    {
        GP_ASSERT(instance);
        (instance ->* method)(cookie);
        Platform::swapBuffers();
    }


    inline void Game::setMultiSampling(bool enabled)
    {
        Platform::setMultiSampling(enabled);
    }


    inline bool Game::isMultiSampling() const
    {
        return Platform::isMultiSampling();
    }


    inline bool Game::canExit() const
    {
        return Platform::canExit();
    }
}
