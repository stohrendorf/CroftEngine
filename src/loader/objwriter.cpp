#include "objwriter.h"

#include "datatypes.h"

#include "CImg.h"

#include <boost/algorithm/string.hpp>
#include <boost/range/adaptors.hpp>

#include <algorithm>

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace
{
#pragma pack(push, 1)
    struct VDataNormal
    {
        bool operator==(const VDataNormal& rhs) const
        {
            return color == rhs.color
                   && position == rhs.position
                   && uv == rhs.uv
                   && normal == rhs.normal;
        }


        glm::vec4 color = {0.8f, 0.8f, 0.8f, 1.0f};
        glm::vec3 position;
        glm::vec2 uv;
        glm::vec3 normal;


        static const gameplay::VertexFormat& getFormat()
        {
            static const gameplay::VertexFormat::Element elems[4] = {
                {gameplay::VertexFormat::COLOR, 4},
                {gameplay::VertexFormat::POSITION, 3},
                {gameplay::VertexFormat::TEXCOORD, 2},
                {gameplay::VertexFormat::NORMAL, 3}
            };
            static const gameplay::VertexFormat fmt{elems, 4};

            Expects(fmt.getVertexSize() == sizeof(VDataNormal));

            return fmt;
        }
    };


    struct VData
    {
        bool operator==(const VData& rhs) const
        {
            return color == rhs.color
                   && position == rhs.position
                   && uv == rhs.uv;
        }


        glm::vec4 color = {0.8f, 0.8f, 0.8f, 1.0f};
        glm::vec3 position;
        glm::vec2 uv;


        static const gameplay::VertexFormat& getFormat()
        {
            static const gameplay::VertexFormat::Element elems[3] = {
                {gameplay::VertexFormat::COLOR, 4},
                {gameplay::VertexFormat::POSITION, 3},
                {gameplay::VertexFormat::TEXCOORD, 2}
            };
            static const gameplay::VertexFormat fmt{elems, 3};

            Expects(fmt.getVertexSize() == sizeof(VData));

            return fmt;
        }
    };
#pragma pack(pop)
}


namespace loader
{
    void OBJWriter::write(const std::shared_ptr<gameplay::Image>& srcImg, size_t id) const
    {
        Expects(srcImg != nullptr);

        cimg_library::CImg<float> img(glm::value_ptr(srcImg->getData()[0]), 4, srcImg->getWidth(), srcImg->getHeight(), 1);
        img.permute_axes("yzcx");
        img *= 255;

        auto fullPath = m_basePath / makeTextureName(id);
        fullPath.replace_extension("png");

        img.save_png(fullPath.string().c_str());
    }


    std::shared_ptr<gameplay::Image> OBJWriter::readImage(const boost::filesystem::path& path) const
    {
        {
            auto it = m_imageCache.find(path);
            if( it != m_imageCache.end() )
                return it->second;
        }

        cimg_library::CImg<float> img((m_basePath / path).string().c_str());
        img /= 255;

        const auto w = img.width();
        const auto h = img.height();
        if( img.spectrum() == 3 )
        {
            img.resize(w, h, img.depth(), 4, -1);
            img.channel(3).fill(1);
        }

        if( img.spectrum() != 4 )
        {
            BOOST_THROW_EXCEPTION(std::runtime_error("Can only use RGB and RGBA images"));
        }

        img.permute_axes("cxyz");

        auto result = std::make_shared<gameplay::Image>(w, h, reinterpret_cast<const glm::vec4*>(img.data()));
        m_imageCache[path] = result;
        return result;
    }


    std::shared_ptr<gameplay::Texture> OBJWriter::readTexture(const boost::filesystem::path& path) const
    {
        auto img = readImage(path);
        return std::make_shared<gameplay::Texture>(img);
    }


    OBJWriter::MaterialLibEntry OBJWriter::readMaterial(const boost::filesystem::path& path, const std::shared_ptr<gameplay::ShaderProgram>& shaderProgram) const
    {
        auto texture = readTexture(path);
        auto sampler = std::make_shared<gameplay::Texture::Sampler>(texture);
        sampler->setWrapMode(gameplay::Texture::CLAMP, gameplay::Texture::CLAMP);

        MaterialLibEntry material;
        material.texture = path.string();
        material.material = std::make_shared<gameplay::Material>(shaderProgram);
        material.material->getParameter("u_diffuseTexture")->set(sampler);
        material.material->getParameter("u_worldViewProjectionMatrix")->bindWorldViewProjectionMatrix();
        material.material->initStateBlockDefaults();

        return material;
    }


