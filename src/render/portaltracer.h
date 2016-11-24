#pragma once

#include "loader/datatypes.h"


namespace render
{
    struct PortalTracer
    {
        gameplay::BoundingBox boundingBox{-1, -1, 0, 1, 1, 0};
        const loader::Portal* lastPortal = nullptr;


        bool checkVisibility(const loader::Portal* portal, const gameplay::Camera& camera)
        {
            const auto portalToCam = glm::vec3{camera.getInverseViewMatrix()[3]} - portal->vertices[0].toRenderSystem();
            if( glm::dot(portal->normal.toRenderSystem(), portalToCam) < 0 )
            {
                return false; // wrong orientation (normals must face the camera)
            }

            int numBehind = 0, numTooFar = 0;
            std::pair<glm::vec3, bool> screen[4];

            gameplay::BoundingBox portalBB{0, 0, 0, 0, 0, 0};
            portalBB.min = {1,1,0};
            portalBB.max = {-1,-1,0};

            for( int i = 0; i < 4; ++i )
            {
                screen[i] = projectOnScreen(portal->vertices[i].toRenderSystem(), camera, numBehind, numTooFar);
                if( !screen[i].second )
                    continue;

                portalBB.min.x = std::min(portalBB.min.x, screen[i].first.x);
                portalBB.min.y = std::min(portalBB.min.y, screen[i].first.y);
                portalBB.max.x = std::max(portalBB.max.x, screen[i].first.x);
                portalBB.max.y = std::max(portalBB.max.y, screen[i].first.y);

                // the first vertex must set the boundingbox to a valid state
                BOOST_ASSERT(portalBB.min.x <= portalBB.max.x);
                BOOST_ASSERT(portalBB.min.y <= portalBB.max.y);
            }

            if( numBehind == 4 || numTooFar == 4 )
                return false;

            BOOST_ASSERT(numBehind <= 3);

            if( numBehind > 0 )
            {
                // patch out-of-bounds coordinates
                auto curr = &screen[0];
                auto prev = &screen[3];
                for( int i = 0; i < 4; ++i , prev = curr++ )
                {
                    // only test edges that cross the camera projection plane
                    if( prev->first.z < 0 == curr->first.z < 0 )
                        continue;

                    if( curr->first.x < -1 && prev->first.x < -1 )
                    {
                        portalBB.min.x = -1;
                    }
                    else if( curr->first.x <= -1 || prev->first.x <= -1 )
                    {
                        portalBB.max.x = 1;
                        portalBB.min.x = -1;
                    }
                    else
                    {
                        portalBB.max.x = 1;
                    }

                    if( curr->first.y < -1 && prev->first.y < -1 )
                    {
                        portalBB.min.y = -1;
                    }
                    else if( curr->first.y <= -1 || prev->first.y <= -1 )
                    {
                        portalBB.max.y = 1;
                        portalBB.min.y = -1;
                    }
                    else
                    {
                        portalBB.max.y = 1;
                    }
                }
            }

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
        static std::pair<glm::vec3, bool> projectOnScreen(glm::vec3 vertex,
                                                          const gameplay::Camera& camera,
                                                          int& numBehind,
                                                          int& numTooFar)
        {
            vertex = glm::vec3(camera.getViewMatrix() * glm::vec4(vertex, 1));
            vertex.z *= -1;

            if( vertex.z <= camera.getNearPlane() )
                ++numBehind;
            else if( vertex.z > camera.getFarPlane() )
                ++numTooFar;

            glm::vec4 tmp{vertex, 1};
            tmp = camera.getProjectionMatrix() * tmp;

            if(std::abs(vertex.z) > camera.getNearPlane()*2)
            {
                glm::vec3 screen{ glm::clamp(tmp.x / tmp.w, -1.0f, 1.0f), glm::clamp(tmp.y / tmp.w, -1.0f, 1.0f), vertex.z };
                return{ screen, vertex.z > camera.getNearPlane() };
            }
            else
            {
                auto sgn = [](float x) -> float { return x < 0 ? -1 : 1; };
                glm::vec3 screen{ sgn(tmp.x), sgn(tmp.y), vertex.z };
                return{ screen, vertex.z > camera.getNearPlane() };
            }
        }
    };
}
