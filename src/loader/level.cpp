/*
 * Copyright 2002 - Florian Schulze <crow@icculus.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * This file is part of vt.
 *
 */

#include "level.h"
#include "tr1level.h"
#include "tr2level.h"
#include "tr3level.h"
#include "tr4level.h"
#include "tr5level.h"

#include "util/vmath.h"
#include "render/portaltracer.h"

#include <algorithm>
#include <stack>
#include <queue>
#include <set>
#include <boost/lexical_cast.hpp>

using namespace loader;

class TRCameraSceneNodeAnimator final : public irr::scene::ISceneNodeAnimator
{
private:
    bool m_firstUpdate = true;
    irr::gui::ICursorControl* m_cursorControl;
    irr::core::vector2df m_prevCursorPos;
    irr::core::vector2df m_currentCursorPos;
    irr::u32 m_lastAnimationTime = 0;
    bool m_firstInput = true;
    irr::f32 m_rotateSpeed = 100;
    irr::f32 m_rotateSpeedLara = 0.1;
    irr::f32 m_maxVerticalAngle = 88;
    irr::f32 m_moveSpeed = 10;
    bool m_forward = false;
    bool m_backward = false;
    bool m_left = false;
    bool m_right = false;

    const Level* m_level;
    
    const irr::core::vector3df m_relativeTarget{0, 256, 0};
    irr::core::vector3df m_relativePosition{0, 0, -1024};

public:
    explicit TRCameraSceneNodeAnimator(irr::gui::ICursorControl* cursorControl, const Level* level)
        : ISceneNodeAnimator(), m_cursorControl(cursorControl), m_level(level)
    {
        BOOST_ASSERT(cursorControl != nullptr);
    }

    //! Animates a scene node.
    /** \param node Node to animate.
    \param timeMs Current time in milli seconds. */
    virtual void animateNode(irr::scene::ISceneNode* node, irr::u32 timeMs) override
    {
        if(!node || node->getType() != irr::scene::ESNT_CAMERA)
            return;

        irr::scene::ICameraSceneNode* camera = static_cast<irr::scene::ICameraSceneNode*>(node);
        irr::scene::ISceneManager* smgr = node->getSceneManager();
        if(smgr && smgr->getActiveCamera() != camera)
            return;
        
        if(!camera->getParent() || camera->getParent()->getType() != irr::scene::ESNT_ANIMATED_MESH)
            return;
        
        irr::scene::IAnimatedMeshSceneNode* lara = static_cast<irr::scene::IAnimatedMeshSceneNode*>(camera->getParent());
        

        if(m_firstUpdate)
        {
            m_cursorControl->setPosition(0.5f, 0.5f);
            m_currentCursorPos = m_prevCursorPos = m_cursorControl->getRelativePosition();

            m_lastAnimationTime = timeMs;

            m_firstUpdate = false;
        }

        // If the camera isn't the active camera, and receiving input, then don't process it.
        if(!camera->isInputReceiverEnabled())
        {
            m_firstInput = true;
            return;
        }

        if(m_firstInput)
        {
            m_left = m_right = m_forward = m_backward = false;
            m_firstInput = false;
        }

        // get time
        irr::f32 timeDiff = timeMs - m_lastAnimationTime;
        m_lastAnimationTime = timeMs;

        // Update mouse rotation
        if(m_currentCursorPos != m_prevCursorPos)
        {
            auto relativeCameraPosition = m_relativePosition - m_relativeTarget;
            irr::core::vector3df relativeRotation{0,0,0};
            relativeRotation.Y = -(0.5f - m_currentCursorPos.X) * m_rotateSpeed;
            relativeRotation.X = -(0.5f - m_currentCursorPos.Y) * m_rotateSpeed;

            // X < MaxVerticalAngle or X > 360-MaxVerticalAngle

            if(relativeRotation.X > m_maxVerticalAngle * 2 && relativeRotation.X < 360.0f - m_maxVerticalAngle)
            {
                relativeRotation.X = 360.0f - m_maxVerticalAngle;
            }
            else if(relativeRotation.X > m_maxVerticalAngle && relativeRotation.X < 360.0f - m_maxVerticalAngle)
            {
                relativeRotation.X = m_maxVerticalAngle;
            }

            // Do the fix as normal, special case below
            // reset cursor position to the centre of the window.
            m_cursorControl->setPosition(0.5f, 0.5f);

            // needed to avoid problems when the event receiver is disabled
            m_currentCursorPos = m_prevCursorPos = m_cursorControl->getRelativePosition();
            irr::core::matrix4 mat(irr::core::IdentityMatrix);
            mat.setRotationDegrees({relativeRotation.X, relativeRotation.Y, 0});
            mat.transformVect(relativeCameraPosition);
            m_relativePosition = m_relativeTarget + relativeCameraPosition;
        }

        // Special case, mouse is whipped outside of window before it can update.
        irr::video::IVideoDriver* driver = smgr->getVideoDriver();
        irr::core::vector2d<irr::u32> mousepos(m_cursorControl->getPosition().X, m_cursorControl->getPosition().Y);
        irr::core::rect<irr::u32> screenRect(0, 0, driver->getScreenSize().Width, driver->getScreenSize().Height);

        // Only if we are moving outside quickly.
        if(!screenRect.isPointInside(mousepos))
        {
            // Force a reset.
            m_cursorControl->setPosition(0.5f, 0.5f);
            m_currentCursorPos = m_prevCursorPos = m_cursorControl->getRelativePosition();
        }

        irr::core::vector3d<irr::f32> forwardDir = {0,0,1};
        forwardDir.rotateXZBy(-lara->getRotation().Y);

        // update position
        auto laraPos = lara->getPosition();

        if(m_forward)
            laraPos += forwardDir * timeDiff * m_moveSpeed;

        if(m_backward)
            laraPos -= forwardDir * timeDiff * m_moveSpeed;

        // strafing

        irr::core::vector3d<irr::f32> rightDir = {1,0,0};
        rightDir.rotateXZBy(lara->getRotation().Y);

#if 0
        if(m_left)
            laraPos -= rightDir * timeDiff * m_moveSpeed;

        if(m_right)
            laraPos += rightDir * timeDiff * m_moveSpeed;
#else
        auto laraRot = lara->getRotation();
        const auto deltaRot = timeDiff * m_rotateSpeedLara;
        if(m_left)
        {
            laraRot.Y -= deltaRot;
        }

        if(m_right)
        {
            laraRot.Y += deltaRot;
        }
        lara->setRotation(laraRot);
#endif

        // write translation
        lara->setPosition(laraPos);

        lara->updateAbsolutePosition();
        camera->setPosition(m_relativePosition + m_relativeTarget);
        camera->updateAbsolutePosition();
        camera->setTarget(lara->getAbsolutePosition() + m_relativeTarget);
        camera->updateAbsolutePosition();
        
        tracePortals(camera);
    }

