#include "converter.h"

#include "datatypes.h"
#include "engine/items/itemnode.h"

#ifdef _X
#undef _X
#endif

#include "CImg.h"

#include <boost/algorithm/string.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <algorithm>

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>


namespace
{
#pragma pack(push, 1)
    struct RenderVertex
    {
        bool operator==(const RenderVertex& rhs) const
        {
            return color == rhs.color
                   && position == rhs.position
                   && uv == rhs.uv
                   && normal == rhs.normal;
        }


        glm::vec4 color = {0.8f, 0.8f, 0.8f, 1.0f};
        glm::vec3 position;
        glm::vec2 uv;
        glm::vec3 normal{0.0f};


        static const gameplay::VertexFormat& getFormat()
        {
            static const gameplay::VertexFormat::Element elems[4] = {
                {gameplay::VertexFormat::COLOR, 4},
                {gameplay::VertexFormat::POSITION, 3},
                {gameplay::VertexFormat::TEXCOORD, 2},
                {gameplay::VertexFormat::NORMAL, 3}
            };
            static const gameplay::VertexFormat fmt{elems, 4};

            Expects(fmt.getVertexSize() == sizeof(RenderVertex));

            return fmt;
        }
    };
#pragma pack(pop)


    void allocateElementMemory(const std::shared_ptr<gameplay::Mesh>& mesh, const gsl::not_null<aiMesh*>& outMesh)
    {
        for( size_t ei = 0; ei < mesh->getVertexFormat().getElementCount(); ++ei )
        {
            switch( mesh->getVertexFormat().getElement(ei).usage )
            {
                case gameplay::VertexFormat::POSITION:
                    BOOST_ASSERT(outMesh->mVertices == nullptr && outMesh->mNumVertices == 0);
                    outMesh->mNumVertices = mesh->getVertexCount();
                    outMesh->mVertices = new aiVector3D[mesh->getVertexCount()];
                    break;

                case gameplay::VertexFormat::NORMAL:
                    BOOST_ASSERT(outMesh->mNormals == nullptr);
                    outMesh->mNormals = new aiVector3D[mesh->getVertexCount()];
                    break;

                case gameplay::VertexFormat::TEXCOORD:
                    BOOST_ASSERT(outMesh->mTextureCoords[0] == nullptr && outMesh->mNumUVComponents[0] == 0);
                    outMesh->mTextureCoords[0] = new aiVector3D[mesh->getVertexCount()];
                    outMesh->mNumUVComponents[0] = 2;
                    break;

                case gameplay::VertexFormat::COLOR:
                    BOOST_ASSERT(outMesh->mColors[0] == nullptr);
                    outMesh->mColors[0] = new aiColor4D[mesh->getVertexCount()];
                    break;

                case gameplay::VertexFormat::TANGENT:
                    break;

                case gameplay::VertexFormat::BINORMAL:
                    break;
            }
        }
    }


    void copyVertexData(const std::shared_ptr<gameplay::Mesh>& mesh, const gsl::not_null<aiMesh*>& outMesh)
    {
        const auto& vfmt = mesh->getVertexFormat();
        const size_t count = mesh->getVertexCount();
        const float* data = static_cast<const float*>(mesh->map());
        for( size_t vi = 0; vi < count; ++vi )
        {
            BOOST_ASSERT(vi < outMesh->mNumVertices);
            for( size_t ei = 0; ei < vfmt.getElementCount(); ++ei )
            {
                switch( vfmt.getElement(ei).usage )
                {
                    case gameplay::VertexFormat::POSITION:
                        BOOST_ASSERT(outMesh->HasPositions());
                        outMesh->mVertices[vi].x = data[0] / loader::SectorSize;
                        outMesh->mVertices[vi].y = data[1] / loader::SectorSize;
                        outMesh->mVertices[vi].z = data[2] / loader::SectorSize;
                        break;

                    case gameplay::VertexFormat::NORMAL:
                        BOOST_ASSERT(outMesh->HasNormals());
                        outMesh->mNormals[vi].x = data[0];
                        outMesh->mNormals[vi].y = data[1];
                        outMesh->mNormals[vi].z = data[2];
                        break;

                    case gameplay::VertexFormat::TEXCOORD:
                        BOOST_ASSERT(outMesh->HasTextureCoords(0));
                        outMesh->mTextureCoords[0][vi].x = data[0];
                        outMesh->mTextureCoords[0][vi].y = data[1];
                        outMesh->mTextureCoords[0][vi].z = 0;
                        break;

                    case gameplay::VertexFormat::COLOR:
                        BOOST_ASSERT(outMesh->HasVertexColors(0));
                        outMesh->mColors[0][vi].r = data[0];
                        outMesh->mColors[0][vi].g = data[1];
                        outMesh->mColors[0][vi].b = data[2];
                        outMesh->mColors[0][vi].a = data[3];
                        break;

                    case gameplay::VertexFormat::TANGENT:
                        break;

                    case gameplay::VertexFormat::BINORMAL:
                        break;

                    default:
                        break;
                }

                data += vfmt.getElement(ei).size;
            }
        }
        mesh->unmap();
    }


