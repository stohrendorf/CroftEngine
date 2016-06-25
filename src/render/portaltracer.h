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

            const auto& proj = camera.getProjectionMatrix();
            const auto& view = camera.getViewMatrix();

            bool allBehind = true;
            irr::core::rectf portalBB;
            portalBB.UpperLeftCorner = project(portal->vertices[0].toIrrlicht(), view, proj, allBehind);
            portalBB.LowerRightCorner = project(portal->vertices[1].toIrrlicht(), view, proj, allBehind);
            portalBB.repair();
            portalBB.addInternalPoint(project(portal->vertices[2].toIrrlicht(), view, proj, allBehind));
            portalBB.addInternalPoint(project(portal->vertices[3].toIrrlicht(), view, proj, allBehind));

            if( allBehind )
                return false;

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
        static irr::core::position2df project(irr::core::vector3df pos, const irr::core::matrix4& view, const irr::core::matrix4& proj, bool& allBehind)
        {
            view.transformVect(pos);
            if( pos.Z >= 0 )
                allBehind = false;

            // clamp Z value to if too close to the eye
            if( pos.Z < 0.001f )
                pos.Z = 0.001f;

            irr::f32 tmp[4];
            proj.transformVect(tmp, pos);
            irr::core::position2df res{tmp[0] / tmp[3], tmp[1] / tmp[3]};

            return res;
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
