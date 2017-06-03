#pragma once

#include "controlleraxis.h"
#include "controllerbutton.h"

#include <yaml-cpp/yaml.h>
#include <GLFW/glfw3.h>

#include <map>
#include <set>

namespace engine
{
class ControllerLayout
{
public:
    explicit ControllerLayout(const std::string& configFilename)
    {
        YAML::Node config = YAML::LoadFile(configFilename);
        for(const auto& name : config["names"])
        {
            m_names.insert(name.as<std::string>());
        }
        for(const auto& button : config["buttons"])
        {
            m_buttons[EnumUtil<ControllerButton>::fromString(button["name"].as<std::string>())] = button["index"].as<int>();
        }
        for(const auto& axis : config["axes"])
        {
            m_axes[EnumUtil<ControllerAxis>::fromString(axis["name"].as<std::string>())] = axis["index"].as<int>();
        }
    }

    bool isButtonPressed(ControllerButton btn, const gsl::span<const uint8_t>& buttons) const
    {
        const auto it = m_buttons.find(btn);
        if(it == m_buttons.end())
            return false;

        return buttons.size() > it->second && buttons[it->second] == GLFW_PRESS;
    };

    float getAxisValue(ControllerAxis axis, const gsl::span<const float>& values, float def) const
    {
        const auto it = m_axes.find(axis);
        if(it == m_axes.end())
            return def;

        return values.size() > it->second ? values[it->second] : def;
    };

    const std::set<std::string>& getNames() const
    {
        return m_names;
    }

private:
    std::set<std::string> m_names;
    std::map<ControllerAxis, int> m_axes;
    std::map<ControllerButton, int> m_buttons;
};
}