    template<typename T>
    void copyIndices(const std::shared_ptr<gameplay::MeshPart>& part, const gsl::not_null<aiMesh*>& outMesh)
    {
        const T* data = static_cast<const T*>(part->map());
        for( size_t fi = 0; fi < part->getIndexCount() / 3; ++fi )
        {
            BOOST_ASSERT(fi < outMesh->mNumFaces);
            outMesh->mFaces[fi].mNumIndices = 3;
            outMesh->mFaces[fi].mIndices = new unsigned int[3];
            outMesh->mFaces[fi].mIndices[0] = data[3 * fi + 0];
            outMesh->mFaces[fi].mIndices[1] = data[3 * fi + 1];
            outMesh->mFaces[fi].mIndices[2] = data[3 * fi + 2];
        }
        part->unmap();
    }


    template<typename T>
    T& append(T*& array, unsigned int& size, const T& value)
    {
        if( array == nullptr )
        {
            BOOST_ASSERT(size == 0);

            array = new T[1];
            array[0] = value;
            size = 1;
            return array[0];
        }

        BOOST_ASSERT(array != nullptr && size > 0);

        T* tmp = new T[size + 1];
        std::copy_n(array, size, tmp);
        tmp[size] = value;
        std::swap(tmp, array);
        ++size;
        return array[size - 1];
    }


    void convert(aiMatrix4x4& dst, const glm::mat4& src)
    {
        dst.a1 = src[0][0];
        dst.a2 = src[1][0];
        dst.a3 = src[2][0];
        dst.a4 = src[3][0] / loader::SectorSize;
        dst.b1 = src[0][1];
        dst.b2 = src[1][1];
        dst.b3 = src[2][1];
        dst.b4 = src[3][1] / loader::SectorSize;
        dst.c1 = src[0][2];
        dst.c2 = src[1][2];
        dst.c3 = src[2][2];
        dst.c4 = src[3][2] / loader::SectorSize;
        dst.d1 = src[0][3];
        dst.d2 = src[1][3];
        dst.d3 = src[2][3];
        dst.d4 = src[3][3];
    }
}


namespace loader
{
    void Converter::write(const std::shared_ptr<gameplay::Image>& srcImg, size_t id) const
    {
        Expects(srcImg != nullptr);

        cimg_library::CImg<float> img(glm::value_ptr(srcImg->getData()[0]), 4, srcImg->getWidth(), srcImg->getHeight(), 1);
        img.permute_axes("yzcx");
        img *= 255;

        auto fullPath = m_basePath / makeTextureName(id);
        fullPath.replace_extension("png");

        img.save_png(fullPath.string().c_str());
    }


    std::shared_ptr<gameplay::TextureHandle> Converter::readTexture(const boost::filesystem::path& path) const
    {
        {
            auto it = m_textureCache.find(path);
            if( it != m_textureCache.end() )
                return it->second;
        }

        cimg_library::CImg<float> srcImage((m_basePath / path).string().c_str());
        srcImage /= 255;

        const auto w = srcImage.width();
        const auto h = srcImage.height();
        if( srcImage.spectrum() == 3 )
        {
            srcImage.channels(0, 3);
            BOOST_ASSERT(srcImage.spectrum() == 4);
            srcImage.get_shared_channel(3).fill(1);
        }

        if( srcImage.spectrum() != 4 )
        {
            BOOST_THROW_EXCEPTION(std::runtime_error("Can only use RGB and RGBA images"));
        }

        srcImage.permute_axes("cxyz");

        auto image = std::make_shared<gameplay::Image>(w, h, reinterpret_cast<const glm::vec4*>(srcImage.data()));
        auto texture = std::make_shared<gameplay::TextureHandle>(GL_TEXTURE_2D);
        texture->set2D(image->getWidth(), image->getHeight(), image->getData(), true);
        return m_textureCache[path] = texture;
    }


