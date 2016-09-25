#include "Base.h"
#include "Light.h"
#include "Node.h"

#include <boost/log/trivial.hpp>

namespace gameplay
{
    Light::Light(Light::Type type, const glm::vec3& color)
        : _type(type)
        , _node(nullptr)
    {
        _directional = new Directional(color);
    }


    Light::Light(Light::Type type, const glm::vec3& color, float range)
        : _type(type)
        , _node(nullptr)
    {
        _point = new Point(color, range);
    }


    Light::Light(Light::Type type, const glm::vec3& color, float range, float innerAngle, float outerAngle)
        : _type(type)
        , _node(nullptr)
    {
        _spot = new Spot(color, range, innerAngle, outerAngle);
    }


    Light::~Light()
    {
        switch( _type )
        {
            case DIRECTIONAL:
                SAFE_DELETE(_directional);
                break;
            case POINT:
                SAFE_DELETE(_point);
                break;
            case SPOT:
                SAFE_DELETE(_spot);
                break;
            default:
                BOOST_LOG_TRIVIAL(error) << "Invalid light type (" << _type << ").";
                break;
        }
    }


    Light* Light::createDirectional(const glm::vec3& color)
    {
        return new Light(DIRECTIONAL, color);
    }


    Light* Light::createDirectional(float red, float green, float blue)
    {
        return new Light(DIRECTIONAL, glm::vec3(red, green, blue));
    }


    Light* Light::createPoint(const glm::vec3& color, float range)
    {
        return new Light(POINT, color, range);
    }


    Light* Light::createPoint(float red, float green, float blue, float range)
    {
        return new Light(POINT, glm::vec3(red, green, blue), range);
    }


    Light* Light::createSpot(const glm::vec3& color, float range, float innerAngle, float outerAngle)
    {
        return new Light(SPOT, color, range, innerAngle, outerAngle);
    }


    Light* Light::createSpot(float red, float green, float blue, float range, float innerAngle, float outerAngle)
    {
        return new Light(SPOT, glm::vec3(red, green, blue), range, innerAngle, outerAngle);
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
                BOOST_ASSERT(_directional);
                return _directional->color;
            case POINT:
                BOOST_ASSERT(_point);
                return _point->color;
            case SPOT:
                BOOST_ASSERT(_spot);
                return _spot->color;
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
                BOOST_ASSERT(_directional);
                _directional->color = color;
                break;
            case POINT:
                BOOST_ASSERT(_point);
                _point->color = color;
                break;
            case SPOT:
                BOOST_ASSERT(_spot);
                _spot->color = color;
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
                BOOST_ASSERT(_point);
                return _point->range;
            case SPOT:
                BOOST_ASSERT(_spot);
                return _spot->range;
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
                BOOST_ASSERT(_point);
                _point->range = range;
                _point->rangeInverse = 1.0f / range;
                break;
            case SPOT:
                BOOST_ASSERT(_spot);
                _spot->range = range;
                _spot->rangeInverse = 1.0f / range;
                break;
            default:
                BOOST_LOG_TRIVIAL(error) << "Unsupported light type (" << _type << ").";
                break;
        }

        if( _node )
            _node->setBoundsDirty();
    }


    float Light::getRangeInverse() const
    {
        BOOST_ASSERT(_type != DIRECTIONAL);

        switch( _type )
        {
            case POINT:
                BOOST_ASSERT(_point);
                return _point->rangeInverse;
            case SPOT:
                BOOST_ASSERT(_spot);
                return _spot->rangeInverse;
            default:
                BOOST_LOG_TRIVIAL(error) << "Unsupported light type (" << _type << ").";
                return 0.0f;
        }
    }


    float Light::getInnerAngle() const
    {
        BOOST_ASSERT(_type == SPOT);

        return _spot->innerAngle;
    }


    void Light::setInnerAngle(float innerAngle)
    {
        BOOST_ASSERT(_type == SPOT);

        _spot->innerAngle = innerAngle;
        _spot->innerAngleCos = cos(innerAngle);
    }


    float Light::getOuterAngle() const
    {
        BOOST_ASSERT(_type == SPOT);

        return _spot->outerAngle;
    }


    void Light::setOuterAngle(float outerAngle)
    {
        BOOST_ASSERT(_type == SPOT);

        _spot->outerAngle = outerAngle;
        _spot->outerAngleCos = cos(outerAngle);

        if( _node )
            _node->setBoundsDirty();
    }


    float Light::getInnerAngleCos() const
    {
        BOOST_ASSERT(_type == SPOT);

        return _spot->innerAngleCos;
    }


    float Light::getOuterAngleCos() const
    {
        BOOST_ASSERT(_type == SPOT);

        return _spot->outerAngleCos;
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
        innerAngleCos = cos(innerAngle);
        outerAngleCos = cos(outerAngle);
    }
}
