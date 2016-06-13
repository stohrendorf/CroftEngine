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
#include "trcamerascenenodeanimator.h"
#include "defaultanimdispatcher.h"
#include "larastatehandler.h"

#include <algorithm>
#include <stack>
#include <set>
#include <boost/lexical_cast.hpp>

using namespace loader;

namespace
{
const irr::video::SColor WaterColor{0, 149, 229, 229};

class LightSelector final : public irr::scene::ILightManager
{
private:
    loader::Level& m_level;

public:
    explicit LightSelector(loader::Level& level)
        : m_level(level)
    {
    }

    void OnPreRender(irr::core::array<irr::scene::ISceneNode*>& lightList) override
    {
        for(irr::u32 i = 0; i<lightList.size(); ++i)
            lightList[i]->setVisible(false);
        //if(node->getType() != irr::scene::ESNT_ANIMATED_MESH && node->getType() != irr::scene::ESNT_BILLBOARD && node->getType() != irr::scene::ESNT_MESH)
        //    return;

        const auto laraPos = m_level.m_lara->getPosition();
        const auto room = m_level.m_camera->getCurrentRoom();
        int maxFrobbel = 0;
        const loader::Light* bestLight = nullptr;
        for(const loader::Light& light : room->lights)
        {
            auto fadeSq = light.specularFade * light.specularFade / 4096;
            const int frobbel = fadeSq + (0x1fff - room->intensity1) * light.specularIntensity
                / (fadeSq + laraPos.getDistanceFromSQ(light.position.toIrrlicht()) / 4096);
            if(frobbel > maxFrobbel)
            {
                maxFrobbel = frobbel;
                bestLight = &light;
            }
        }
        BOOST_ASSERT(bestLight != nullptr);
        bestLight->node->setVisible(true);
    }

    void OnPostRender() override
    {
        // nop
    }

    void OnRenderPassPreRender(irr::scene::E_SCENE_NODE_RENDER_PASS /*renderPass*/) override
    {
    }

    void OnRenderPassPostRender(irr::scene::E_SCENE_NODE_RENDER_PASS /*renderPass*/) override
    {
    }

    void OnNodePreRender(irr::scene::ISceneNode* /*node*/) override
    {
    }

    void OnNodePostRender(irr::scene::ISceneNode* /*node*/) override
    {
        // nop
    }
};
}

class DoorTriggerHandler final : public AbstractTriggerHandler
{
private:
    uint16_t m_openState = 1;
    uint16_t m_closedState = 0;
    bool m_mustOpen = false;
    bool m_mustClose = false;

public:
    explicit DoorTriggerHandler(const Item& item, const std::shared_ptr<loader::DefaultAnimDispatcher>& dispatcher)
        : AbstractTriggerHandler(item, dispatcher)
    {
    }
    
    virtual void prepare() override
    {
        AbstractTriggerHandler::prepare();

        if(m_mustOpen)
        {
            BOOST_LOG_TRIVIAL(debug) << "Door " << getDispatcher()->getName() << ": swapping opened/closed states";
            update(0);
            std::swap(m_openState, m_closedState);
        }
    }

    virtual void onActivate(AbstractTriggerHandler* /*activator*/) override
    {
        m_mustOpen = true;
        m_mustClose = false;
    }

    virtual void onDeactivate(AbstractTriggerHandler* /*activator*/) override
    {
        m_mustOpen = false;
        m_mustClose = true;
    }

    virtual void onCollide() override {}
    virtual void onStand() override {}
    virtual void onHit() override {}
    virtual void onRoomCollide() override {}
    virtual void update(irr::f32 frameTime) override
    {
        if(getDispatcher()->getCurrentAnimationId() != 1 && getDispatcher()->getCurrentAnimationId() != 3)
        {
            const auto st = getDispatcher()->getCurrentAnimState();
            if(m_mustOpen && m_openState != st)
            {
                BOOST_LOG_TRIVIAL(debug) << "Door " << getDispatcher()->getName() << " opening (timer=" << getTimer() << ")";
                getDispatcher()->setTargetState(m_openState);
                m_mustOpen = false;
            }
            else if(m_mustClose && m_closedState != st)
            {
                BOOST_LOG_TRIVIAL(debug) << "Door " << getDispatcher()->getName() << " closing (timer=" << getTimer() << ")";
                getDispatcher()->setTargetState(m_closedState);
                m_mustClose = false;
            }
        }
    
        if(updateTimer(frameTime) == TimerState::Stopped)
        {
            m_mustOpen = false;
            m_mustClose = true;
        }
    }