    std::shared_ptr<gameplay::Material> Converter::readMaterial(const boost::filesystem::path& path, const std::shared_ptr<gameplay::ShaderProgram>& shaderProgram) const
    {
        auto texture = readTexture(path);
        texture->set(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        texture->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        auto material = std::make_shared<gameplay::Material>(shaderProgram);
        material->getParameter("u_diffuseTexture")->set(texture);
        material->getParameter("u_worldViewProjectionMatrix")->bindWorldViewProjectionMatrix();
        material->getParameter("u_modelMatrix")->bindModelMatrix();
        material->getParameter("u_baseLight")->bind(&engine::items::ItemNode::lightBaseBinder);
        material->getParameter("u_baseLightDiff")->bind(&engine::items::ItemNode::lightBaseDiffBinder);
        material->getParameter("u_lightPosition")->bind(&engine::items::ItemNode::lightPositionBinder);
        material->initStateBlockDefaults();

        return material;
    }


    std::shared_ptr<gameplay::Model> Converter::readModel(const boost::filesystem::path& path, const std::shared_ptr<gameplay::ShaderProgram>& shaderProgram, const glm::vec3& ambientColor) const
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile((m_basePath / path).string(), aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_ValidateDataStructure | aiProcess_FlipUVs);
        BOOST_ASSERT(scene != nullptr);

        auto renderModel = std::make_shared<gameplay::Model>();

        for( unsigned int mi = 0; mi < scene->mNumMeshes; ++mi )
        {
            BOOST_LOG_TRIVIAL(info) << "Converting mesh " << mi + 1 << " of " << scene->mNumMeshes << " from " << m_basePath / path;

            const aiMesh* mesh = scene->mMeshes[mi];
            if( mesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE )
            BOOST_THROW_EXCEPTION(std::runtime_error("Mesh does not consist of triangles only"));
            if( !mesh->HasTextureCoords(0) )
            BOOST_THROW_EXCEPTION(std::runtime_error("Mesh does not have UV coordinates"));
            if( mesh->mNumUVComponents[0] != 2 )
            BOOST_THROW_EXCEPTION(std::runtime_error("Mesh does not have a 2D UV channel"));
            if( !mesh->HasFaces() )
            BOOST_THROW_EXCEPTION(std::runtime_error("Mesh does not have faces"));
            if( !mesh->HasPositions() )
            BOOST_THROW_EXCEPTION(std::runtime_error("Mesh does not have positions"));

            std::shared_ptr<gameplay::Mesh> renderMesh;

            {
                std::vector<RenderVertex> vbuf(mesh->mNumVertices);
                for( unsigned int i = 0; i < mesh->mNumVertices; ++i )
                {
                    vbuf[i].position = glm::vec3{mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z} * static_cast<float>(SectorSize);
                    if( mesh->HasNormals() )
                        vbuf[i].normal = glm::vec3{mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};
                    vbuf[i].uv = glm::vec2{mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
                    if( mesh->HasVertexColors(0) )
                        vbuf[i].color = glm::vec4(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b, mesh->mColors[0][i].a);
                    else
                        vbuf[i].color = glm::vec4(ambientColor, 1);
                }

                renderMesh = std::make_shared<gameplay::Mesh>(RenderVertex::getFormat(), mesh->mNumVertices, false);
                renderMesh->rebuild(reinterpret_cast<const float*>(vbuf.data()), mesh->mNumVertices);
            }

            std::vector<uint32_t> faces;
            for( const aiFace& face : gsl::span<aiFace>(mesh->mFaces, mesh->mNumFaces) )
            {
                BOOST_ASSERT(face.mNumIndices == 3);
                faces.push_back(face.mIndices[0]);
                faces.push_back(face.mIndices[1]);
                faces.push_back(face.mIndices[2]);
            }

            auto part = renderMesh->addPart(gameplay::Mesh::TRIANGLES, gameplay::Mesh::INDEX32, mesh->mNumFaces * 3, false);
            part->setIndexData(faces.data(), 0, faces.size());

            const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            aiString textureName;
            if( material->GetTexture(aiTextureType_DIFFUSE, 0, &textureName) != aiReturn_SUCCESS )
            BOOST_THROW_EXCEPTION(std::runtime_error("Failed to get diffuse texture path from mesh"));

            part->setMaterial(readMaterial(textureName.C_Str(), shaderProgram));

            renderModel->addMesh(renderMesh);
        }

        return renderModel;
    }


    void Converter::write(const std::shared_ptr<gameplay::Model>& model,
                          const std::string& baseName,
                          const std::map<loader::TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap1,
                          const std::map<loader::TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap2,
                          const glm::vec3& ambientColor) const
    {
        Expects(model != nullptr);

        auto fullPath = m_basePath / baseName;

        Assimp::Exporter exporter;
        std::string formatIdentifier;
        for( size_t i = 0; i < exporter.GetExportFormatCount(); ++i )
        {
            auto descr = exporter.GetExportFormatDescription(i);
            BOOST_ASSERT(descr != nullptr);

            std::string exporterExtension = std::string(".") + descr->fileExtension;

            if( exporterExtension == fullPath.extension().string() )
            {
                formatIdentifier = descr->id;
                break;
            }
        }

        if( formatIdentifier.empty() )
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to find an exporter for the supplied file extension";
            BOOST_LOG_TRIVIAL(info) << "Here's the list of registered exporters";

            for( size_t i = 0; i < exporter.GetExportFormatCount(); ++i )
            {
                auto descr = exporter.GetExportFormatDescription(i);
                BOOST_ASSERT(descr != nullptr);

                BOOST_LOG_TRIVIAL(info) << descr->description << ", extension `" << descr->fileExtension << "`, id `" << descr->id << "`";
            }

            BOOST_THROW_EXCEPTION(std::runtime_error("Failed to find an exporter for the supplied file extension"));
        }

        std::unique_ptr<aiScene> scene = std::make_unique<aiScene>();
        BOOST_ASSERT(scene->mRootNode == nullptr);
        scene->mRootNode = new aiNode();

        {
            size_t totalPartCount = 0;
            for( const auto& mesh : model->getMeshes() )
            {
                totalPartCount += mesh->getPartCount();
            }

            scene->mNumMaterials = totalPartCount;
            scene->mMaterials = new aiMaterial*[totalPartCount];
            std::fill_n(scene->mMaterials, totalPartCount, nullptr);

            scene->mNumMeshes = totalPartCount;
            scene->mMeshes = new aiMesh*[totalPartCount];
            std::fill_n(scene->mMeshes, totalPartCount, nullptr);

            scene->mRootNode->mNumMeshes = totalPartCount;
            scene->mRootNode->mMeshes = new unsigned int[totalPartCount];
            for( size_t i = 0; i < totalPartCount; ++i )
                scene->mRootNode->mMeshes[i] = i;
        }

        for( size_t mi = 0, globalPartIndex = 0; mi < model->getMeshes().size(); ++mi )
        {
            const auto& mesh = model->getMeshes()[mi];

            for( size_t pi = 0; pi < mesh->getPartCount(); ++pi , ++globalPartIndex )
            {
                BOOST_ASSERT(globalPartIndex < scene->mNumMaterials);
                BOOST_ASSERT(globalPartIndex < scene->mNumMeshes);
                const std::shared_ptr<gameplay::MeshPart>& part = mesh->getPart(pi);

                scene->mMeshes[globalPartIndex] = new aiMesh();
                aiMesh* outMesh = scene->mMeshes[globalPartIndex];

                allocateElementMemory(mesh, outMesh);
                copyVertexData(mesh, outMesh);

                BOOST_ASSERT(part->getPrimitiveType() == gameplay::Mesh::PrimitiveType::TRIANGLES && part->getIndexCount() % 3 == 0);
                outMesh->mMaterialIndex = globalPartIndex;
                scene->mMaterials[globalPartIndex] = new aiMaterial();
                scene->mMaterials[globalPartIndex]->AddProperty(new aiColor4D(ambientColor.r, ambientColor.g, ambientColor.b, 1), 1, AI_MATKEY_COLOR_AMBIENT);

                {
                    // try to find the texture for our material

                    using Entry = decltype(*mtlMap1.begin());
                    auto finder = [&part](const Entry& entry)
                        {
                            return entry.second == part->getMaterial();
                        };

                    auto texIt = std::find_if(mtlMap1.begin(), mtlMap1.end(), finder);

                    bool found = false;
                    if( texIt != mtlMap1.end() )
                    {
                        scene->mMaterials[globalPartIndex]->AddProperty(new aiString(makeTextureName(texIt->first.tileAndFlag & TextureIndexMask) + ".png"), AI_MATKEY_TEXTURE_DIFFUSE(0));
                        found = true;
                    }

                    if( !found )
                    {
                        texIt = std::find_if(mtlMap2.begin(), mtlMap2.end(), finder);
                        if( texIt != mtlMap2.end() )
                        {
                            scene->mMaterials[globalPartIndex]->AddProperty(new aiString(makeTextureName(texIt->first.tileAndFlag & TextureIndexMask) + ".png"), AI_MATKEY_TEXTURE_DIFFUSE(0));
                        }
                    }
                }

                outMesh->mNumFaces = part->getIndexCount() / 3;
                outMesh->mFaces = new aiFace[outMesh->mNumFaces];

                switch( part->getIndexFormat() )
                {
                    case gameplay::Mesh::INDEX8:
                        copyIndices<uint8_t>(part, outMesh);
                        break;
                    case gameplay::Mesh::INDEX16:
                        copyIndices<uint16_t>(part, outMesh);
                        break;
                    case gameplay::Mesh::INDEX32:
                        copyIndices<uint32_t>(part, outMesh);
                        break;
                    default:
                        break;
                }

                if(outMesh->mNormals != nullptr && isnan(outMesh->mNormals[0].x))
                {
                    delete[] outMesh->mNormals;
                    outMesh->mNormals = nullptr;
                }
            }
        }

        exporter.Export(scene.get(), formatIdentifier.c_str(), fullPath.string(), aiProcess_JoinIdenticalVertices | aiProcess_ValidateDataStructure | aiProcess_FlipUVs);
    }


