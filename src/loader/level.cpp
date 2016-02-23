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

#include <algorithm>
#include <stack>
#include <queue>
#include <boost/lexical_cast.hpp>

using namespace loader;

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
        if(check[0] == 0x20 &&
           check[1] == 0x00 &&
           check[2] == 0x00 &&
           check[3] == 0x00)
        {
            ret = Game::TR1;
        }
    }
    else if(ext == ".TUB")
    {
        if(check[0] == 0x20 &&
           check[1] == 0x00 &&
           check[2] == 0x00 &&
           check[3] == 0x00)
        {
            ret = loader::Game::TR1UnfinishedBusiness;
        }
    }
    else if(ext == ".TR2")
    {
        if(check[0] == 0x2D &&
           check[1] == 0x00 &&
           check[2] == 0x00 &&
           check[3] == 0x00)
        {
            ret = loader::Game::TR2;
        }
        else if((check[0] == 0x38 || check[0] == 0x34) &&
                check[1] == 0x00 &&
                (check[2] == 0x18 || check[2] == 0x08) &&
                check[3] == 0xFF)
        {
            ret = loader::Game::TR3;
        }
    }
    else if(ext == ".TR4")
    {
        if(check[0] == 0x54 &&                                         // T
           check[1] == 0x52 &&                                         // R
           check[2] == 0x34 &&                                         // 4
           check[3] == 0x00)
        {
            ret = loader::Game::TR4;
        }
        else if(check[0] == 0x54 &&                                         // T
                check[1] == 0x52 &&                                         // R
                check[2] == 0x34 &&                                         // 4
                check[3] == 0x63)                                           //
        {
            ret = loader::Game::TR4;
        }
        else if(check[0] == 0xF0 &&                                         // T
                check[1] == 0xFF &&                                         // R
                check[2] == 0xFF &&                                         // 4
                check[3] == 0xFF)
        {
            ret = loader::Game::TR4;
        }
    }
    else if(ext == ".TRC")
    {
        if(check[0] == 0x54 &&                                              // T
           check[1] == 0x52 &&                                              // R
           check[2] == 0x34 &&                                              // C
           check[3] == 0x00)
        {
            ret = loader::Game::TR5;
        }
    }

    return ret;
}

StaticMesh *Level::findStaticMeshById(uint32_t object_id)
{
    for(size_t i = 0; i < m_staticMeshes.size(); i++)
        if(m_staticMeshes[i].object_id == object_id && m_meshIndices[m_staticMeshes[i].mesh])
            return &m_staticMeshes[i];

    return nullptr;
}

int Level::findStaticMeshIndexByObjectId(uint32_t object_id) const
{
    for(size_t i = 0; i < m_staticMeshes.size(); i++)
        if(m_staticMeshes[i].object_id == object_id)
            return m_meshIndices[m_staticMeshes[i].mesh];
    
    return -1;
}

int Level::findAnimatedModelIndexByObjectId(uint32_t object_id) const
{
    for(size_t i = 0; i < m_animatedModels.size(); i++)
        if(m_animatedModels[i]->object_id == object_id)
            return i;
    
    return -1;
}

Item *Level::findItemById(int32_t object_id)
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
    for(int i=0; i<m_textures.size(); ++i)
    {
        DWordTexture& texture = m_textures[i];
        textures.emplace_back(texture.toTexture(drv, i));
    }
    return textures;
}

