#pragma once

#include "loader/datatypes.h"

namespace
{
    struct BoundingBox
    {
        glm::vec3 min;
        glm::vec3 max;

        BoundingBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
            : min{ minX, minY, minZ }
            , max{ maxX, maxY, maxZ }
        {
        }
    };
}

namespace render
{
    struct PortalTracer
    {
        BoundingBox boundingBox{-1, -1, 0, 1, 1, 0};
        const loader::Portal* lastPortal = nullptr;


        bool checkVisibility(const loader::Portal* portal, const gameplay::Camera& camera)
        {
            const auto portalToCam = glm::vec3{camera.getInverseViewMatrix()[3]} - portal->vertices[0].toRenderSystem();
            if( glm::dot(portal->normal.toRenderSystem(), portalToCam) < 0 )
            {
                return false; // wrong orientation (normals must face the camera)
            }

            int numBehind = 0, numTooFar = 0;

            // screen space bounding box of the portal
            BoundingBox portalBB{0, 0, 0, 0, 0, 0};
            portalBB.min = {1,1,0};
            portalBB.max = {-1,-1,0};

            for( auto vertex : portal->vertices )
            {
                const auto screen = projectOnScreen(vertex.toRenderSystem(), camera, numBehind, numTooFar);

                portalBB.min.x = std::min(portalBB.min.x, screen.x);
                portalBB.min.y = std::min(portalBB.min.y, screen.y);
                portalBB.max.x = std::max(portalBB.max.x, screen.x);
                portalBB.max.y = std::max(portalBB.max.y, screen.y);

                // the first vertex must set the boundingbox to a valid state
                BOOST_ASSERT(portalBB.min.x <= portalBB.max.x);
                BOOST_ASSERT(portalBB.min.y <= portalBB.max.y);
            }

            if( numBehind == 4 || numTooFar == 4 )
                return false;

            BOOST_ASSERT(numBehind <= 3);

            // intersect full portal path bounding box with current portal bounding box
            boundingBox.min.x = std::max(portalBB.min.x, boundingBox.min.x);
            boundingBox.min.y = std::max(portalBB.min.y, boundingBox.min.y);
            boundingBox.max.x = std::min(portalBB.max.x, boundingBox.max.x);
            boundingBox.max.y = std::min(portalBB.max.y, boundingBox.max.y);

            lastPortal = portal;

            return boundingBox.min.x < boundingBox.max.x && boundingBox.min.y < boundingBox.max.y;
        }


        uint16_t getLastDestinationRoom() const
        {
            return getLastPortal()->adjoining_room;
        }


        const loader::Portal* getLastPortal() const
        {
            Expects(lastPortal != nullptr);
            return lastPortal;
        }


    private:
        static glm::vec2 projectOnScreen(glm::vec3 vertex,
                                         const gameplay::Camera& camera,
                                         int& numBehind,
                                         int& numTooFar)
        {
            vertex = glm::vec3(camera.getViewMatrix() * glm::vec4(vertex, 1));

            if( vertex.z > -camera.getNearPlane() )
            {
                ++numBehind;
            }
            else if( vertex.z < -camera.getFarPlane() )
            {
                ++numTooFar;
            }

            // clamp to avoid div-by-near-zero
            if(vertex.z > -0.001f)
                vertex.z = -0.001f;

            glm::vec4 projVertex{vertex, 1};
            projVertex = camera.getProjectionMatrix() * projVertex;
            projVertex /= projVertex.w;

            glm::vec2 screen{glm::clamp(projVertex.x, -1.0f, 1.0f), glm::clamp(projVertex.y, -1.0f, 1.0f)};
            return screen;
        }
    };
}