    //! Creates a clone of this animator.
    /** Please note that you will have to drop
    (IReferenceCounted::drop()) the returned pointer after calling this. */
    virtual irr::scene::ISceneNodeAnimator* createClone(irr::scene::ISceneNode* /*node*/, irr::scene::ISceneManager* /*newManager*/ = nullptr) override
    {
        BOOST_ASSERT(false);
        return nullptr;
    }

    //! Returns true if this animator receives events.
    /** When attached to an active camera, this animator will be
    able to respond to events such as mouse and keyboard events. */
    virtual bool isEventReceiverEnabled() const override
    {
        return true;
    }

    //! Event receiver, override this function for camera controlling animators
    virtual bool OnEvent(const irr::SEvent& evt) override
    {
        switch(evt.EventType)
        {
            case irr::EET_KEY_INPUT_EVENT:
                switch(evt.KeyInput.Key)
                {
                    case irr::KEY_KEY_A:
                        m_left = evt.KeyInput.PressedDown;
                        return true;
                    case irr::KEY_KEY_D:
                        m_right = evt.KeyInput.PressedDown;
                        return true;
                    case irr::KEY_KEY_W:
                        m_forward = evt.KeyInput.PressedDown;
                        return true;
                    case irr::KEY_KEY_S:
                        m_backward = evt.KeyInput.PressedDown;
                        return true;
                    default:
                        return false;
                }
                break;

            case irr::EET_MOUSE_INPUT_EVENT:
                if(evt.MouseInput.Event == irr::EMIE_MOUSE_MOVED)
                {
                    m_currentCursorPos = m_cursorControl->getRelativePosition();
                    return true;
                }
                break;

            default:
                break;
        }

        return false;
    }
    
private:
    void tracePortals(irr::scene::ICameraSceneNode* camera)
    {
        // Breadth-first queue
        std::queue<render::PortalTracer> toVisit;

        uint16_t startRoom = std::numeric_limits<uint16_t>::max();
        for(size_t i = 0; i < m_level->m_rooms.size(); ++i)
        {
            const Room& room = m_level->m_rooms[i];
            if(room.node->getTransformedBoundingBox().isPointInside(camera->getAbsolutePosition()))
            {
                startRoom = i;
                room.node->setVisible(true);
            }
            else
            {
                room.node->setVisible(false);
            }
        }

        if(startRoom >= m_level->m_rooms.size())
        {
            for(const Room& room : m_level->m_rooms)
                room.node->setVisible(true);
            return;
        }

        // always process direct neighbours
        for(const loader::Portal& portal : m_level->m_rooms[startRoom].portals)
        {
            render::PortalTracer path;
            if(!path.checkVisibility(&portal, camera->getAbsolutePosition(), *camera->getViewFrustum()))
                continue;

            m_level->m_rooms[portal.adjoining_room].node->setVisible(true);

            toVisit.emplace(std::move(path));
        }

        // Avoid infinite loops
        std::set<const loader::Portal*> visited;
        while(!toVisit.empty())
        {
            const render::PortalTracer currentPath = std::move(toVisit.front());
            toVisit.pop();

            if(!visited.insert(currentPath.getLastPortal()).second)
            {
                continue; // already tested
            }

            // iterate through the last room's portals and add the destinations if suitable
            uint16_t destRoom = currentPath.getLastDestinationRoom();
            for(const loader::Portal& srcPortal : m_level->m_rooms[destRoom].portals)
            {
                render::PortalTracer newPath = currentPath;
                if(!newPath.checkVisibility(&srcPortal, camera->getAbsolutePosition(), *camera->getViewFrustum()))
                    continue;

                m_level->m_rooms[srcPortal.adjoining_room].node->setVisible(true);
                toVisit.emplace(std::move(newPath));
            }
        }
    }
};

