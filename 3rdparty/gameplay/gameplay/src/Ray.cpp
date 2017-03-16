#include "Base.h"
#include "Ray.h"
#include "Plane.h"
#include "Frustum.h"
#include "BoundingBox.h"

#include <boost/log/trivial.hpp>
#include <glm/gtc/constants.hpp>


namespace gameplay
{
    Ray::Ray()
        : _direction(0, 0, 1)
    {
    }


    Ray::Ray(const glm::vec3& origin, const glm::vec3& direction)
    {
        set(origin, direction);
    }


    Ray::Ray(float originX, float originY, float originZ, float dirX, float dirY, float dirZ)
    {
        set(glm::vec3{ originX, originY, originZ }, glm::vec3{ dirX, dirY, dirZ });
    }


    Ray::Ray(const Ray& copy)
    {
        set(copy);
    }


    Ray::~Ray() = default;


    const glm::vec3& Ray::getOrigin() const
    {
        return _origin;
    }


    void Ray::setOrigin(const glm::vec3& origin)
    {
        _origin = origin;
    }


    void Ray::setOrigin(float x, float y, float z)
    {
        _origin = {x, y, z};
    }


    const glm::vec3& Ray::getDirection() const
    {
        return _direction;
    }


    void Ray::setDirection(const glm::vec3& direction)
    {
        _direction = direction;
        normalize();
    }


    void Ray::setDirection(float x, float y, float z)
    {
        _direction = {x, y, z};
        normalize();
    }


    bool Ray::intersects(const BoundingBox& box) const
    {
        return box.intersects(*this);
    }


    bool Ray::intersects(const Plane& plane) const
    {
        const glm::vec3& normal = plane.getNormal();
        // If the origin of the ray is on the plane then the distance is zero.
        float alpha = glm::dot(normal, _origin) + plane.getDistance();
        if( fabs(alpha) < glm::epsilon<float>() )
        {
            return true; // 0.0f;
        }

        float dot = glm::dot(normal, _direction);

        // If the dot product of the plane's normal and this ray's direction is zero,
        // then the ray is parallel to the plane and does not intersect it.
        if( dot == 0.0f )
        {
            return false;
        }

        // Calculate the distance along the ray's direction vector to the point where
        // the ray intersects the plane (if it is negative the plane is behind the ray).
        float d = -alpha / dot;
        if( d < 0.0f )
        {
            return false;
        }
        return true; // d;
    }


    void Ray::set(const glm::vec3& origin, const glm::vec3& direction)
    {
        _origin = origin;
        _direction = direction;
        normalize();
    }


    void Ray::set(const Ray& ray)
    {
        _origin = ray._origin;
        _direction = ray._direction;
        normalize();
    }


    void Ray::transform(const glm::mat4& matrix)
    {
        _origin = glm::vec3(matrix * glm::vec4(_origin, 1));
        _direction = glm::normalize(glm::vec3(matrix * glm::vec4(_direction, 1)));
    }


    void Ray::normalize()
    {
        if( glm::length(_direction) <= std::numeric_limits<float>::epsilon() )
        {
            BOOST_LOG_TRIVIAL(error) << "Invalid ray object; a ray's direction must be non-zero.";
            return;
        }

        _direction = glm::normalize(_direction);
    }
}