std::map<UVTexture::TextureKey, irr::video::SMaterial> Level::createMaterials(const std::vector<irr::video::ITexture*>& textures)
{
    const auto texMask = gameToEngine(m_gameVersion)==Engine::TR4 ? TextureIndexMaskTr4 : TextureIndexMask;
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
        : m_level(level)
        , m_model(model)
        , m_currentAnimation(model.animationIndex)
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
        for(auto i=0; i<currentAnim.transitionsCount; ++i)
        {
            auto tIdx = currentAnim.transitionsIndex + i;
            BOOST_ASSERT(tIdx < m_level->m_transitions.size());
            const Transitions& tr = m_level->m_transitions[tIdx];
            if(tr.stateId != getCurrentState())
                continue;
            
            for(auto j=tr.firstTransitionCase; j<tr.firstTransitionCase+tr.transitionCaseCount; ++j)
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

void Level::createItems(irr::scene::ISceneManager* mgr, const std::vector<irr::scene::SMesh*>& staticMeshes, const std::vector<irr::scene::ISkinnedMesh*>& skinnedMeshes)
{
    for(const Item& item : m_items)
    {
        int meshIdx = findStaticMeshIndexByObjectId(item.object_id);
        if(meshIdx >= 0)
        {
            BOOST_ASSERT(static_cast<size_t>(meshIdx) < staticMeshes.size());
            irr::scene::IMeshSceneNode* n = mgr->addMeshSceneNode(staticMeshes[meshIdx]);
            n->setPosition(item.position);
            n->setRotation({0,item.rotation,0});
            staticMeshes[meshIdx]->drop();
            continue;
        }
        
        meshIdx = findAnimatedModelIndexByObjectId(item.object_id);
        if(meshIdx >= 0)
        {
            BOOST_ASSERT(static_cast<size_t>(meshIdx) < skinnedMeshes.size());
            auto n = mgr->addAnimatedMeshSceneNode(skinnedMeshes[meshIdx]);
            n->setAutomaticCulling(false);
            n->setPosition(item.position);
            n->setRotation({0,item.rotation,0});
            //n->setDebugDataVisible(irr::scene::EDS_FULL);
            n->setDebugDataVisible(irr::scene::EDS_SKELETON|irr::scene::EDS_BBOX_ALL|irr::scene::EDS_MESH_WIRE_OVERLAY);
            n->setAnimationSpeed(30);
            n->setLoopMode(false);
            n->setAnimationEndCallback(new DefaultAnimDispatcher(this, *m_animatedModels[meshIdx], n));
            skinnedMeshes[meshIdx]->drop();
        }
    }
}

void Level::loadAnimation(irr::f32 frameOffset, const AnimatedModel& model, const Animation& animation, irr::scene::ISkinnedMesh* skinnedMesh)
{
    const auto meshPositionIndex = animation.poseDataOffset/2;
    const int16_t* pData = &m_poseData[meshPositionIndex];

    for(irr::u32 frameIdx = animation.firstFrame; frameIdx <= animation.lastFrame; frameIdx += animation.stretchFactor)
    {
        uint16_t angleSetOfs = 0x0a;

        for(size_t k = 0; k < model.meshCount; k++)
        {
            auto joint = skinnedMesh->getAllJoints()[k];
            auto pKey = skinnedMesh->addPositionKey(joint);
            pKey->frame = frameIdx + frameOffset;
            
            if(k==0)
            {
                pKey->position = {pData[6], -pData[7], pData[8]};
            }
            else
            {
                BOOST_ASSERT(model.boneTreeIndex + 4*k <= m_boneTrees.size());
                const int32_t* boneTreeData = &m_boneTrees[ model.boneTreeIndex + (k - 1) * 4 ];
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
            //rot *= 2*M_PI / 1024.0;
            
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
        
        for(size_t modelMeshIdx=0; modelMeshIdx<model->meshCount; ++modelMeshIdx)        
        {
            BOOST_ASSERT(model->firstMesh+modelMeshIdx < m_meshIndices.size());
            const auto meshIndex = m_meshIndices[model->firstMesh + modelMeshIdx];
            BOOST_ASSERT(meshIndex < staticMeshes.size());
            const auto staticMesh = staticMeshes[meshIndex];
            irr::scene::ISkinnedMesh::SJoint* joint = skinnedMesh->addJoint();

            // clone static mesh buffers to skinned mesh buffers
            for(irr::u32 meshBufIdx=0; meshBufIdx<staticMesh->MeshBuffers.size(); ++meshBufIdx)
            {
                irr::scene::SSkinMeshBuffer* skinMeshBuffer = skinnedMesh->addMeshBuffer();
                BOOST_ASSERT(skinMeshBuffer != nullptr);
                BOOST_ASSERT(staticMesh->MeshBuffers[meshBufIdx]->getIndexType() == skinMeshBuffer->getIndexType());
                BOOST_ASSERT(staticMesh->MeshBuffers[meshBufIdx]->getVertexType() == skinMeshBuffer->getVertexType());
                for(irr::u32 i=0; i<staticMesh->MeshBuffers[meshBufIdx]->getIndexCount(); ++i)
                    skinMeshBuffer->Indices.push_back( staticMesh->MeshBuffers[meshBufIdx]->getIndices()[i] );
                
                for(irr::u32 i = 0; i < staticMesh->MeshBuffers[meshBufIdx]->getVertexCount(); ++i)
                {
                    skinMeshBuffer->Vertices_Standard.push_back( static_cast<irr::video::S3DVertex*>(staticMesh->MeshBuffers[meshBufIdx]->getVertices())[i] );
                    
                    auto w = skinnedMesh->addWeight(joint);
                    w->buffer_id = skinnedMesh->getMeshBuffers().size()-1;
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
                parentStack.push( joint );
                continue;
            }
    
            auto pred = skinnedMesh->getAllJoints()[modelMeshIdx - 1];
            
            irr::scene::ISkinnedMesh::SJoint* parent = nullptr;
            BOOST_ASSERT(model->boneTreeIndex + 4*modelMeshIdx <= m_boneTrees.size());
            const int32_t* boneTreeData = &m_boneTrees[ model->boneTreeIndex + (modelMeshIdx - 1) * 4 ];
            
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
            
            model->frameMapping.emplace(std::make_pair(currentAnimIdx, AnimatedModel::FrameRange(currentAnimOffset, animation.firstFrame, animation.lastFrame)));
            animationsToLoad.push(animation.nextAnimation);
            
            currentAnimOffset += animation.getKeyframeCount() * animation.stretchFactor;
            
            for(auto i=0; i<animation.transitionsCount; ++i)
            {
                auto tIdx = animation.transitionsIndex + i;
                BOOST_ASSERT(tIdx < m_transitions.size());
                const Transitions& tr = m_transitions[tIdx];
                for(auto j=tr.firstTransitionCase; j<tr.firstTransitionCase+tr.transitionCaseCount; ++j)
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

    auto img = drv->createImageFromData(
                   irr::video::ECF_A8R8G8B8,
                   {2, 2},
                   &pixels[0][0]);
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

void Level::toIrrlicht(irr::scene::ISceneManager* mgr)
{
    std::vector<irr::video::ITexture*> textures = createTextures(mgr->getVideoDriver());
    std::map<UVTexture::TextureKey, irr::video::SMaterial> materials = createMaterials(textures);
    std::vector<irr::video::SMaterial> coloredMaterials;
    for(int i=0; i<256; ++i)
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
    for(size_t i=0; i<m_meshes.size(); ++i)
    {
        staticMeshes.emplace_back(m_meshes[i].createMesh(mgr, i, m_uvTextures, materials, coloredMaterials));
    }

    irr::core::vector3df cpos;
    for(size_t i=0; i<m_rooms.size(); ++i)
    {
        irr::scene::IMeshSceneNode* n = m_rooms[i].createSceneNode(mgr, i, *this, materials, textures, staticMeshes);
        if(i==0)
            cpos = n->getAbsolutePosition();
    }

    std::vector<irr::scene::ISkinnedMesh*> skinnedMeshes = createSkinnedMeshes(mgr, staticMeshes);
      
    createItems(mgr, staticMeshes, skinnedMeshes);

    irr::SKeyMap keyMap[7];
    keyMap[0].Action = irr::EKA_MOVE_FORWARD;
    keyMap[0].KeyCode = irr::KEY_KEY_W;

    keyMap[1].Action = irr::EKA_MOVE_BACKWARD;
    keyMap[1].KeyCode = irr::KEY_KEY_S;

    keyMap[2].Action = irr::EKA_STRAFE_LEFT;
    keyMap[2].KeyCode = irr::KEY_KEY_A;

    keyMap[3].Action = irr::EKA_STRAFE_RIGHT;
    keyMap[3].KeyCode = irr::KEY_KEY_D;

    keyMap[4].Action = irr::EKA_JUMP_UP;
    keyMap[4].KeyCode = irr::KEY_SPACE;

    keyMap[5].Action = irr::EKA_CROUCH;
    keyMap[5].KeyCode = irr::KEY_SHIFT;
    keyMap[6].Action = irr::EKA_CROUCH;
    keyMap[6].KeyCode = irr::KEY_CONTROL;

    irr::scene::ICameraSceneNode* camera = mgr->addCameraSceneNodeFPS(nullptr, 50.f, 10.0f, -1, keyMap, 7, false, 10.0f);
    camera->setNearValue(1);
    camera->setFarValue(2e5);
    
    camera->setPosition(cpos);
}

void Level::convertTexture(ByteTexture & tex, Palette & pal, DWordTexture & dst)
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

void Level::convertTexture(WordTexture & tex, DWordTexture & dst)
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
                dst.pixels[y][x].set(a,r,g,b);
            }
            else
            {
                dst.pixels[y][x].set(0);
            }
        }
    }
}