namespace
{
const irr::video::SColor WaterColor{0, 149, 229, 229};
}

/// \brief reads the mesh data.
void Level::readMeshData(io::SDLReader& reader)
{
    uint32_t meshDataWords = reader.readU32();
    const auto basePos = reader.tell();

    const auto meshDataSize = meshDataWords * 2;
    reader.skip(meshDataSize);

    reader.readVector(m_meshIndices, reader.readU32());
    const auto endPos = reader.tell();

    m_meshes.clear();

    size_t meshDataPos = 0;
    for(size_t i = 0; i < m_meshIndices.size(); i++)
    {
        std::replace(m_meshIndices.begin(), m_meshIndices.end(), meshDataPos, i);

        reader.seek(basePos + meshDataPos);

        if(gameToEngine(m_gameVersion) >= Engine::TR4)
            m_meshes.emplace_back(*Mesh::readTr4(reader));
        else
            m_meshes.emplace_back(*Mesh::readTr1(reader));

        for(size_t j = 0; j < m_meshIndices.size(); j++)
        {
            if(m_meshIndices[j] > meshDataPos)
            {
                meshDataPos = m_meshIndices[j];
                break;
            }
        }
    }

    reader.seek(endPos);
}

/// \brief reads frame and moveable data.
void Level::readPoseDataAndModels(io::SDLReader& reader)
{
    m_poseData.resize(reader.readU32());
    reader.readVector(m_poseData, m_poseData.size());

    m_animatedModels.resize(reader.readU32());
    for(std::unique_ptr<AnimatedModel>& model : m_animatedModels)
    {
        if(gameToEngine(m_gameVersion) < Engine::TR5)
        {
            model = AnimatedModel::readTr1(reader);
            // Disable unused skybox polygons.
            if(gameToEngine(m_gameVersion) == Engine::TR3 && model->object_id == 355)
            {
                m_meshes[m_meshIndices[model->firstMesh]].colored_triangles.resize(16);
            }
        }
        else
        {
            model = AnimatedModel::readTr5(reader);
        }
    }
}

std::unique_ptr<Level> Level::createLoader(const std::string& filename, Game game_version)
{
    size_t len2 = 0;

    for(size_t i = 0; i < filename.length(); i++)
    {
        if(filename[i] == '/' || filename[i] == '\\')
        {
            len2 = i;
        }
    }

    std::string sfxPath;

    if(len2 > 0)
    {
        sfxPath = filename.substr(0, len2 + 1) + "MAIN.SFX";
    }
    else
    {
        sfxPath = "MAIN.SFX";
    }

    io::SDLReader reader(filename);
    if(!reader.isOpen())
        return nullptr;

    if(game_version == Game::Unknown)
        game_version = probeVersion(reader, filename);
    if(game_version == Game::Unknown)
        return nullptr;

    reader.seek(0);
    return createLoader(std::move(reader), game_version, sfxPath);
}

/** \brief reads the level.
  *
  * Takes a SDL_RWop and the game_version of the file and reads the structures into the members of TR_Level.
  */
std::unique_ptr<Level> Level::createLoader(io::SDLReader&& reader, Game game_version, const std::string& sfxPath)
{
    if(!reader.isOpen())
        return nullptr;

    std::unique_ptr<Level> result;

    switch(game_version)
    {
        case Game::TR1:
            result.reset(new TR1Level(game_version, std::move(reader)));
            break;
        case Game::TR1Demo:
        case Game::TR1UnfinishedBusiness:
            result.reset(new TR1Level(game_version, std::move(reader)));
            result->m_demoOrUb = true;
            break;
        case Game::TR2:
            result.reset(new TR2Level(game_version, std::move(reader)));
            break;
        case Game::TR2Demo:
            result.reset(new TR2Level(game_version, std::move(reader)));
            result->m_demoOrUb = true;
            break;
        case Game::TR3:
            result.reset(new TR3Level(game_version, std::move(reader)));
            break;
        case Game::TR4:
        case Game::TR4Demo:
            result.reset(new TR4Level(game_version, std::move(reader)));
            break;
        case Game::TR5:
            result.reset(new TR5Level(game_version, std::move(reader)));
            break;
        default:
            BOOST_THROW_EXCEPTION(std::runtime_error("Invalid game version"));
    }

    result->m_sfxPath = sfxPath;
    return result;
}