    void Converter::write(const std::vector<loader::Room>& rooms,
                          const std::vector<loader::Box>& boxes,
                          const std::string& baseName,
                          const std::map<loader::TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap1,
                          const std::map<loader::TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap2) const
    {
        auto fullPath = m_basePath / baseName;

        Assimp::Exporter exporter;
        std::string formatIdentifier;
        for( size_t i = 0; i < exporter.GetExportFormatCount(); ++i )
        {
            auto descr = exporter.GetExportFormatDescription(i);
            BOOST_ASSERT(descr != nullptr);

            std::string exporterExtension = std::string(".") + descr->fileExtension;

            if( exporterExtension == fullPath.extension().string() )
            {
                formatIdentifier = descr->id;
                break;
            }
        }

        if( formatIdentifier.empty() )
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to find an exporter for the supplied file extension";
            BOOST_LOG_TRIVIAL(info) << "Here's the list of registered exporters";

            for( size_t i = 0; i < exporter.GetExportFormatCount(); ++i )
            {
                auto descr = exporter.GetExportFormatDescription(i);
                BOOST_ASSERT(descr != nullptr);

                BOOST_LOG_TRIVIAL(info) << descr->description << ", extension `" << descr->fileExtension << "`, id `" << descr->id << "`";
            }

            BOOST_THROW_EXCEPTION(std::runtime_error("Failed to find an exporter for the supplied file extension"));
        }

