#pragma once

#include "render/scene/glfw.h"
#include "gsl-lite.hpp"

#include <map>

namespace hid
{
enum class ControllerButton
{
    a,
    b,
    x,
    y,
    back,
    guide,
    start,
    leftstick,
    rightstick,
    leftshoulder,
    rightshoulder,
    dpup,
    dpdown,
    dpleft,
    dpright,
};

enum class ControllerAxis
{
    leftx,
    lefty,
    rightx,
    righty,
    lefttrigger,
    righttrigger,
};


class Controller
{
public:
    class OutputHandler;


    friend OutputHandler;


    class InputChannel;


    friend InputChannel;

    explicit Controller(const std::vector<std::string>& mappings);

    ~Controller();

    void init();

    bool isButtonPressed(const ControllerButton btn) const
    {
        const auto it = m_buttons.find( btn );
        if( it == m_buttons.end() )
            return false;

        return it->second;
    }

    float getAxisValue(const ControllerAxis axis) const
    {
        const auto it = m_axes.find( axis );
        if( it == m_axes.end() )
            return 0;

        return it->second;
    }

    const std::string& getName() const
    {
        return m_name;
    }

    const std::string& getGuid() const
    {
        return m_guid;
    }

    void update(int controllerIndex);

private:
    std::string m_name;
    std::string m_guid;
    std::map<ControllerAxis, float> m_axes;
    std::map<ControllerButton, bool> m_buttons;
    std::vector<std::shared_ptr<InputChannel>> m_channels;
    std::vector<std::string> m_rawMappings;
};
}
