#include "Base.h"
#include "Frustum.h"
#include "BoundingSphere.h"
#include "BoundingBox.h"


namespace gameplay
{
    Frustum::Frustum()
    {
        set(glm::mat4{1.0f});
    }


    Frustum::Frustum(const glm::mat4& matrix)
    {
        set(matrix);
    }


    Frustum::Frustum(const Frustum& frustum)
    {
        set(frustum);
    }


    Frustum::~Frustum()
    {
    }


    const Plane& Frustum::getNear() const
    {
        return _near;
    }


    const Plane& Frustum::getFar() const
    {
        return _far;
    }


    const Plane& Frustum::getLeft() const
    {
        return _left;
    }


    const Plane& Frustum::getRight() const
    {
        return _right;
    }


    const Plane& Frustum::getBottom() const
    {
        return _bottom;
    }


    const Plane& Frustum::getTop() const
    {
        return _top;
    }


    void Frustum::getMatrix(glm::mat4* dst) const
    {
        BOOST_ASSERT(dst);
        *dst = _matrix;
    }


    void Frustum::getCorners(glm::vec3* corners) const
    {
        getNearCorners(corners);
        getFarCorners(corners + 4);
    }


    void Frustum::getNearCorners(glm::vec3* corners) const
    {
        BOOST_ASSERT(corners);

        Plane::intersection(_near, _left, _top, &corners[0]);
        Plane::intersection(_near, _left, _bottom, &corners[1]);
        Plane::intersection(_near, _right, _bottom, &corners[2]);
        Plane::intersection(_near, _right, _top, &corners[3]);
    }


    void Frustum::getFarCorners(glm::vec3* corners) const
    {
        BOOST_ASSERT(corners);

        Plane::intersection(_far, _right, _top, &corners[0]);
        Plane::intersection(_far, _right, _bottom, &corners[1]);
        Plane::intersection(_far, _left, _bottom, &corners[2]);
        Plane::intersection(_far, _left, _top, &corners[3]);
    }


    bool Frustum::intersects(const glm::vec3& point) const
    {
        if( _near.distance(point) <= 0 )
            return false;
        if( _far.distance(point) <= 0 )
            return false;
        if( _left.distance(point) <= 0 )
            return false;
        if( _right.distance(point) <= 0 )
            return false;
        if( _top.distance(point) <= 0 )
            return false;
        if( _bottom.distance(point) <= 0 )
            return false;

        return true;
    }


    bool Frustum::intersects(float x, float y, float z) const
    {
        return intersects(glm::vec3(x, y, z));
    }


    bool Frustum::intersects(const BoundingSphere& sphere) const
    {
        return sphere.intersects(*this);
    }


    bool Frustum::intersects(const BoundingBox& box) const
    {
        return box.intersects(*this);
    }


    float Frustum::intersects(const Plane& plane) const
    {
        return plane.intersects(*this);
    }


    float Frustum::intersects(const Ray& ray) const
    {
        return ray.intersects(*this);
    }


    void Frustum::set(const Frustum& frustum)
    {
        _near = frustum._near;
        _far = frustum._far;
        _bottom = frustum._bottom;
        _top = frustum._top;
        _left = frustum._left;
        _right = frustum._right;
        _matrix = frustum._matrix;
    }


    void Frustum::updatePlanes()
    {
        _near  .set(glm::vec3(_matrix[0][3] + _matrix[0][2], _matrix[1][3] + _matrix[1][2], _matrix[2][3] + _matrix[2][2]), _matrix[3][3] + _matrix[3][2]);
        _far   .set(glm::vec3(_matrix[0][3] - _matrix[0][2], _matrix[1][3] - _matrix[1][2], _matrix[2][3] - _matrix[2][2]), _matrix[3][3] - _matrix[3][2]);
        _bottom.set(glm::vec3(_matrix[0][3] + _matrix[0][1], _matrix[1][3] + _matrix[1][1], _matrix[2][3] + _matrix[2][1]), _matrix[3][3] + _matrix[3][1]);
        _top   .set(glm::vec3(_matrix[0][3] - _matrix[0][1], _matrix[1][3] - _matrix[1][1], _matrix[2][3] - _matrix[2][1]), _matrix[3][3] - _matrix[3][1]);
        _left  .set(glm::vec3(_matrix[0][3] + _matrix[0][0], _matrix[1][3] + _matrix[1][0], _matrix[2][3] + _matrix[2][0]), _matrix[3][3] + _matrix[3][0]);
        _right .set(glm::vec3(_matrix[0][3] - _matrix[0][0], _matrix[1][3] - _matrix[1][0], _matrix[2][3] - _matrix[2][0]), _matrix[3][3] - _matrix[3][0]);
    }


    void Frustum::set(const glm::mat4& matrix)
    {
        _matrix = matrix;

        // Update the planes.
        updatePlanes();
    }
}