Game Level::probeVersion(io::SDLReader& reader, const std::string& filename)
{
    if(!reader.isOpen() || filename.length() < 5)
        return Game::Unknown;

    std::string ext;
    ext += filename[filename.length() - 4];
    ext += toupper(filename[filename.length() - 3]);
    ext += toupper(filename[filename.length() - 2]);
    ext += toupper(filename[filename.length() - 1]);

    reader.seek(0);
    uint8_t check[4];
    reader.readBytes(check, 4);

    Game ret = Game::Unknown;
    if(ext == ".PHD")
    {
        if(check[0] == 0x20 && check[1] == 0x00 && check[2] == 0x00 && check[3] == 0x00)
        {
            ret = Game::TR1;
        }
    }
    else if(ext == ".TUB")
    {
        if(check[0] == 0x20 && check[1] == 0x00 && check[2] == 0x00 && check[3] == 0x00)
        {
            ret = loader::Game::TR1UnfinishedBusiness;
        }
    }
    else if(ext == ".TR2")
    {
        if(check[0] == 0x2D && check[1] == 0x00 && check[2] == 0x00 && check[3] == 0x00)
        {
            ret = loader::Game::TR2;
        }
        else if((check[0] == 0x38 || check[0] == 0x34) && check[1] == 0x00 && (check[2] == 0x18 || check[2] == 0x08) && check[3] == 0xFF)
        {
            ret = loader::Game::TR3;
        }
    }
    else if(ext == ".TR4")
    {
        if(check[0] == 0x54 && // T
           check[1] == 0x52 && // R
           check[2] == 0x34 && // 4
           check[3] == 0x00)
        {
            ret = loader::Game::TR4;
        }
        else if(check[0] == 0x54 && // T
                check[1] == 0x52 && // R
                check[2] == 0x34 && // 4
                check[3] == 0x63)   //
        {
            ret = loader::Game::TR4;
        }
        else if(check[0] == 0xF0 && // T
                check[1] == 0xFF && // R
                check[2] == 0xFF && // 4
                check[3] == 0xFF)
        {
            ret = loader::Game::TR4;
        }
    }
    else if(ext == ".TRC")
    {
        if(check[0] == 0x54 && // T
           check[1] == 0x52 && // R
           check[2] == 0x34 && // C
           check[3] == 0x00)
        {
            ret = loader::Game::TR5;
        }
    }

    return ret;
}

StaticMesh* Level::findStaticMeshById(uint32_t object_id)
{
    for(size_t i = 0; i < m_staticMeshes.size(); i++)
        if(m_staticMeshes[i].object_id == object_id && m_meshIndices[m_staticMeshes[i].mesh])
            return &m_staticMeshes[i];

    return nullptr;
}

int Level::findStaticMeshIndexByObjectId(uint32_t object_id) const
{
    for(size_t i = 0; i < m_staticMeshes.size(); i++)
    {
        if(m_staticMeshes[i].object_id == object_id)
        {
            BOOST_ASSERT(m_staticMeshes[i].mesh < m_meshIndices.size());
            return m_meshIndices[m_staticMeshes[i].mesh];
        }
    }

    return -1;
}

int Level::findAnimatedModelIndexByObjectId(uint32_t object_id) const
{
    for(size_t i = 0; i < m_animatedModels.size(); i++)
        if(m_animatedModels[i]->object_id == object_id)
            return i;

    return -1;
}

int Level::findSpriteSequenceByObjectId(uint32_t object_id) const
{
    for(size_t i = 0; i < m_spriteSequences.size(); i++)
        if(m_spriteSequences[i].object_id == object_id)
            return i;

    return -1;
}

Item* Level::findItemById(int32_t object_id)
{
    for(size_t i = 0; i < m_items.size(); i++)
        if(m_items[i].object_id == object_id)
            return &m_items[i];

    return nullptr;
}

AnimatedModel* Level::findModelById(uint32_t object_id)
{
    for(size_t i = 0; i < m_animatedModels.size(); i++)
        if(m_animatedModels[i]->object_id == object_id)
            return m_animatedModels[i].get();

    return nullptr;
}

std::vector<irr::video::ITexture*> Level::createTextures(irr::video::IVideoDriver* drv)
{
    BOOST_ASSERT(!m_textures.empty());
    std::vector<irr::video::ITexture*> textures;
    for(int i = 0; i < m_textures.size(); ++i)
    {
        DWordTexture& texture = m_textures[i];
        textures.emplace_back(texture.toTexture(drv, i));
    }
    return textures;
}

