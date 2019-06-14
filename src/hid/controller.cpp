#include "controller.h"

#include <regex>

#include <boost/algorithm/string.hpp>
#include <boost/optional.hpp>
#include <utility>

namespace hid
{
class Controller::OutputHandler
{
public:
    virtual ~OutputHandler() = default;

    virtual void apply(Controller& controller, float value) = 0;

protected:
    static void setButton(Controller& controller, ControllerButton button, bool value = true)
    {
        controller.m_buttons[button] = value;
    }

    static void setAxis(Controller& controller, ControllerAxis axis, float value)
    {
        controller.m_axes[axis] = value;
    }
};


class ButtonOutputHandler final : public Controller::OutputHandler
{
public:
    explicit ButtonOutputHandler(ControllerButton button)
            : m_button{button}
    {}

    void apply(Controller& controller, float value) override
    {
        setButton( controller, m_button, value > 0 );
    }

private:
    const ControllerButton m_button;
};


class AxisOutputHandler final : public Controller::OutputHandler
{
public:
    explicit AxisOutputHandler(ControllerAxis axis)
            : m_axis{axis}
    {}

    void apply(Controller& controller, float value) override
    {
        setAxis( controller, m_axis, value );
    }

private:
    const ControllerAxis m_axis;
};


using AxisNormalizer = std::function<float(float)>;


class Controller::InputChannel
{
public:
    explicit InputChannel(std::unique_ptr<Controller::OutputHandler>&& outputHandler)
            : m_outputHandler{std::move( outputHandler )}
    {
    }

    virtual ~InputChannel() = default;

    virtual void
    update(Controller& controller,
           const gsl::span<const float>& axisValues,
           const gsl::span<const uint8_t>& hats,
           const gsl::span<const uint8_t>& buttons) = 0;

protected:
    std::unique_ptr<Controller::OutputHandler> m_outputHandler;
};


class AxisInputChannel final : public Controller::InputChannel
{
public:
    explicit AxisInputChannel(uint8_t inputAxis, AxisNormalizer normalizer,
                              std::unique_ptr<Controller::OutputHandler>&& outputHandler)
            : Controller::InputChannel{std::move( outputHandler )}
            , m_inputAxis{inputAxis}
            , m_normalizer{std::move( normalizer )}
    {
    }

    void
    update(Controller& controller,
           const gsl::span<const float>& axisValues,
           const gsl::span<const uint8_t>& hats,
           const gsl::span<const uint8_t>& buttons) override
    {
        m_outputHandler->apply( controller, m_normalizer( axisValues[m_inputAxis] ) );
    }

private:
    const uint8_t m_inputAxis;
    const AxisNormalizer m_normalizer;
};


class ButtonInputChannel final : public Controller::InputChannel
{
public:
    explicit ButtonInputChannel(uint8_t inputButton,
                                AxisNormalizer normalizer,
                                std::unique_ptr<Controller::OutputHandler>&& outputHandler)
            : Controller::InputChannel{std::move( outputHandler )}
            , m_inputButton{inputButton}
            , m_normalizer{std::move( normalizer )}
    {
    }

    void
    update(Controller& controller,
           const gsl::span<const float>& axisValues,
           const gsl::span<const uint8_t>& hats,
           const gsl::span<const uint8_t>& buttons) override
    {
        m_outputHandler->apply( controller, m_normalizer( buttons[m_inputButton] == GLFW_PRESS ? 1.0f : 0.0f ) );
    }

private:
    const uint8_t m_inputButton;
    const AxisNormalizer m_normalizer;
};


class HatInputChannel final : public Controller::InputChannel
{
public:
    explicit HatInputChannel(uint8_t inputHat, uint8_t inputMask, AxisNormalizer normalizer,
                             std::unique_ptr<Controller::OutputHandler>&& outputHandler)
            : Controller::InputChannel{std::move( outputHandler )}
            , m_inputHat{inputHat}
            , m_inputMask{inputMask}
            , m_normalizer{std::move( normalizer )}
    {
    }

