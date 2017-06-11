#pragma once

#include <glm/glm.hpp>


namespace core
{
    struct BoundingBox final
    {
        explicit BoundingBox() = default;


        explicit BoundingBox(const glm::ivec3& min_, const glm::ivec3& max_)
            : min{min_}
            , max{max_}
        {
        }

        bool isValid() const noexcept
        {
            return min.x <= max.x && min.y <= max.y && min.z <= max.z;
        }

        void makeValid() noexcept
        {
            if (min.x > max.x)
                std::swap(min.x, max.x);
            if (min.y > max.y)
                std::swap(min.y, max.y);
            if (min.z > max.z)
                std::swap(min.z, max.z);
        }

        bool intersects(const BoundingBox& box) const noexcept
        {
            return ((min.x >= box.min.x && min.x <= box.max.x) || (box.min.x >= min.x && box.min.x <= max.x)) &&
                ((min.y >= box.min.y && min.y <= box.max.y) || (box.min.y >= min.y && box.min.y <= max.y)) &&
                ((min.z >= box.min.z && min.z <= box.max.z) || (box.min.z >= min.z && box.min.z <= max.z));
        }

        bool contains(const glm::ivec3& v) const noexcept
        {
            return v.x >= min.x && v.x <= max.x
                && v.y >= min.y && v.y <= max.y
                && v.z >= min.z && v.z <= max.z;
        }

        glm::ivec3 min{0};

        glm::ivec3 max{0};
    };
}