std::map<UVTexture::TextureKey, irr::video::SMaterial> Level::createMaterials(const std::vector<irr::video::ITexture*>& textures)
{
    const auto texMask = gameToEngine(m_gameVersion) == Engine::TR4 ? TextureIndexMaskTr4 : TextureIndexMask;
    std::map<UVTexture::TextureKey, irr::video::SMaterial> materials;
    for(UVTexture& uvTexture : m_uvTextures)
    {
        const auto& key = uvTexture.textureKey;
        if(materials.find(key) != materials.end())
            continue;

        materials[key] = UVTexture::createMaterial(textures[key.tileAndFlag & texMask], key.blendingMode);
    }
    return materials;
}

class DefaultAnimDispatcher final : public irr::scene::IAnimationEndCallBack
{
private:
    const Level* const m_level;
    const AnimatedModel& m_model;
    uint16_t m_currentAnimation;

public:
    DefaultAnimDispatcher(const Level* level, const AnimatedModel& model, irr::scene::IAnimatedMeshSceneNode* node)
        : m_level(level), m_model(model), m_currentAnimation(model.animationIndex)
    {
        auto it = model.frameMapping.find(m_currentAnimation);
        if(it == model.frameMapping.end())
            return;

        node->setFrameLoop(it->second.firstFrame, it->second.lastFrame);
    }

    virtual void OnAnimationEnd(irr::scene::IAnimatedMeshSceneNode* node) override
    {
        BOOST_ASSERT(m_currentAnimation < m_level->m_animations.size());
        const Animation& currentAnim = m_level->m_animations[m_currentAnimation];
        const auto currentFrame = getCurrentFrame(node);
        for(auto i = 0; i < currentAnim.transitionsCount; ++i)
        {
            auto tIdx = currentAnim.transitionsIndex + i;
            BOOST_ASSERT(tIdx < m_level->m_transitions.size());
            const Transitions& tr = m_level->m_transitions[tIdx];
            if(tr.stateId != getCurrentState())
                continue;

            for(auto j = tr.firstTransitionCase; j < tr.firstTransitionCase + tr.transitionCaseCount; ++j)
            {
                BOOST_ASSERT(j < m_level->m_transitionCases.size());
                const TransitionCase& trc = m_level->m_transitionCases[j];
                if(currentFrame >= trc.firstFrame && currentFrame <= trc.lastFrame)
                {
                    m_currentAnimation = trc.targetAnimation;
                    startAnimLoop(node, trc.targetFrame);
                    return;
                }
            }
        }

        m_currentAnimation = currentAnim.nextAnimation;
        startAnimLoop(node, currentAnim.nextFrame);
    }

private:
    void startAnimLoop(irr::scene::IAnimatedMeshSceneNode* node, irr::u32 frame)
    {
        auto it = m_model.frameMapping.find(m_currentAnimation);
        BOOST_ASSERT(it != m_model.frameMapping.end());

        node->setFrameLoop(it->second.firstFrame, it->second.lastFrame);
        node->setCurrentFrame(it->second.offset + frame);
    }

    irr::u32 getCurrentFrame(irr::scene::IAnimatedMeshSceneNode* node) const
    {
        auto it = m_model.frameMapping.find(m_currentAnimation);
        BOOST_ASSERT(it != m_model.frameMapping.end());

        return node->getFrameNr() - it->second.offset;
    }

    uint16_t getCurrentState() const
    {
        BOOST_ASSERT(m_currentAnimation < m_level->m_animations.size());
        const Animation& currentAnim = m_level->m_animations[m_currentAnimation];
        return currentAnim.state_id;
    }
};