        std::unique_ptr<aiScene> scene = std::make_unique<aiScene>();
        BOOST_ASSERT(scene->mRootNode == nullptr);
        scene->mRootNode = new aiNode();

        for(const auto& room : rooms)
        {
            auto node = convert(*scene, *room.node, mtlMap1, mtlMap2, glm::vec3{ room.getAmbientBrightness() });
            if(node == nullptr)
                continue;

            append(scene->mRootNode->mChildren, scene->mRootNode->mNumChildren, node);
            append(scene->mRootNode->mChildren, scene->mRootNode->mNumChildren, convert(*scene, room.sectors, boxes))->mName = room.node->getId() + ":boxes";

            size_t lightId = 0;
            for(const auto& light : room.lights)
            {
                auto outLight = append(scene->mLights, scene->mNumLights, new aiLight());
                outLight->mName = room.node->getId() + "_light:" + std::to_string(lightId++);
                outLight->mType = aiLightSource_POINT;
                outLight->mColorDiffuse.r = light.getBrightness();
                outLight->mColorDiffuse.g = light.getBrightness();
                outLight->mColorDiffuse.b = light.getBrightness();
                outLight->mColorSpecular = outLight->mColorDiffuse;
                outLight->mColorAmbient = outLight->mColorDiffuse;
                // out = 1 / ( a * d*d )
                // Must be 1/2 at the light radius, so we need to fulfill 2 = a * r*r => a = 2/(r*r)
                const auto r = gsl::narrow_cast<float>(light.radius) / SectorSize;
                outLight->mAttenuationConstant = 0;
                outLight->mAttenuationLinear = 0;
                outLight->mAttenuationQuadratic = 2 / (r*r);

                auto lightNode = append(node->mChildren, node->mNumChildren, new aiNode(outLight->mName.C_Str()));
                const auto p = light.position.toRenderSystem() - room.position.toRenderSystem();
                lightNode->mTransformation.a4 = p.x / SectorSize;
                lightNode->mTransformation.b4 = p.y / SectorSize;
                lightNode->mTransformation.c4 = p.z / SectorSize;
            }
        }

