#include "Base.h"
#include "BoundingBox.h"


namespace gameplay
{
    BoundingBox::BoundingBox()
    {
    }


    BoundingBox::BoundingBox(const glm::vec3& min, const glm::vec3& max)
    {
        set(min, max);
    }


    BoundingBox::BoundingBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
    {
        set(minX, minY, minZ, maxX, maxY, maxZ);
    }


    BoundingBox::BoundingBox(const BoundingBox& copy)
    {
        set(copy);
    }


    BoundingBox::~BoundingBox()
    {
    }


    const BoundingBox& BoundingBox::empty()
    {
        static BoundingBox b;
        return b;
    }


    void BoundingBox::getCorners(glm::vec3* dst) const
    {
        BOOST_ASSERT(dst);

        // Near face, specified counter-clockwise looking towards the origin from the positive z-axis.
        // Left-top-front.
        dst[0] = { min.x, max.y, max.z };
        // Left-bottom-front.
        dst[1] = { min.x, min.y, max.z };
        // Right-bottom-front.
        dst[2] = { max.x, min.y, max.z };
        // Right-top-front.
        dst[3] = { max.x, max.y, max.z };

        // Far face, specified counter-clockwise looking towards the origin from the negative z-axis.
        // Right-top-back.
        dst[4] = { max.x, max.y, min.z };
        // Right-bottom-back.
        dst[5] = { max.x, min.y, min.z };
        // Left-bottom-back.
        dst[6] = { min.x, min.y, min.z };
        // Left-top-back.
        dst[7] = { min.x, max.y, min.z };
    }


    glm::vec3 BoundingBox::getCenter() const
    {
        glm::vec3 center;
        getCenter(&center);
        return center;
    }


    void BoundingBox::getCenter(glm::vec3* dst) const
    {
        BOOST_ASSERT(dst);

        *dst = glm::mix(min, max, 0.5f);
    }


    bool BoundingBox::intersects(const BoundingBox& box) const
    {
        return ((min.x >= box.min.x && min.x <= box.max.x) || (box.min.x >= min.x && box.min.x <= max.x)) &&
            ((min.y >= box.min.y && min.y <= box.max.y) || (box.min.y >= min.y && box.min.y <= max.y)) &&
            ((min.z >= box.min.z && min.z <= box.max.z) || (box.min.z >= min.z && box.min.z <= max.z));
    }


    bool BoundingBox::isEmpty() const
    {
        return min.x == max.x && min.y == max.y && min.z == max.z;
    }


    void BoundingBox::merge(const BoundingBox& box)
    {
        // Calculate the new minimum point.
        min.x = std::min(min.x, box.min.x);
        min.y = std::min(min.y, box.min.y);
        min.z = std::min(min.z, box.min.z);

        // Calculate the new maximum point.
        max.x = std::max(max.x, box.max.x);
        max.y = std::max(max.y, box.max.y);
        max.z = std::max(max.z, box.max.z);
    }


    void BoundingBox::set(const glm::vec3& min, const glm::vec3& max)
    {
        this->min = min;
        this->max = max;
    }


    void BoundingBox::set(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
    {
        min = { minX, minY, minZ };
        max = { maxX, maxY, maxZ };
    }


    static void updateMinMax(glm::vec3* point, glm::vec3* min, glm::vec3* max)
    {
        BOOST_ASSERT(point);
        BOOST_ASSERT(min);
        BOOST_ASSERT(max);

        // Leftmost point.
        if( point->x < min->x )
        {
            min->x = point->x;
        }

        // Rightmost point.
        if( point->x > max->x )
        {
            max->x = point->x;
        }

        // Lowest point.
        if( point->y < min->y )
        {
            min->y = point->y;
        }

        // Highest point.
        if( point->y > max->y )
        {
            max->y = point->y;
        }

        // Farthest point.
        if( point->z < min->z )
        {
            min->z = point->z;
        }

        // Nearest point.
        if( point->z > max->z )
        {
            max->z = point->z;
        }
    }


    void BoundingBox::set(const BoundingBox& box)
    {
        min = box.min;
        max = box.max;
    }


    void BoundingBox::transform(const glm::mat4& matrix)
    {
        // Calculate the corners.
        glm::vec3 corners[8];
        getCorners(corners);

        // Transform the corners, recalculating the min and max points along the way.
        corners[0] = glm::vec3(matrix * glm::vec4(corners[0], 1.0f));
        glm::vec3 newMin = corners[0];
        glm::vec3 newMax = corners[0];
        for( int i = 1; i < 8; i++ )
        {
            corners[i] = glm::vec3(matrix * glm::vec4(corners[i], 1));
            updateMinMax(&corners[i], &newMin, &newMax);
        }
        this->min.x = newMin.x;
        this->min.y = newMin.y;
        this->min.z = newMin.z;
        this->max.x = newMax.x;
        this->max.y = newMax.y;
        this->max.z = newMax.z;
    }
}