irr::scene::IAnimatedMeshSceneNode* Level::createItems(irr::scene::ISceneManager* mgr, const std::vector<irr::scene::SMesh*>& staticMeshes,
                                                       const std::vector<irr::scene::ISkinnedMesh*>& skinnedMeshes)
{
    irr::scene::IAnimatedMeshSceneNode* lara = nullptr;
    for(const Item& item : m_items)
    {
        BOOST_ASSERT(item.room < m_rooms.size());
        const Room& room = m_rooms[item.room];

#if 0
        int meshIdx = findStaticMeshIndexByObjectId(item.object_id);
        if(meshIdx >= 0)
        {
            BOOST_ASSERT(findSpriteSequenceByObjectId(item.object_id) < 0);
            BOOST_ASSERT(findAnimatedModelIndexByObjectId(item.object_id) < 0);
            BOOST_LOG_TRIVIAL(info) << "Object " << item.object_id << " uses static mesh " << meshIdx;
            BOOST_ASSERT(static_cast<size_t>(meshIdx) < staticMeshes.size());
            irr::scene::IMeshSceneNode* node = mgr->addMeshSceneNode(staticMeshes[meshIdx], room.node);
            node->setPosition(item.position - room.node->getPosition());
            node->setRotation({0, item.rotation, 0});
            for(irr::u32 i = 0; i < node->getMaterialCount(); ++i)
            {
                irr::video::SColor col;
                col.set(room.lightColor.a * 255, room.lightColor.r * 255, room.lightColor.g * 255, room.lightColor.b * 255);
                node->getMaterial(i).AmbientColor = col;
                node->getMaterial(i).DiffuseColor = col;
                node->getMaterial(i).EmissiveColor = col;
                node->getMaterial(i).SpecularColor = col;
                node->getMaterial(i).Lighting = false;
            }
            node->addShadowVolumeSceneNode();
            staticMeshes[meshIdx]->drop();
            continue;
        }
#endif

        auto meshIdx = findAnimatedModelIndexByObjectId(item.object_id);
        if(meshIdx >= 0)
        {
            BOOST_ASSERT(findSpriteSequenceByObjectId(item.object_id) < 0);
            BOOST_LOG_TRIVIAL(info) << "Object " << item.object_id << " uses animated model " << meshIdx;
            BOOST_ASSERT(static_cast<size_t>(meshIdx) < skinnedMeshes.size());
            auto node = mgr->addAnimatedMeshSceneNode(skinnedMeshes[meshIdx], item.object_id == 0 ? nullptr : room.node);
            if(item.object_id == 0)
            {
                lara = node;
                node->setPosition(item.position);
            }
            else
            {
                node->setPosition(item.position - room.node->getPosition());
            }
            node->setAutomaticCulling(false);
            node->setRotation({0, item.rotation, 0});
            // n->setDebugDataVisible(irr::scene::EDS_FULL);
            // node->setDebugDataVisible(irr::scene::EDS_SKELETON|irr::scene::EDS_BBOX_ALL|irr::scene::EDS_MESH_WIRE_OVERLAY);
            node->setAnimationSpeed(30);
            node->setLoopMode(false);
            node->setAnimationEndCallback(new DefaultAnimDispatcher(this, *m_animatedModels[meshIdx], node));
            for(irr::u32 i = 0; i < node->getMaterialCount(); ++i)
            {
                irr::video::SColor col;
                col.set(room.lightColor.a * 255, room.lightColor.r * 255, room.lightColor.g * 255, room.lightColor.b * 255);
                node->getMaterial(i).AmbientColor = col;
                node->getMaterial(i).DiffuseColor = col;
                node->getMaterial(i).EmissiveColor = col;
                node->getMaterial(i).SpecularColor = col;
                node->getMaterial(i).Lighting = false;
            }
            node->addShadowVolumeSceneNode();
            skinnedMeshes[meshIdx]->drop();
            continue;
        }
        
        meshIdx = findSpriteSequenceByObjectId(item.object_id);
        if(meshIdx >= 0)
        {
            BOOST_LOG_TRIVIAL(warning) << "Unimplemented: Object " << item.object_id << " is a sprite sequence";
            continue;
        }
        
        BOOST_LOG_TRIVIAL(error) << "No static mesh or animated model for object id " << int(item.object_id);
    }

    return lara;
}

void Level::loadAnimation(irr::f32 frameOffset, const AnimatedModel& model, const Animation& animation, irr::scene::ISkinnedMesh* skinnedMesh)
{
    const auto meshPositionIndex = animation.poseDataOffset / 2;
    const int16_t* pData = &m_poseData[meshPositionIndex];

    for(irr::u32 frameIdx = animation.firstFrame; frameIdx <= animation.lastFrame; frameIdx += animation.stretchFactor)
    {
        uint16_t angleSetOfs = 0x0a;

        for(size_t k = 0; k < model.meshCount; k++)
        {
            auto joint = skinnedMesh->getAllJoints()[k];
            auto pKey = skinnedMesh->addPositionKey(joint);
            pKey->frame = frameIdx + frameOffset;

            if(k == 0)
            {
                pKey->position = {pData[6], -pData[7], pData[8]};
            }
            else
            {
                BOOST_ASSERT(model.boneTreeIndex + 4 * k <= m_boneTrees.size());
                const int32_t* boneTreeData = &m_boneTrees[model.boneTreeIndex + (k - 1) * 4];
                pKey->position = {boneTreeData[1], -boneTreeData[2], boneTreeData[3]};
            }

            auto rKey = skinnedMesh->addRotationKey(joint);
            rKey->frame = frameIdx + frameOffset;

            auto temp2 = pData[angleSetOfs++];
            auto temp1 = pData[angleSetOfs++];

            irr::core::vector3df rot;
            rot.X = static_cast<irr::f32>((temp1 & 0x3ff0) >> 4);
            rot.Y = -static_cast<irr::f32>(((temp1 & 0x000f) << 6) | ((temp2 & 0xfc00) >> 10));
            rot.Z = static_cast<irr::f32>(temp2 & 0x03ff);
            rot *= 360 / 1024.0;
            // rot *= 2*M_PI / 1024.0;

            rKey->rotation = util::trRotationToQuat(rot);
        }

        pData += animation.poseDataSize;
    }
}