        exporter.Export(scene.get(), formatIdentifier.c_str(), fullPath.string(), aiProcess_JoinIdenticalVertices | aiProcess_ValidateDataStructure | aiProcess_FlipUVs);
    }


    aiNode* Converter::convert(aiScene& scene,
                               const gameplay::Node& sourceNode,
                               const std::map<loader::TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap1,
                               const std::map<loader::TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap2,
                               const glm::vec3& ambientColor) const
    {
        auto outNode = std::make_unique<aiNode>(sourceNode.getId());
        ::convert(outNode->mTransformation, sourceNode.getLocalMatrix());

        bool hasContent = false;
        if( auto sourceModel = std::dynamic_pointer_cast<gameplay::Model>(sourceNode.getDrawable()) )
        {
            convert(scene, *outNode, sourceModel, mtlMap1, mtlMap2, ambientColor);
            hasContent = true;
        }

        for( const auto& child : sourceNode.getChildren() )
        {
            auto subNode = convert(scene, *child, mtlMap1, mtlMap2, ambientColor);
            if( subNode == nullptr )
                continue;

            append(outNode->mChildren, outNode->mNumChildren, subNode);
            hasContent = true;
        }

        if( !hasContent )
            return nullptr;

        return outNode.release();
    }


    void Converter::convert(aiScene& scene,
                            aiNode& outNode,
                            const std::shared_ptr<gameplay::Model>& inModel,
                            const std::map<loader::TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap1,
                            const std::map<loader::TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap2,
                            const glm::vec3& ambientColor) const
    {
        Expects(inModel != nullptr);

        for( const auto& inMesh : inModel->getMeshes() )
        {
            for( size_t pi = 0; pi < inMesh->getPartCount(); ++pi )
            {
                const std::shared_ptr<gameplay::MeshPart>& inPart = inMesh->getPart(pi);

                append(outNode.mMeshes, outNode.mNumMeshes, scene.mNumMeshes);
                auto outMesh = append(scene.mMeshes, scene.mNumMeshes, new aiMesh());

                allocateElementMemory(inMesh, outMesh);
                copyVertexData(inMesh, outMesh);

                BOOST_ASSERT(inPart->getPrimitiveType() == gameplay::Mesh::PrimitiveType::TRIANGLES && inPart->getIndexCount() % 3 == 0);
                outMesh->mMaterialIndex = scene.mNumMaterials;
                auto outMaterial = append(scene.mMaterials, scene.mNumMaterials, new aiMaterial());
                outMaterial->AddProperty(new aiColor4D(ambientColor.r, ambientColor.g, ambientColor.b, 1), 1, AI_MATKEY_COLOR_AMBIENT);

                {
                    // try to find the texture for our material

                    using Entry = decltype(*mtlMap1.begin());
                    auto finder = [&inPart](const Entry& entry)
                        {
                            return entry.second == inPart->getMaterial();
                        };

                    auto texIt = std::find_if(mtlMap1.begin(), mtlMap1.end(), finder);

                    bool found = false;
                    if( texIt != mtlMap1.end() )
                    {
                        outMaterial->AddProperty(new aiString(makeTextureName(texIt->first.tileAndFlag & TextureIndexMask) + ".png"), AI_MATKEY_TEXTURE_DIFFUSE(0));
                        found = true;
                    }

                    if( !found )
                    {
                        texIt = std::find_if(mtlMap2.begin(), mtlMap2.end(), finder);
                        if( texIt != mtlMap2.end() )
                        {
                            outMaterial->AddProperty(new aiString(makeTextureName(texIt->first.tileAndFlag & TextureIndexMask) + ".png"), AI_MATKEY_TEXTURE_DIFFUSE(0));
                        }
                    }
                }

                outMesh->mNumFaces = inPart->getIndexCount() / 3;
                outMesh->mFaces = new aiFace[outMesh->mNumFaces];

                switch( inPart->getIndexFormat() )
                {
                    case gameplay::Mesh::INDEX8:
                        copyIndices<uint8_t>(inPart, outMesh);
                        break;
                    case gameplay::Mesh::INDEX16:
                        copyIndices<uint16_t>(inPart, outMesh);
                        break;
                    case gameplay::Mesh::INDEX32:
                        copyIndices<uint32_t>(inPart, outMesh);
                        break;
                    default:
                        break;
                }

                if(outMesh->mNormals != nullptr && isnan(outMesh->mNormals[0].x))
                {
                    delete[] outMesh->mNormals;
                    outMesh->mNormals = nullptr;
                }
            }
        }
    }


    void Converter::write(const std::string& filename, const YAML::Node& tree) const
    {
        std::ofstream file{(m_basePath / filename).string(), std::ios::trunc};
        file << tree;
    }


    std::string Converter::makeTextureName(size_t id)
    {
        return "texture_" + std::to_string(id);
    }


    aiNode* Converter::convert(aiScene& scene, const std::vector<loader::Sector>& sectors, const std::vector<loader::Box>& boxes) const
    {
        std::unique_ptr<aiNode> outNode = std::make_unique<aiNode>("boxes");

        append(outNode->mMeshes, outNode->mNumMeshes, scene.mNumMeshes);
        auto outMesh = append(scene.mMeshes, scene.mNumMeshes, new aiMesh());

        for(const auto& sector : sectors)
        {
            if(sector.boxIndex == 0xffff)
                continue;

            BOOST_ASSERT(sector.boxIndex < boxes.size());
            const auto& box = boxes[sector.boxIndex];

            const auto firstVertex = outMesh->mNumVertices;

            glm::vec3 v;

            v = core::TRCoordinates(box.xmin, box.floor, box.zmin).toRenderSystem() / float(SectorSize);
            append(outMesh->mVertices, outMesh->mNumVertices, aiVector3D(v.x, v.y, v.z));
            v = core::TRCoordinates(box.xmax, box.floor, box.zmin).toRenderSystem() / float(SectorSize);
            append(outMesh->mVertices, outMesh->mNumVertices, aiVector3D(v.x, v.y, v.z));
            v = core::TRCoordinates(box.xmax, box.floor, box.zmax).toRenderSystem() / float(SectorSize);
            append(outMesh->mVertices, outMesh->mNumVertices, aiVector3D(v.x, v.y, v.z));
            v = core::TRCoordinates(box.xmin, box.floor, box.zmax).toRenderSystem() / float(SectorSize);
            append(outMesh->mVertices, outMesh->mNumVertices, aiVector3D(v.x, v.y, v.z));

            auto& face = append(outMesh->mFaces, outMesh->mNumFaces, aiFace());
            append(face.mIndices, face.mNumIndices, firstVertex + 0);
            append(face.mIndices, face.mNumIndices, firstVertex + 1);
            append(face.mIndices, face.mNumIndices, firstVertex + 2);
            append(face.mIndices, face.mNumIndices, firstVertex + 3);
        }

        return outNode.release();
    }

}
