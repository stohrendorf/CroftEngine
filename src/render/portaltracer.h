#pragma once

#include "loader/datatypes.h"

namespace render
{
    struct PortalTracer
    {
        irr::core::rectf boundingBox{-1, -1, 1, 1};
        const loader::Portal* lastPortal = nullptr;

        bool checkVisibility(const loader::Portal* portal, const irr::scene::ICameraSceneNode& camera, irr::video::IVideoDriver* drv)
        {
            if( portal->normal.toIrrlicht().dotProduct(portal->vertices[0].toIrrlicht() - camera.getAbsolutePosition()) >= 0 )
            {
                return false; // wrong orientation (normals must face the camera)
            }

            int numBehind = 0, numTooFar = 0;
            std::pair<irr::core::vector3df, bool> screen[4];

            irr::core::rectf portalBB;
            portalBB.UpperLeftCorner = irr::core::vector2df{ 1,1 };
            portalBB.LowerRightCorner = irr::core::vector2df{ -1,-1 };
            {
                for(int i = 0; i < 4; ++i)
                {
                    screen[i] = projectOnScreen(portal->vertices[i].toIrrlicht(), camera, numBehind, numTooFar);
                    if(!screen[i].second)
                        continue;

                    portalBB.addInternalPoint(screen[i].first.X, screen[i].first.Y);
                }
            }

            if(numBehind == 4 || numTooFar == 4)
                return false;

            if(numBehind == 0)
            {
                boundingBox.clipAgainst(portalBB);
                lastPortal = portal;

                drawBB(drv, portalBB, irr::video::SColor(255, 0, 255, 0));
                drawBB(drv, boundingBox, irr::video::SColor(255, 0, 0, 255));

                return boundingBox.getArea() * drv->getScreenSize().getArea() >= 1;
            }

            BOOST_ASSERT(numBehind >= 1 && numBehind <= 3);
            const irr::core::vector3df* prev = &screen[3].first;
            for(int i = 0; i < 4; ++i)
            {
                const irr::core::vector3df* curr = &screen[i].first;

                if(prev->Z < 0 == curr->Z < 0)
                {
                    prev = curr;
                    continue;
                }

                if(prev->X <= 0 || curr->X <= 0)
                {
                    portalBB.LowerRightCorner.X = 1;
                    if(prev->X >= 0 || curr->X >= 0)
                    {
                        portalBB.UpperLeftCorner.X = -1;
                    }
                }
                else
                {
                    portalBB.UpperLeftCorner.X = -1;
                }

                if(prev->Y <= 0 || curr->Y <= 0)
                {
                    portalBB.LowerRightCorner.Y = 1;
                    if(prev->Y >= 0 || curr->Y >= 0)
                    {
                        portalBB.UpperLeftCorner.Y = -1;
                    }
                }
                else
                {
                    portalBB.UpperLeftCorner.Y = -1;
                }

                prev = curr;
            }

            portalBB.repair();

            boundingBox.clipAgainst(portalBB);
            lastPortal = portal;

            drawBB(drv, portalBB, irr::video::SColor(255, 0, 255, 0));
            drawBB(drv, boundingBox, irr::video::SColor(255, 0, 0, 255));

            return boundingBox.getArea() * drv->getScreenSize().getArea() >= 1;
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
        static std::pair<irr::core::vector3df, bool> projectOnScreen(irr::core::vector3df vertex,
                                                                     const irr::scene::ICameraSceneNode& camera,
                                                                     int& numBehind,
                                                                     int& numTooFar)
        {
            camera.getViewMatrix().transformVect(vertex);
            if(vertex.Z <= camera.getNearValue())
                ++numBehind;
            else if(vertex.Z > camera.getFarValue())
                ++numTooFar;

            irr::f32 tmp[4];
            camera.getProjectionMatrix().transformVect(tmp, vertex);

            auto signToLimits = [](float f) -> float
            {
                return f <= 0 ? -1 : 1;
            };

            if(std::abs(vertex.Z) <= camera.getNearValue())
                return{ {signToLimits(vertex.X), signToLimits(vertex.Y), vertex.Z}, false };

            irr::core::vector3df screen{tmp[0] / tmp[3], tmp[1] / tmp[3], vertex.Z};
            return{ screen, vertex.Z > camera.getNearValue() };
        }

        static void drawBB(irr::video::IVideoDriver* drv, const irr::core::rectf& bb, const irr::video::SColor& col)
        {
            const auto w = drv->getScreenSize().Width;
            const auto h = drv->getScreenSize().Height;
            // top
            drawBBLine(drv, w, h, {bb.UpperLeftCorner.X, bb.UpperLeftCorner.Y}, {bb.LowerRightCorner.X, bb.UpperLeftCorner.Y}, col);
            // bottom
            drawBBLine(drv, w, h, {bb.UpperLeftCorner.X, bb.LowerRightCorner.Y}, {bb.LowerRightCorner.X, bb.LowerRightCorner.Y}, col);
            // left
            drawBBLine(drv, w, h, {bb.UpperLeftCorner.X, bb.UpperLeftCorner.Y}, {bb.UpperLeftCorner.X, bb.LowerRightCorner.Y}, col);
            // right
            drawBBLine(drv, w, h, {bb.LowerRightCorner.X, bb.UpperLeftCorner.Y}, {bb.LowerRightCorner.X, bb.LowerRightCorner.Y}, col);

            drawBBLine(drv, w, h, {bb.UpperLeftCorner.X, bb.UpperLeftCorner.Y}, {bb.LowerRightCorner.X, bb.LowerRightCorner.Y}, col);
            drawBBLine(drv, w, h, {bb.UpperLeftCorner.X, bb.LowerRightCorner.Y}, {bb.LowerRightCorner.X, bb.UpperLeftCorner.Y}, col);
        }

        static void drawBBLine(irr::video::IVideoDriver* drv, int w, int h, const irr::core::vector2df& a, const irr::core::vector2df& b, const irr::video::SColor& col)
        {
            const auto a_ = irr::core::dimension2di(w * (a.X + 1) / 2, h - h * (a.Y + 1) / 2);
            const auto b_ = irr::core::dimension2di(w * (b.X + 1) / 2, h - h * (b.Y + 1) / 2);
            drv->draw2DLine(a_, b_, col);
        }
    };
}