std::vector<irr::scene::ISkinnedMesh*> Level::createSkinnedMeshes(irr::scene::ISceneManager* mgr, const std::vector<irr::scene::SMesh*>& staticMeshes)
{
    std::vector<irr::scene::ISkinnedMesh*> skinnedMeshes;

    for(const std::unique_ptr<AnimatedModel>& model : m_animatedModels)
    {
        BOOST_ASSERT(model != nullptr);
        irr::scene::ISkinnedMesh* skinnedMesh = mgr->createSkinnedMesh();
        skinnedMeshes.emplace_back(skinnedMesh);

        std::stack<irr::scene::ISkinnedMesh::SJoint*> parentStack;

        for(size_t modelMeshIdx = 0; modelMeshIdx < model->meshCount; ++modelMeshIdx)
        {
            BOOST_ASSERT(model->firstMesh + modelMeshIdx < m_meshIndices.size());
            const auto meshIndex = m_meshIndices[model->firstMesh + modelMeshIdx];
            BOOST_ASSERT(meshIndex < staticMeshes.size());
            const auto staticMesh = staticMeshes[meshIndex];
            irr::scene::ISkinnedMesh::SJoint* joint = skinnedMesh->addJoint();

            // clone static mesh buffers to skinned mesh buffers
            for(irr::u32 meshBufIdx = 0; meshBufIdx < staticMesh->MeshBuffers.size(); ++meshBufIdx)
            {
                irr::scene::SSkinMeshBuffer* skinMeshBuffer = skinnedMesh->addMeshBuffer();
                BOOST_ASSERT(skinMeshBuffer != nullptr);
                BOOST_ASSERT(staticMesh->MeshBuffers[meshBufIdx]->getIndexType() == skinMeshBuffer->getIndexType());
                BOOST_ASSERT(staticMesh->MeshBuffers[meshBufIdx]->getVertexType() == skinMeshBuffer->getVertexType());
                for(irr::u32 i = 0; i < staticMesh->MeshBuffers[meshBufIdx]->getIndexCount(); ++i)
                    skinMeshBuffer->Indices.push_back(staticMesh->MeshBuffers[meshBufIdx]->getIndices()[i]);

                for(irr::u32 i = 0; i < staticMesh->MeshBuffers[meshBufIdx]->getVertexCount(); ++i)
                {
                    skinMeshBuffer->Vertices_Standard.push_back(static_cast<irr::video::S3DVertex*>(staticMesh->MeshBuffers[meshBufIdx]->getVertices())[i]);

                    auto w = skinnedMesh->addWeight(joint);
                    w->buffer_id = skinnedMesh->getMeshBuffers().size() - 1;
                    w->strength = 1.0f;
                    w->vertex_id = i;
                }

                skinMeshBuffer->Material = staticMesh->MeshBuffers[meshBufIdx]->getMaterial();

                skinMeshBuffer->setDirty();
                skinMeshBuffer->boundingBoxNeedsRecalculated();
                skinMeshBuffer->recalculateBoundingBox();
            }

            if(modelMeshIdx == 0)
            {
                parentStack.push(joint);
                continue;
            }

            auto pred = skinnedMesh->getAllJoints()[modelMeshIdx - 1];

            irr::scene::ISkinnedMesh::SJoint* parent = nullptr;
            BOOST_ASSERT(model->boneTreeIndex + 4 * modelMeshIdx <= m_boneTrees.size());
            const int32_t* boneTreeData = &m_boneTrees[model->boneTreeIndex + (modelMeshIdx - 1) * 4];

            switch(boneTreeData[0])
            {
                case 0: // use predecessor
                    parent = pred;
                    parent->Children.push_back(joint);
                    break;
                case 2: // push
                    parent = pred;
                    parent->Children.push_back(joint);
                    parentStack.push(parent);
                    break;
                case 1: // pop
                    if(parentStack.empty())
                        throw std::runtime_error("Invalid skeleton stack operation: cannot pop from empty stack");
                    parent = parentStack.top();
                    parent->Children.push_back(joint);
                    parentStack.pop();
                    break;
                case 3: // top
                    if(parentStack.empty())
                        throw std::runtime_error("Invalid skeleton stack operation: cannot take top of empty stack");
                    parent = parentStack.top();
                    parent->Children.push_back(joint);
                    break;
                default:
                    throw std::runtime_error("Invalid skeleton stack operation");
            }
        }

        std::queue<uint16_t> animationsToLoad;
        animationsToLoad.push(model->animationIndex);

        irr::u32 currentAnimOffset = 0;

        while(!animationsToLoad.empty())
        {
            const auto currentAnimIdx = animationsToLoad.front();
            animationsToLoad.pop();
            BOOST_ASSERT(currentAnimIdx < m_animations.size());

            if(model->frameMapping.find(currentAnimIdx) != model->frameMapping.end())
            {
                continue; // already loaded
            }

            const Animation& animation = m_animations[currentAnimIdx];
            loadAnimation(currentAnimOffset, *model, animation, skinnedMesh);

            model->frameMapping.emplace(
                std::make_pair(currentAnimIdx, AnimatedModel::FrameRange(currentAnimOffset, animation.firstFrame, animation.lastFrame)));
            animationsToLoad.push(animation.nextAnimation);

            currentAnimOffset += animation.getKeyframeCount() * animation.stretchFactor;

            for(auto i = 0; i < animation.transitionsCount; ++i)
            {
                auto tIdx = animation.transitionsIndex + i;
                BOOST_ASSERT(tIdx < m_transitions.size());
                const Transitions& tr = m_transitions[tIdx];
                for(auto j = tr.firstTransitionCase; j < tr.firstTransitionCase + tr.transitionCaseCount; ++j)
                {
                    BOOST_ASSERT(j < m_transitionCases.size());
                    const TransitionCase& trc = m_transitionCases[j];
                    animationsToLoad.push(trc.targetAnimation);
                }
            }
        }

        skinnedMesh->finalize();
    }

    return skinnedMeshes;
}

