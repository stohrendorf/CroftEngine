#include "Base.h"
#include "Light.h"
#include "Node.h"

#include <boost/log/trivial.hpp>

namespace gameplay
{
    Light::Light(Light::Type type, const glm::vec3& color)
        : _type(type)
        , _light{ Directional{color} }
        , _node(nullptr)
    {
    }


    Light::Light(Light::Type type, const glm::vec3& color, float range)
        : _type(type)
        , _light{ Point{color, range} }
        , _node(nullptr)
    {
    }


    Light::Light(Light::Type type, const glm::vec3& color, float range, float innerAngle, float outerAngle)
        : _type(type)
        , _light{ Spot{color, range, innerAngle, outerAngle} }
        , _node(nullptr)
    {
    }


    Light::~Light() = default;


    std::shared_ptr<Light> Light::createDirectional(const glm::vec3& color)
    {
        return std::make_shared<Light>(DIRECTIONAL, color);
    }


    std::shared_ptr<Light> Light::createDirectional(float red, float green, float blue)
    {
        return std::make_shared<Light>(DIRECTIONAL, glm::vec3(red, green, blue));
    }


    std::shared_ptr<Light> Light::createPoint(const glm::vec3& color, float range)
    {
        return std::make_shared<Light>(POINT, color, range);
    }


    std::shared_ptr<Light> Light::createPoint(float red, float green, float blue, float range)
    {
        return std::make_shared<Light>(POINT, glm::vec3(red, green, blue), range);
    }


    std::shared_ptr<Light> Light::createSpot(const glm::vec3& color, float range, float innerAngle, float outerAngle)
    {
        return std::make_shared<Light>(SPOT, color, range, innerAngle, outerAngle);
    }


    std::shared_ptr<Light> Light::createSpot(float red, float green, float blue, float range, float innerAngle, float outerAngle)
    {
        return std::make_shared<Light>(SPOT, glm::vec3(red, green, blue), range, innerAngle, outerAngle);
    }


    Light::Type Light::getLightType() const
    {
        return _type;
    }


    Node* Light::getNode() const
    {
        return _node;
    }


    void Light::setNode(Node* node)
    {
        // Connect the new node.
        _node = node;
    }


    const glm::vec3& Light::getColor() const
    {
        switch( _type )
        {
            case DIRECTIONAL:
                return boost::get<Directional>(_light).color;
            case POINT:
                return boost::get<Point>(_light).color;
            case SPOT:
                return boost::get<Spot>(_light).color;
            default:
                BOOST_LOG_TRIVIAL(error) << "Unsupported light type (" << _type << ").";
        }
        static const glm::vec3 zero{ 0,0,0 };
        return zero;
    }


    void Light::setColor(const glm::vec3& color)
    {
        switch( _type )
        {
            case DIRECTIONAL:
                boost::get<Directional>(_light).color = color;
                break;
            case POINT:
                boost::get<Point>(_light).color = color;
                break;
            case SPOT:
                boost::get<Spot>(_light).color = color;
                break;
            default:
                BOOST_LOG_TRIVIAL(error) << "Unsupported light type (" << _type << ").";
                break;
        }
    }


    void Light::setColor(float red, float green, float blue)
    {
        setColor(glm::vec3(red, green, blue));
    }


    float Light::getRange() const
    {
        BOOST_ASSERT(_type != DIRECTIONAL);

        switch( _type )
        {
            case POINT:
                return boost::get<Point>(_light).range;
            case SPOT:
                return boost::get<Spot>(_light).range;
            default:
                BOOST_LOG_TRIVIAL(error) << "Unsupported light type (" << _type << ").";
                return 0.0f;
        }
    }


    void Light::setRange(float range)
    {
        BOOST_ASSERT(_type != DIRECTIONAL);

        switch( _type )
        {
            case POINT:
                boost::get<Point>(_light).range = range;
                boost::get<Point>(_light).rangeInverse = 1.0f / range;
                break;
            case SPOT:
                boost::get<Spot>(_light).range = range;
                boost::get<Spot>(_light).rangeInverse = 1.0f / range;
                break;
            default:
                BOOST_LOG_TRIVIAL(error) << "Unsupported light type (" << _type << ").";
                break;
        }
    }


    float Light::getRangeInverse() const
    {
        BOOST_ASSERT(_type != DIRECTIONAL);

        switch( _type )
        {
            case POINT:
                return boost::get<Point>(_light).rangeInverse;
            case SPOT:
                return boost::get<Spot>(_light).rangeInverse;
            default:
                BOOST_LOG_TRIVIAL(error) << "Unsupported light type (" << _type << ").";
                return 0.0f;
        }
    }


    float Light::getInnerAngle() const
    {
        BOOST_ASSERT(_type == SPOT);

        return boost::get<Spot>(_light).innerAngle;
    }


    void Light::setInnerAngle(float innerAngle)
    {
        BOOST_ASSERT(_type == SPOT);

        boost::get<Spot>(_light).innerAngle = innerAngle;
        boost::get<Spot>(_light).innerAngleCos = std::cos(innerAngle);
    }


    float Light::getOuterAngle() const
    {
        BOOST_ASSERT(_type == SPOT);

        return boost::get<Spot>(_light).outerAngle;
    }


    void Light::setOuterAngle(float outerAngle)
    {
        BOOST_ASSERT(_type == SPOT);

        boost::get<Spot>(_light).outerAngle = outerAngle;
        boost::get<Spot>(_light).outerAngleCos = std::cos(outerAngle);
    }


    float Light::getInnerAngleCos() const
    {
        BOOST_ASSERT(_type == SPOT);

        return boost::get<Spot>(_light).innerAngleCos;
    }


    float Light::getOuterAngleCos() const
    {
        BOOST_ASSERT(_type == SPOT);

        return boost::get<Spot>(_light).outerAngleCos;
    }


    Light::Directional::Directional(const glm::vec3& color)
        : color(color)
    {
    }


    Light::Point::Point(const glm::vec3& color, float range)
        : color(color)
        , range(range)
    {
        rangeInverse = 1.0f / range;
    }


    Light::Spot::Spot(const glm::vec3& color, float range, float innerAngle, float outerAngle)
        : color(color)
        , range(range)
        , innerAngle(innerAngle)
        , outerAngle(outerAngle)
    {
        rangeInverse = 1.0f / range;
        innerAngleCos = std::cos(innerAngle);
        outerAngleCos = std::cos(outerAngle);
    }
}