    virtual void onSave() override {}
    virtual void onLoad() override {}
};

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

const StaticMesh* Level::findStaticMeshById(uint32_t object_id) const
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

Item* Level::findItemById(int32_t objectId)
{
    for(size_t i = 0; i < m_items.size(); i++)
        if(m_items[i].objectId == objectId)
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

std::vector<irr::video::ITexture*> Level::createTextures(irr::scene::ISceneManager* mgr)
{
    BOOST_ASSERT(!m_textures.empty());
    std::vector<irr::video::ITexture*> textures;
    for(size_t i = 0; i < m_textures.size(); ++i)
    {
        DWordTexture& texture = m_textures[i];
        textures.emplace_back(texture.toTexture(mgr, i));
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

#ifndef NDEBUG
using StateMap = std::map<loader::LaraStateId, std::map<AnimationId, std::map<AnimationId, std::string>>>;

void loadAnim(StateMap& map, AnimationId aid, uint16_t ofs, const Animation& anim, const Level* level)
{
    map[static_cast<loader::LaraStateId>(anim.state_id)][aid][static_cast<AnimationId>(anim.nextAnimation - ofs)] = "@";
    for(size_t i = 0; i < anim.transitionsCount; ++i)
    {
        auto tIdx = anim.transitionsIndex + i;
        BOOST_ASSERT(tIdx < level->m_transitions.size());
        const Transitions& tr = level->m_transitions[tIdx];
        
        for(auto j = tr.firstTransitionCase; j < tr.firstTransitionCase + tr.transitionCaseCount; ++j)
        {
            BOOST_ASSERT(j < level->m_transitionCases.size());
            const TransitionCase& trc = level->m_transitionCases[j];

            AnimationId a = static_cast<AnimationId>(trc.targetAnimation - ofs);
            
            map[static_cast<loader::LaraStateId>(anim.state_id)][aid][a] = {};
        }
    }
}

void dumpAnims(const AnimatedModel& model, const Level* level)
{
    StateMap map;
    for(const auto& fm : model.frameMapping)
    {
        loadAnim(map, static_cast<AnimationId>(fm.first - model.animationIndex), model.animationIndex, level->m_animations[fm.first], level);
    }
    
    std::cerr << "@startuml\n\n";
    std::cerr << "[*] --> Stop\n\n";
    
    for(const auto& state : map)
    {
        const char* st = toString(state.first);
        if(st == nullptr)
        {
            st = "???";
        }
        std::cerr << "state \"" << st << "\" as s" << static_cast<uint16_t>(state.first) << " { \n";
        
        for(const auto& anim : state.second)
        {
            const char* an = toString(anim.first);
            if(an == nullptr)
            {
                an = "???";
            }
            std::cerr << "    state \"" << an << "\" as a" << static_cast<uint16_t>(anim.first) << "\n";
        }
        
        std::cerr << "}\n\n";
    }

    for(const auto& state : map)
    {
        for(const auto& anim : state.second)
        {
            for(const auto& targ : anim.second)
            {
                if(targ.second.empty())
                    std::cerr << "a" << static_cast<uint16_t>(anim.first) << " --> a" << static_cast<uint16_t>(targ.first) << "\n";
                else
                    std::cerr << "a" << static_cast<uint16_t>(anim.first) << " --> a" << static_cast<uint16_t>(targ.first) << " : " << targ.second << "\n";
            }
        }
    }
    
    std::cerr << "@enduml\n";
}

#endif

Level::PlayerInfo Level::createItems(irr::scene::ISceneManager* mgr, const std::vector<irr::scene::ISkinnedMesh*>& skinnedMeshes)
{
    PlayerInfo lara;
    int id = -1;
    for(Item& item : m_items)
    {
        ++id;
        
        BOOST_ASSERT(item.room < m_rooms.size());
        Room& room = m_rooms[item.room];

        auto meshIdx = findAnimatedModelIndexByObjectId(item.objectId);
        if(meshIdx >= 0)
        {
            BOOST_ASSERT(findSpriteSequenceByObjectId(item.objectId) < 0);
            BOOST_ASSERT(static_cast<size_t>(meshIdx) < skinnedMeshes.size());
            irr::scene::IAnimatedMeshSceneNode* node;
            
            if(item.objectId == 0)
            {
                node = mgr->addAnimatedMeshSceneNode(skinnedMeshes[meshIdx], nullptr); // Lara doesn't have a scene graph owner
                node->setPosition(item.position.toIrrlicht());
                lara.node = node;
                lara.room = &room;
            }
            else
            {
                 node = mgr->addAnimatedMeshSceneNode(skinnedMeshes[meshIdx], room.node);
                 node->setPosition(item.position.toIrrlicht() - room.node->getPosition());
            }

            std::string name = "item";
            name += std::to_string(id);
            name += "(object";
            name += std::to_string(item.objectId);
            name += "/animatedModel)";
            node->setName(name.c_str());
                        
            //node->setAutomaticCulling(false);
            node->setRotation({0, util::auToDeg(item.rotation), 0});
            //node->setDebugDataVisible(irr::scene::EDS_SKELETON|irr::scene::EDS_BBOX_ALL|irr::scene::EDS_MESH_WIRE_OVERLAY);
            //node->setDebugDataVisible(irr::scene::EDS_FULL);
            node->setAnimationSpeed(30);
            node->setLoopMode(false);
            auto dispatcher = DefaultAnimDispatcher::create(node, this, *m_animatedModels[meshIdx], name + ":dispatcher");
            
            if(item.objectId == 0)
            {
                dispatcher->playLocalAnimation(static_cast<uint16_t>(AnimationId::STAY_IDLE));
                lara.stateHandler = new LaraStateHandler(this, dispatcher, node, name + ":statehandler");
                node->addAnimator(lara.stateHandler);
                lara.stateHandler->drop();
                node->addShadowVolumeSceneNode();
#ifndef NDEBUG
                dumpAnims(*m_animatedModels[meshIdx], this);
#endif
            }
            
            for(irr::u32 i = 0; i < node->getMaterialCount(); ++i)
            {
                node->getMaterial(i).DiffuseColor = room.lightColor.toSColor(room.intensity1 / 8191.0f);
                node->getMaterial(i).SpecularColor = room.lightColor.toSColor(room.intensity1 / 8191.0f / 4);
                node->getMaterial(i).EmissiveColor.set(0);
            }
            if(item.isInitiallyInvisible())
                node->setVisible(false);
            
            if(item.objectId >= 57 && item.objectId <= 64)
            {
                item.triggerHandler = AbstractTriggerHandler::create<DoorTriggerHandler>(item, dispatcher);
            }
            
            continue;
        }
        
        meshIdx = findSpriteSequenceByObjectId(item.objectId);
        if(meshIdx >= 0)
        {
            BOOST_LOG_TRIVIAL(warning) << "Unimplemented: Item " << id << "/Object " << item.objectId << " is a sprite sequence";
            continue;
        }
        
        BOOST_LOG_TRIVIAL(error) << "No static mesh or animated model for item " << id << "/object " << int(item.objectId);
    }

    return lara;
}

void Level::loadAnimFrame(irr::u32 frameIdx, irr::f32 frameOffset, const AnimatedModel& model, const Animation& animation, irr::scene::ISkinnedMesh* skinnedMesh, const int16_t*& pData, irr::core::aabbox3di& bbox)
{
    BOOST_ASSERT(pData != nullptr);
    uint16_t angleSetOfs = 0x0a;

    for(size_t meshIdx = 0; meshIdx < model.meshCount; meshIdx++)
    {
        auto joint = skinnedMesh->getAllJoints()[meshIdx];
        auto pKey = skinnedMesh->addPositionKey(joint);
        pKey->frame = frameIdx + frameOffset;

        if(meshIdx == 0)
        {
            bbox.MinEdge = {pData[0], pData[2], pData[1]};
            bbox.MaxEdge = {pData[3], pData[5], pData[4]};
            pKey->position.set(pData[6], static_cast<irr::f32>(-pData[7]), pData[8]);
        }
        else
        {
            BOOST_ASSERT(model.boneTreeIndex + 4 * meshIdx <= m_boneTrees.size());
            const int32_t* boneTreeData = &m_boneTrees[model.boneTreeIndex + (meshIdx - 1) * 4];
            pKey->position.set(static_cast<irr::f32>(boneTreeData[1]), static_cast<irr::f32>(-boneTreeData[2]), static_cast<irr::f32>(boneTreeData[3]));
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

AnimatedModel::FrameRange Level::loadAnimation(irr::u32& frameOffset, const AnimatedModel& model, const Animation& animation, irr::scene::ISkinnedMesh* skinnedMesh)
{
    const auto meshPositionIndex = animation.poseDataOffset / 2;
    const int16_t* pData = &m_poseData[meshPositionIndex];
    const int16_t* lastPData = nullptr;

    irr::core::aabbox3di bbox;
    // prepend the first frame
    loadAnimFrame(0, frameOffset, model, animation, skinnedMesh, pData, bbox);
    frameOffset += animation.stretchFactor;

    const auto firstLinearFrame = frameOffset;

    std::map<irr::u32, irr::core::aabbox3di> bboxes;
    pData = &m_poseData[meshPositionIndex];
    for(irr::u32 i = 0; i <= animation.lastFrame - animation.firstFrame; i += animation.stretchFactor)
    {
        lastPData = pData;
        loadAnimFrame(0, frameOffset, model, animation, skinnedMesh, pData, bbox);
        bboxes.insert(std::make_pair(i, bbox));
        frameOffset += animation.stretchFactor;
    }

    irr::u32 framePatch = 0;
    // is necessary, create pseudo-frames, because otherwise irrlicht thinks
    // there's no animation at all
    while(animation.firstFrame >= animation.lastFrame + framePatch)
    {
        pData = lastPData;
        loadAnimFrame(0, frameOffset, model, animation, skinnedMesh, pData, bbox);
        frameOffset += animation.stretchFactor;
        ++framePatch;
    }

    // append the last frame again
    pData = lastPData;
    loadAnimFrame(0, frameOffset, model, animation, skinnedMesh, pData, bbox);
    frameOffset += animation.stretchFactor;

    return AnimatedModel::FrameRange{ firstLinearFrame, animation.firstFrame, animation.lastFrame + framePatch, std::move(bboxes) };
}

std::vector<irr::scene::ISkinnedMesh*> Level::createSkinnedMeshes(irr::scene::ISceneManager* mgr, const std::vector<irr::scene::SMesh*>& staticMeshes)
{
    BOOST_ASSERT(!m_animatedModels.empty());
    
    std::set<uint16_t> animStarts;
    for(const std::unique_ptr<AnimatedModel>& model : m_animatedModels)
    {
        if(model->animationIndex == 0xffff)
        {
            BOOST_LOG_TRIVIAL(warning) << "Model 0x" << std::hex << reinterpret_cast<uintptr_t>(model.get()) << std::dec << " has animationIndex==0xffff";
            continue;
        }
        
        BOOST_ASSERT(animStarts.find(model->animationIndex) == animStarts.end());
        animStarts.insert(model->animationIndex);
    }
    animStarts.insert(m_animations.size());
    
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
            if(model->object_id == 0)
            {
                if(modelMeshIdx == 7)
                    joint->Name = "chest";
                else if(modelMeshIdx == 0)
                    joint->Name = "hips";
            }

            // clone static mesh buffers to skinned mesh buffers
            for(irr::u32 meshBufIdx = 0; meshBufIdx < staticMesh->MeshBuffers.size(); ++meshBufIdx)
            {
                irr::scene::SSkinMeshBuffer* skinMeshBuffer = skinnedMesh->addMeshBuffer();
                BOOST_ASSERT(skinMeshBuffer != nullptr);
                Expects(staticMesh->MeshBuffers[meshBufIdx]->getIndexType() == skinMeshBuffer->getIndexType());
                Expects(staticMesh->MeshBuffers[meshBufIdx]->getVertexType() == skinMeshBuffer->getVertexType());
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

        const auto currentAnimIt = animStarts.find(model->animationIndex);
        
        if(currentAnimIt == animStarts.end())
            continue;

        irr::u32 currentAnimOffset = 0;
        const auto nextAnimIdx = *std::next(currentAnimIt);

        for(auto currentAnimIdx = model->animationIndex; currentAnimIdx < nextAnimIdx; ++currentAnimIdx)
        {
            if(currentAnimIdx >= m_animations.size())
                continue;

            const Animation& animation = m_animations[currentAnimIdx];
            model->frameMapping.emplace( std::make_pair(currentAnimIdx, loadAnimation(currentAnimOffset, *model, animation, skinnedMesh)) );
        }
        
#ifndef NDEBUG
        BOOST_LOG_TRIVIAL(debug) << "Model 0x" << std::hex << reinterpret_cast<uintptr_t>(model.get()) << std::dec << " frame mapping:";
        for(const auto& fm : model->frameMapping)
        {
            BOOST_LOG_TRIVIAL(debug) << "  - anim " << fm.first << ": offset=" << fm.second.offset << ", first=" << fm.second.firstFrame << ", last=" << fm.second.lastFrame;
        }
#endif

        skinnedMesh->finalize();
    }

    return skinnedMeshes;
}

irr::video::ITexture* Level::createSolidColorTex(irr::scene::ISceneManager* mgr, uint8_t color) const
{
    irr::video::SColor pixels[2][2];
    pixels[0][0].set(m_palette->color[color].a, m_palette->color[color].r, m_palette->color[color].g, m_palette->color[color].b);
    pixels[1][0] = pixels[0][0];
    pixels[0][1] = pixels[0][0];
    pixels[1][1] = pixels[0][0];

    auto img = mgr->getVideoDriver()->createImageFromData(irr::video::ECF_A8R8G8B8, {2, 2}, &pixels[0][0]);
    irr::io::path p;
    p = "tex_color";
    p += boost::lexical_cast<std::string>(int(color)).c_str();
    p += ".png";
    auto tex = mgr->getVideoDriver()->addTexture(p, img);

    mgr->getFileSystem()->changeWorkingDirectoryTo("dump");
    mgr->getVideoDriver()->writeImageToFile(img, p);
    mgr->getFileSystem()->changeWorkingDirectoryTo("..");

    img->drop();
    return tex;
}

void Level::toIrrlicht(irr::scene::ISceneManager* mgr, irr::gui::ICursorControl* cursorCtrl)
{
    mgr->getVideoDriver()->setFog(WaterColor, irr::video::EFT_FOG_LINEAR, 1024, 1024 * 32, .003f, true, false);
    mgr->getVideoDriver()->setTextureCreationFlag(irr::video::ETCF_CREATE_MIP_MAPS, false);
    mgr->setLightManager(new LightSelector(*this));

    std::vector<irr::video::ITexture*> textures = createTextures(mgr);
    std::map<UVTexture::TextureKey, irr::video::SMaterial> materials = createMaterials(textures);
    std::vector<irr::video::SMaterial> coloredMaterials;
    for(int i = 0; i < 256; ++i)
    {
        irr::video::SMaterial result;
        // Set some defaults
        result.setTexture(0, createSolidColorTex(mgr, i));
        //result.BackfaceCulling = false;
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
    
    const auto lara = createItems(mgr, skinnedMeshes);
    if(lara.node == nullptr)
        return;
    m_lara = lara.node;

    for(auto* ptr : staticMeshes)
        ptr->drop();
    
    for(auto& ptr : skinnedMeshes)
        ptr->drop();
    
    irr::scene::ICameraSceneNode* camera = mgr->addCameraSceneNode(lara.node, {0, 0, -256}, {0, 0, 0}, -1, true);
    m_camera = new TRCameraSceneNodeAnimator(cursorCtrl, this, lara.room, lara.stateHandler);
    camera->addAnimator(m_camera);
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

const Sector* Level::findSectorForPosition(const TRCoordinates& position, const Room** room) const
{
    BOOST_ASSERT(room != nullptr);
    BOOST_ASSERT(*room != nullptr);

    const Sector* sector = nullptr;
    while(true)
    {
        sector = (*room)->getSectorByClampedIndex((position.X - (*room)->position.X) / SectorSize, (position.Z - (*room)->position.Z) / SectorSize);
        BOOST_ASSERT(sector != nullptr);
        const auto portalTarget = sector->getPortalTarget(m_floorData);
        if(!portalTarget)
        {
            break;
        }

        BOOST_ASSERT(*portalTarget != 0xff && *portalTarget < m_rooms.size());
        *room = &m_rooms[*portalTarget];
    }

    BOOST_ASSERT(sector != nullptr);
    if(sector->floorHeight * QuarterSectorSize > position.Y)
    {
        while(sector->ceilingHeight*QuarterSectorSize > position.Y && sector->roomAbove != 0xff)
        {
            BOOST_ASSERT(sector->roomAbove < m_rooms.size());
            *room = &m_rooms[sector->roomAbove];
            sector = (*room)->getSectorByAbsolutePosition(position);
            BOOST_ASSERT(sector != nullptr);
        }
    }
    else
    {
        while(sector->floorHeight*QuarterSectorSize <= position.Y && sector->roomBelow != 0xff)
        {
            BOOST_ASSERT(sector->roomBelow < m_rooms.size());
            *room = &m_rooms[sector->roomBelow];
            sector = (*room)->getSectorByAbsolutePosition(position);
            BOOST_ASSERT(sector != nullptr);
        }
    }

    return sector;
}

const Room* Level::findRoomForPosition(const TRCoordinates& position, const Room* room) const
{
    BOOST_ASSERT(room != nullptr);

    const Sector* sector = nullptr;
    while(true)
    {
        sector = room->getSectorByClampedIndex((position.X - room->position.X) / SectorSize, (position.Z - room->position.Z) / SectorSize);
        BOOST_ASSERT(sector != nullptr);
        const auto portalTarget = sector->getPortalTarget(m_floorData);
        if(!portalTarget)
        {
            break;
        }

        BOOST_ASSERT(*portalTarget != 0xff && *portalTarget < m_rooms.size());
        room = &m_rooms[*portalTarget];
    }

    BOOST_ASSERT(sector != nullptr);
    if(sector->floorHeight * QuarterSectorSize > position.Y)
    {
        while(sector->ceilingHeight*QuarterSectorSize > position.Y && sector->roomAbove != 0xff)
        {
            BOOST_ASSERT(sector->roomAbove < m_rooms.size());
            room = &m_rooms[sector->roomAbove];
            sector = room->getSectorByAbsolutePosition(position);
            BOOST_ASSERT(sector != nullptr);
        }
    }
    else
    {
        while(sector->floorHeight*QuarterSectorSize <= position.Y && sector->roomBelow != 0xff)
        {
            BOOST_ASSERT(sector->roomBelow < m_rooms.size());
            room = &m_rooms[sector->roomBelow];
            sector = room->getSectorByAbsolutePosition(position);
            BOOST_ASSERT(sector != nullptr);
        }
    }

    return room;
}