irr::video::ITexture* Level::createSolidColorTex(irr::video::IVideoDriver* drv, uint8_t color)
{
    irr::video::SColor pixels[2][2];
    pixels[0][0].set(m_palette->color[color].a, m_palette->color[color].r, m_palette->color[color].g, m_palette->color[color].b);
    pixels[1][0] = pixels[0][0];
    pixels[0][1] = pixels[0][0];
    pixels[1][1] = pixels[0][0];

    auto img = drv->createImageFromData(irr::video::ECF_A8R8G8B8, {2, 2}, &pixels[0][0]);
    irr::io::path p;
    p = "tex_color";
    p += boost::lexical_cast<std::string>(int(color)).c_str();
    p += ".png";
    auto tex = drv->addTexture(p, img);

    chdir("dump");
    drv->writeImageToFile(img, p);
    chdir("..");

    img->drop();
    return tex;
}

void Level::toIrrlicht(irr::scene::ISceneManager* mgr, irr::gui::ICursorControl* cursorCtrl)
{
    mgr->getVideoDriver()->setFog(WaterColor, irr::video::EFT_FOG_LINEAR, 1024, 1024 * 32, .003f, true, false);

    std::vector<irr::video::ITexture*> textures = createTextures(mgr->getVideoDriver());
    std::map<UVTexture::TextureKey, irr::video::SMaterial> materials = createMaterials(textures);
    std::vector<irr::video::SMaterial> coloredMaterials;
    for(int i = 0; i < 256; ++i)
    {
        irr::video::SMaterial result;
        // Set some defaults
        result.setTexture(0, createSolidColorTex(mgr->getVideoDriver(), i));
        result.BackfaceCulling = false;
        result.ColorMaterial = irr::video::ECM_AMBIENT;
        result.Lighting = true;
        result.AmbientColor.set(0);

        coloredMaterials.emplace_back(result);
    }

    std::vector<irr::scene::SMesh*> staticMeshes;
    for(size_t i = 0; i < m_meshes.size(); ++i)
    {
        staticMeshes.emplace_back(m_meshes[i].createMesh(mgr, i, m_uvTextures, materials, coloredMaterials));
    }

    for(size_t i = 0; i < m_rooms.size(); ++i)
    {
        m_rooms[i].createSceneNode(mgr, i, *this, materials, textures, staticMeshes);
    }

    std::vector<irr::scene::ISkinnedMesh*> skinnedMeshes = createSkinnedMeshes(mgr, staticMeshes);

    const auto lara = createItems(mgr, staticMeshes, skinnedMeshes);
    if(lara == nullptr)
        return;

    irr::scene::ICameraSceneNode* camera = mgr->addCameraSceneNode(lara, {0, 0, -256}, {0, 0, 0}, -1, true);
    camera->addAnimator(new TRCameraSceneNodeAnimator(cursorCtrl, this));
    camera->bindTargetAndRotation(true);
    camera->setNearValue(1);
    camera->setFarValue(2e5);
}

void Level::convertTexture(ByteTexture& tex, Palette& pal, DWordTexture& dst)
{
    for(int y = 0; y < 256; y++)
    {
        for(int x = 0; x < 256; x++)
        {
            int col = tex.pixels[y][x];

            if(col > 0)
                dst.pixels[y][x].set(0xff, pal.color[col].r, pal.color[col].g, pal.color[col].b);
            else
                dst.pixels[y][x].set(0);
        }
    }
}

void Level::convertTexture(WordTexture& tex, DWordTexture& dst)
{
    for(int y = 0; y < 256; y++)
    {
        for(int x = 0; x < 256; x++)
        {
            int col = tex.pixels[y][x];

            if(col & 0x8000)
            {
                const uint32_t a = 0xff;
                const uint32_t r = ((col & 0x00007c00) >> 7);
                const uint32_t g = ((col & 0x000003e0) >> 2);
                const uint32_t b = ((col & 0x0000001f) << 3);
                dst.pixels[y][x].set(a, r, g, b);
            }
            else
            {
                dst.pixels[y][x].set(0);
            }
        }
    }
}