    void
    update(Controller& controller,
           const gsl::span<const float>& axisValues,
           const gsl::span<const uint8_t>& hats,
           const gsl::span<const uint8_t>& buttons) override
    {
        m_outputHandler->apply( controller, m_normalizer( (hats[m_inputHat] & m_inputMask) != 0 ? 1.0f : 0.0f ) );
    }

private:
    const uint8_t m_inputHat;
    const uint8_t m_inputMask;
    const AxisNormalizer m_normalizer;
};

namespace
{
boost::optional<ControllerAxis> tryParseAxis(const std::string& s)
{
    if( s == "leftx" )
        return ControllerAxis::leftx;
    if( s == "lefty" )
        return ControllerAxis::lefty;
    if( s == "rightx" )
        return ControllerAxis::rightx;
    if( s == "righty" )
        return ControllerAxis::righty;
    if( s == "lefttrigger" )
        return ControllerAxis::lefttrigger;
    if( s == "righttrigger" )
        return ControllerAxis::righttrigger;

    return boost::none;
}

boost::optional<ControllerButton> tryParseButton(const std::string& s)
{
    if( s == "a" )
        return ControllerButton::a;
    if( s == "b" )
        return ControllerButton::b;
    if( s == "x" )
        return ControllerButton::x;
    if( s == "y" )
        return ControllerButton::y;
    if( s == "back" )
        return ControllerButton::back;
    if( s == "guide" )
        return ControllerButton::guide;
    if( s == "start" )
        return ControllerButton::start;
    if( s == "leftstick" )
        return ControllerButton::leftstick;
    if( s == "rightstick" )
        return ControllerButton::rightstick;
    if( s == "leftshoulder" )
        return ControllerButton::leftshoulder;
    if( s == "rightshoulder" )
        return ControllerButton::rightshoulder;
    if( s == "dpup" )
        return ControllerButton::dpup;
    if( s == "dpdown" )
        return ControllerButton::dpdown;
    if( s == "dpleft" )
        return ControllerButton::dpleft;
    if( s == "dpright" )
        return ControllerButton::dpright;

    return boost::none;
}

using InputNormalizer = std::function<float(float)>;
}

Controller::Controller(const std::vector<std::string>& mappings)
{
    Expects( mappings.size() >= 2 );
    m_guid = mappings[0];
    m_name = mappings[1];
    std::copy( std::next( mappings.begin(), 2 ), mappings.end(), std::back_inserter( m_rawMappings ) );
}

void Controller::init()
{
    if( !m_channels.empty() )
        return;

    BOOST_LOG_TRIVIAL( debug ) << "Loading controller mapping for " << m_name;

    for( const auto& rawMapping : m_rawMappings )
    {
        if( rawMapping.empty() )
            continue;

        if( boost::starts_with( rawMapping, "platform:" ) )
            continue;

        static const std::regex re{"([-+]?)" // 1 = +-
                                   "(.+)" // 2 = name
                                   ":"
                                   "([-+]?)" // 3 = +-
                                   // 4=abh, 5=aX, 6=bX, 7=hX.Y, 8=X, 9=Y, 10=~
                                   "((a[0-9]+)|(b[0-9]+)|(h([0-9]+)\\.([0-9]+)))(~?)",
                                   std::regex::icase | std::regex::optimize | std::regex::extended};

        std::smatch match;
        if( !std::regex_match( rawMapping, match, re ) )
        {
            BOOST_LOG_TRIVIAL( warning ) << "Ignoring invalid mapping: " << rawMapping;
            continue;
        }

        const auto name = boost::algorithm::to_lower_copy( match[2].str() );
        const auto axis = tryParseAxis( name );
        const auto button = tryParseButton( name );
        Expects( axis.is_initialized() ^ button.is_initialized() );

        const bool invertInput = match[10].length() > 0;
        const auto halfAxisOutput = match[1].str();
        const auto halfAxisInput = match[3].str();

        // value = inputMin + (value - inputMin) / (inputMax - inputMin) * (outputMax - outputMin)
        float inputMin = -1, inputMax = 1;
        float outputMin = -1, outputMax = 1;
        if( axis.is_initialized() )
        {
            if( *axis == ControllerAxis::lefttrigger || *axis == ControllerAxis::righttrigger )
            {
                outputMin = 0;
            }
            else if( halfAxisOutput == "+" )
            {
                outputMin = 0;
            }
            else if( halfAxisOutput == "-" )
            {
                outputMin = 0;
                outputMax = -1;
            }
        }

        std::unique_ptr<OutputHandler> outputHandler;
        if( axis.is_initialized() )
            outputHandler = std::make_unique<AxisOutputHandler>( *axis );
        else
            outputHandler = std::make_unique<ButtonOutputHandler>( *button );

        if( match[5].length() > 0 )
        {
            // axis
            if( halfAxisInput == "+" )
            {
                inputMin = 0;
            }
            else if( halfAxisInput == "-" )
            {
                inputMin = 0;
                inputMax = -1;
            }
            if( invertInput )
            {
                std::swap( inputMin, inputMax );
            }

            m_channels.emplace_back(
                    std::make_shared<AxisInputChannel>( std::stoi( match[5].str().substr( 1 ) ),
                                                        [=](float value) {
                                                            return inputMin + (value - inputMin) / (inputMax - inputMin)
                                                                              * (outputMax - outputMin);
                                                        },
                                                        std::move( outputHandler ) )
            );
        }
        else if( match[6].length() > 0 )
        {
            m_channels.emplace_back(
                    std::make_shared<ButtonInputChannel>( std::stoi( match[6].str().substr( 1 ) ),
                                                          [=](float value) {
                                                              return inputMin
                                                                     + (value - inputMin) / (inputMax - inputMin) * 2;
                                                          },
                                                          std::move( outputHandler ) )
            );
        }
        else if( match[7].length() > 0 )
        {
            const auto hatId = std::stoi( match[8].str() );
            const auto hatMask = std::stoi( match[9].str() );
            m_channels.emplace_back(
                    std::make_shared<HatInputChannel>( hatId, hatMask,
                                                       [=](float value) {
                                                           return inputMin
                                                                  + (value - inputMin) / (inputMax - inputMin) * 2;
                                                       },
                                                       std::move( outputHandler ) )
            );
        }
        else
        {
            BOOST_LOG_TRIVIAL( warning ) << "Ignoring invalid mapping \"" << rawMapping << "\" for controller "
                                         << m_name;
        }
    }
}

void Controller::update(int controllerIndex)
{
    int buttonCount = 0;
    const auto buttons = glfwGetJoystickButtons( controllerIndex, &buttonCount );
    const auto buttonSpan = gsl::make_span( buttons, buttonCount );

    int axisCount = 0;
    const auto axisValues = glfwGetJoystickAxes( controllerIndex, &axisCount );
    const auto axisSpan = gsl::make_span( axisValues, axisCount );

    int hatCount = 0;
    const auto hats = glfwGetJoystickHats( controllerIndex, &hatCount );
    const auto hatSpan = gsl::make_span( hats, hatCount );

    for( const auto& channel : m_channels )
        channel->update( *this, axisSpan, hatSpan, buttonSpan );
}

Controller::~Controller() = default;
}