    std::shared_ptr<gameplay::Model> OBJWriter::readModel(const boost::filesystem::path& path, const std::shared_ptr<gameplay::ShaderProgram>& shaderProgram, const glm::vec3& ambientColor) const
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile((m_basePath / path).string(), aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_ValidateDataStructure);
        BOOST_ASSERT(scene != nullptr);

        auto renderModel = std::make_shared<gameplay::Model>();

        for(unsigned int mi = 0; mi < scene->mNumMeshes; ++mi)
        {
            BOOST_LOG_TRIVIAL(info) << "Converting mesh " << mi + 1 << " of " << scene->mNumMeshes << " from " << m_basePath/path;

            const aiMesh* mesh = scene->mMeshes[mi];
            if(mesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE)
                BOOST_THROW_EXCEPTION(std::runtime_error("Mesh does not consist of triangles only"));
            if(!mesh->HasTextureCoords(0))
                BOOST_THROW_EXCEPTION(std::runtime_error("Mesh does not have UV coordinates"));
            if(mesh->mNumUVComponents[0] != 2)
                BOOST_THROW_EXCEPTION(std::runtime_error("Mesh does not have a 2D UV channel"));
            if(!mesh->HasFaces())
                BOOST_THROW_EXCEPTION(std::runtime_error("Mesh does not have faces"));
            if(!mesh->HasPositions())
                BOOST_THROW_EXCEPTION(std::runtime_error("Mesh does not have positions"));

            std::shared_ptr<gameplay::Mesh> renderMesh;

            if(mesh->HasNormals())
            {
                std::vector<VDataNormal> vbuf(mesh->mNumVertices);
                for(unsigned int i = 0; i < mesh->mNumVertices; ++i)
                {
                    vbuf[i].position = glm::vec3{ mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z } * static_cast<float>(SectorSize);
                    vbuf[i].normal = glm::vec3{ mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
                    vbuf[i].uv = glm::vec2{ mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
                    if(mesh->HasVertexColors(0))
                        vbuf[i].color = glm::vec4(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b, mesh->mColors[0][i].a);
                    else
                        vbuf[i].color = glm::vec4(ambientColor, 1);
                }

                renderMesh = std::make_shared<gameplay::Mesh>(VDataNormal::getFormat(), mesh->mNumVertices, false);
                renderMesh->rebuild(reinterpret_cast<const float*>(vbuf.data()), mesh->mNumVertices);
            }
            else
            {
                std::vector<VData> vbuf(mesh->mNumVertices);
                for(unsigned int i = 0; i < mesh->mNumVertices; ++i)
                {
                    vbuf[i].position = glm::vec3{ mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z } *static_cast<float>(SectorSize);
                    vbuf[i].uv = glm::vec2{ mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
                    if(mesh->HasVertexColors(0))
                        vbuf[i].color = glm::vec4(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b, mesh->mColors[0][i].a);
                    else
                        vbuf[i].color = glm::vec4(ambientColor, 1);
                }

                renderMesh = std::make_shared<gameplay::Mesh>(VData::getFormat(), mesh->mNumVertices, false);
                renderMesh->rebuild(reinterpret_cast<const float*>(vbuf.data()), mesh->mNumVertices);
            }

            std::vector<uint32_t> faces;
            for(const aiFace& face : gsl::span<aiFace>(mesh->mFaces, mesh->mNumFaces))
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
            if(material->GetTexture(aiTextureType_DIFFUSE, 0, &textureName) != aiReturn_SUCCESS)
                BOOST_THROW_EXCEPTION(std::runtime_error("Failed to get diffuse texture path from mesh"));

            part->setMaterial(readMaterial(textureName.C_Str(), shaderProgram).material);

            renderModel->addMesh(renderMesh);
        }

        return renderModel;
    }


    void OBJWriter::write(const std::shared_ptr<gameplay::Model>& model,
                          const std::string& baseName,
                          const std::map<loader::TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap1,
                          const std::map<loader::TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap2,
                          const glm::vec3& ambientColor) const
    {
        Expects(model != nullptr);

        auto fullPath = m_basePath / baseName;
        fullPath.replace_extension("obj");

        aiScene* scene = new aiScene();
        BOOST_ASSERT(scene->mRootNode == nullptr);
        scene->mRootNode = new aiNode();

        {
            size_t totalPartCount = 0;
            for(const auto& mesh : model->getMeshes())
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
            for(size_t i = 0; i < totalPartCount; ++i)
                scene->mRootNode->mMeshes[i] = i;
        }

        for( size_t mi = 0, globalPartIndex = 0; mi<model->getMeshes().size(); ++mi )
        {
            BOOST_ASSERT(mi < scene->mNumMeshes);
            const auto& mesh = model->getMeshes()[mi];

            for(size_t pi = 0; pi < mesh->getPartCount(); ++pi, ++globalPartIndex)
            {
                BOOST_ASSERT(globalPartIndex < scene->mNumMaterials);
                const std::shared_ptr<gameplay::MeshPart>& part = mesh->getPart(pi);

                scene->mMeshes[globalPartIndex] = new aiMesh();
                aiMesh* outMesh = scene->mMeshes[globalPartIndex];

                // init memory
                for(size_t ei = 0; ei < mesh->getVertexFormat().getElementCount(); ++ei)
                {
                    switch(mesh->getVertexFormat().getElement(ei).usage)
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

                // copy vertex data
                {
                    const auto& vfmt = mesh->getVertexFormat();
                    const size_t count = mesh->getVertexCount();
                    const float* data = static_cast<const float*>(mesh->map());
                    for(size_t vi = 0; vi < count; ++vi)
                    {
                        BOOST_ASSERT(vi < outMesh->mNumVertices);
                        for(size_t ei = 0; ei < vfmt.getElementCount(); ++ei)
                        {
                            switch(vfmt.getElement(ei).usage)
                            {
                                case gameplay::VertexFormat::POSITION:
                                    BOOST_ASSERT(outMesh->HasPositions());
                                    outMesh->mVertices[vi].x = data[0] / SectorSize;
                                    outMesh->mVertices[vi].y = data[1] / SectorSize;
                                    outMesh->mVertices[vi].z = data[2] / SectorSize;
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
                    if(texIt != mtlMap1.end())
                    {
                        scene->mMaterials[globalPartIndex]->AddProperty(new aiString(makeTextureName(texIt->first.tileAndFlag & TextureIndexMask) + ".png"), AI_MATKEY_TEXTURE_DIFFUSE(0));
                        found = true;
                    }

                    if(!found)
                    {
                        texIt = std::find_if(mtlMap2.begin(), mtlMap2.end(), finder);
                        if(texIt != mtlMap2.end())
                        {
                            scene->mMaterials[globalPartIndex]->AddProperty(new aiString(makeTextureName(texIt->first.tileAndFlag & TextureIndexMask) + ".png"), AI_MATKEY_TEXTURE_DIFFUSE(0));
                        }
                    }
                }

                outMesh->mNumFaces = part->getIndexCount() / 3;
                outMesh->mFaces = new aiFace[outMesh->mNumFaces];

                switch(part->getIndexFormat())
                {
                    case gameplay::Mesh::INDEX8:
                        {
                            const uint8_t* data = static_cast<const uint8_t*>(part->map());
                            for(size_t fi = 0; fi < part->getIndexCount() / 3; ++fi)
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
                        break;
                    case gameplay::Mesh::INDEX16:
                        {
                            const uint16_t* data = static_cast<const uint16_t*>(part->map());
                            for(size_t fi = 0; fi < part->getIndexCount() / 3; ++fi)
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
                        break;
                    case gameplay::Mesh::INDEX32:
                        {
                            const uint32_t* data = static_cast<const uint32_t*>(part->map());
                            for(size_t fi = 0; fi < part->getIndexCount() / 3; ++fi)
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
                        break;
                    default:
                        break;
                }
            }
        }

        Assimp::Exporter exporter;
        exporter.Export(scene, "obj", fullPath.string(), aiProcess_JoinIdenticalVertices | aiProcess_ValidateDataStructure | aiProcess_FlipUVs);

        delete scene;
    }

    std::string OBJWriter::makeTextureName(size_t id)
    {
        return "texture_" + std::to_string(id);
    }


    bool rayIntersectsTriangle(const glm::vec3& start, const glm::vec3& end,
                               const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2)
    {
        if( start == v0 || start == v1 || start == v2 )
            return false;
        if( end == v0 || end == v1 || end == v2 )
            return false;

        const auto dir = end - start;

        const auto e1 = v1 - v0;
        const auto e2 = v2 - v0;

        const auto p = glm::cross(dir, e2);

        const auto a = glm::dot(e1, p);

        const auto Epsilon = std::numeric_limits<decltype(a)>::epsilon();
        if( glm::abs(a) <= Epsilon )
            return false;

        const auto f = decltype(a)(1) / a;

        const auto s = start - v0;
        const auto x = f * glm::dot(s, p);
        if( x < 0 )
            return false;
        if( x > 1 )
            return false;

        const auto q = glm::cross(s, e1);
        const auto y = f * glm::dot(dir, q);
        if( y < 0 )
            return false;
        if( y + x > 1 )
            return false;

        const auto z = f * glm::dot(e2, q);

        return z >= 0 && z <= 1;
    }
}
