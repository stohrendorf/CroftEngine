#include "objwriter.h"

#include "datatypes.h"

#include "CImg.h"

#include <boost/algorithm/string.hpp>
#include <boost/range/adaptors.hpp>

#include <algorithm>
#include <fstream>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/DefaultLogger.hpp>

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


    std::map<std::string, OBJWriter::MaterialLibEntry> OBJWriter::readMaterialLib(const boost::filesystem::path& path, const std::shared_ptr<gameplay::ShaderProgram>& shaderProgram) const
    {
        std::ifstream mtl{(m_basePath / path).string(), std::ios::in};
        if( !mtl.is_open() )
        {
            BOOST_THROW_EXCEPTION(std::runtime_error("Cannot open .mtl file"));
        }

        std::map<std::string, MaterialLibEntry> result;

        std::string line, mtlName;
        while( std::getline(mtl, line) )
        {
            boost::algorithm::trim(line);

            if( line.empty() || line[0] == '#' )
                continue;

            if( boost::algorithm::istarts_with(line, "newmtl ") )
            {
                auto space = line.find(' ');
                if( space != std::string::npos )
                {
                    mtlName = line.substr(space);
                    boost::algorithm::trim(mtlName);
                }
                else
                {
                    mtlName.clear();
                }

                if( mtlName.empty() )
                {
                    BOOST_THROW_EXCEPTION(std::runtime_error("Missing material name after newmtl statement"));
                }

                if( result.find(mtlName) != result.end() )
                {
                    BOOST_THROW_EXCEPTION(std::runtime_error("Material already defined"));
                }

                continue;
            }

            if( boost::algorithm::istarts_with(line, "map_kd ") )
            {
                if( mtlName.empty() )
                {
                    BOOST_THROW_EXCEPTION(std::runtime_error("map_kd statement without active material"));
                }

                std::string texFilename;
                auto space = line.find(' ');
                if( space != std::string::npos )
                {
                    texFilename = line.substr(space);
                    boost::algorithm::trim(texFilename);
                }
                else
                {
                    texFilename.clear();
                }

                if( texFilename.empty() )
                {
                    BOOST_THROW_EXCEPTION(std::runtime_error("Missing texture filename after map_kd statement"));
                }

                if( !boost::filesystem::is_regular(m_basePath / texFilename) )
                {
                    BOOST_THROW_EXCEPTION(std::runtime_error("Invalid texture filename specified in map_kd statement"));
                }

                result[mtlName] = readMaterial(texFilename, shaderProgram);
            }
        }

        return result;
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
        std::ofstream objFile{fullPath.string(), std::ios::out | std::ios::trunc};
        if( !objFile.is_open() )
        BOOST_THROW_EXCEPTION(std::runtime_error("Cannot create .obj file"));
        objFile << std::fixed;

        fullPath.replace_extension("mtl");
        std::ofstream mtlFile{fullPath.string(), std::ios::out | std::ios::trunc};
        if( !mtlFile.is_open() )
        BOOST_THROW_EXCEPTION(std::runtime_error("Cannot create .mtl file"));
        mtlFile << std::fixed;

        objFile << "# EdisonEngine Model Dump\n";
        objFile << "mtllib " << baseName << ".mtl\n\n";

        for( const auto& mesh : model->getMeshes() )
        {
            writeMesh(mtlMap1, mtlMap2, objFile, mtlFile, mesh, ambientColor);
        }
    }


    template<typename T>
    void OBJWriter::writeTriFaces(const void* rawIdx, size_t faceCount, size_t vertexCount, bool hasNormals, bool hasTexCoord, std::ostream& obj)
    {
        auto writeFace = [&](uint32_t vidx)
            {
                obj << '-' << (vertexCount - vidx) << "/";
                if( hasTexCoord )
                    obj << '-' << (vertexCount - vidx);
                obj << "/";
                if( hasNormals )
                    obj << '-' << (vertexCount - vidx);
            };

        const T* idx = static_cast<const T*>(rawIdx);
        for( size_t i = 0; i < faceCount; i += 3 , idx += 3 )
        {
            obj << "f ";
            writeFace(idx[0]);
            obj << " ";
            writeFace(idx[1]);
            obj << " ";
            writeFace(idx[2]);
            obj << "\n";
        }
    }


    void OBJWriter::write(const std::shared_ptr<gameplay::Material>& material, std::ostream& mtl, const glm::vec3& color)
    {
        // write some dummy values, as we don't know which texture is bound to the material.
        mtl << "newmtl " << makeMtlName(material) << "\n";
        mtl << "Kd " << color.r << " " << color.g << " " << color.b << "\n\n";
    }


    void OBJWriter::write(const std::shared_ptr<gameplay::Material>& material, size_t textureId, std::ostream& mtl, const glm::vec3& color)
    {
        // write some dummy values, as we don't know which texture is bound to the material.
        mtl << "newmtl " << makeMtlName(material) << "\n";
        mtl << "Kd " << color.r << " " << color.g << " " << color.b << "\n";
        mtl << "map_Kd " << makeTextureName(textureId) << ".png\n";
        mtl << "map_d " << makeTextureName(textureId) << ".png\n\n";
    }


    std::string OBJWriter::makeMtlName(const std::shared_ptr<gameplay::Material>& material)
    {
        return "material_" + std::to_string(reinterpret_cast<uintptr_t>(material.get()));
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


    void OBJWriter::calcColor(glm::vec4& vertexColor, const glm::vec3& vpos, const glm::vec3& ambientColor, const TriList& tris)
    {
        const auto lpos = glm::vec3(2.9953809f, 2.9791765f, -7.5483832f) * static_cast<float>(SectorSize);

        const auto d = glm::distance(lpos, vpos);
        static constexpr float outerRadius = 7 * SectorSize;
        vertexColor = glm::vec4(ambientColor, 0);
        if( d > outerRadius )
            return;

        for( const auto& tri : tris )
        {
            if( rayIntersectsTriangle(lpos, vpos, tri[0], tri[1], tri[2]) )
            {
                return;
            }
        }

        static const glm::vec4 lcolor = glm::vec4(5, .25, .25, 1);
        const auto f = 1 - d / outerRadius;
        vertexColor += lcolor * (f * f);
    }


    template<typename T>
    inline size_t findOrAppend(const T& value, std::vector<T>& buffer)
    {
        auto it = std::find(buffer.begin(), buffer.end(), value);
        if( it != buffer.end() )
            return std::distance(buffer.begin(), it);

        buffer.push_back(value);
        return buffer.size() - 1;
    }


    std::shared_ptr<gameplay::Mesh> OBJWriter::buildMesh(const std::map<std::string, MaterialLibEntry>& mtlLib,
                                                         const std::string& activeMaterial,
                                                         const std::vector<glm::vec2>& uvCoords,
                                                         const std::vector<glm::vec3>& vpos,
                                                         const std::vector<glm::vec3>& vnorm,
                                                         const FaceList& faces,
                                                         const glm::vec3& ambientColor,
                                                         const TriList& tris) const
    {
        BOOST_LOG_TRIVIAL(debug) << "Building mesh...";

        if( vnorm.empty() )
        {
            std::vector<VData> vbuf;
            std::vector<uint32_t> idxbuf;

            for( const auto& face : faces )
            {
                // v/vt/vn
                for( const auto& idx : face )
                {
                    VData vertex;
                    vertex.position = vpos[idx.v];
                    vertex.uv = uvCoords[idx.vt];

                    calcColor(vertex.color, vertex.position, ambientColor, tris);

                    idxbuf.push_back(findOrAppend(vertex, vbuf));
                }
            }

            auto mesh = std::make_shared<gameplay::Mesh>(VData::getFormat(), 0, false);
            mesh->rebuild(reinterpret_cast<float*>(&vbuf[0]), vbuf.size());

            auto part = mesh->addPart(gameplay::Mesh::TRIANGLES, gameplay::Mesh::INDEX32, idxbuf.size(), false);
            part->setIndexData(idxbuf.data(), 0, idxbuf.size());
            const auto mtl = mtlLib.find(activeMaterial);
            BOOST_ASSERT(mtl != mtlLib.end());
            part->setMaterial(mtl->second.material);

            return mesh;
        }
        else
        {
            std::vector<VDataNormal> vbuf;
            std::vector<uint32_t> idxbuf;

            for( const auto& face : faces )
            {
                // v/vt/vn
                for( const auto& idx : face )
                {
                    VDataNormal vertex;
                    vertex.position = vpos[idx.v];
                    vertex.uv = uvCoords[idx.vt];
                    vertex.normal = vnorm[idx.vn];

                    calcColor(vertex.color, vertex.position, ambientColor, tris);

                    idxbuf.push_back(findOrAppend(vertex, vbuf));
                }
            }

            auto mesh = std::make_shared<gameplay::Mesh>(VDataNormal::getFormat(), 0, false);
            mesh->rebuild(reinterpret_cast<float*>(&vbuf[0]), vbuf.size());

            auto part = mesh->addPart(gameplay::Mesh::TRIANGLES, gameplay::Mesh::INDEX32, idxbuf.size(), false);
            part->setIndexData(idxbuf.data(), 0, idxbuf.size());
            const auto mtl = mtlLib.find(activeMaterial);
            BOOST_ASSERT(mtl != mtlLib.end());
            part->setMaterial(mtl->second.material);

            return mesh;
        }
    }


    void OBJWriter::writeMesh(const std::map<loader::TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap1,
                              const std::map<loader::TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap2,
                              std::ofstream& objFile,
                              std::ofstream& mtlFile,
                              const std::shared_ptr<gameplay::Mesh>& mesh,
                              const glm::vec3& ambientColor) const
    {
        objFile << "o mesh_" << reinterpret_cast<uintptr_t>(mesh.get()) << "\n";

        bool hasNormals = false;
        bool hasTexCoord = false;
        {
            const auto& vfmt = mesh->getVertexFormat();
            const size_t count = mesh->getVertexCount();
            const float* data = static_cast<const float*>(mesh->map());
            for( size_t i = 0; i < count; ++i )
            {
                for( size_t j = 0; j < vfmt.getElementCount(); ++j )
                {
                    switch( vfmt.getElement(j).usage )
                    {
                        case gameplay::VertexFormat::POSITION:
                            objFile << "v " << data[0] / SectorSize << " " << data[1] / SectorSize << " " << data[2] / SectorSize << "\n";
                            break;
                        case gameplay::VertexFormat::NORMAL:
                            objFile << "vn " << data[0] << " " << data[1] << " " << data[2] << "\n";
                            hasNormals = true;
                            break;
                        case gameplay::VertexFormat::TEXCOORD:
                            objFile << "vt " << data[0] << " " << 1 - data[1] << "\n";
                            hasTexCoord = true;
                            break;
                        case gameplay::VertexFormat::COLOR:
                        case gameplay::VertexFormat::TANGENT:
                        case gameplay::VertexFormat::BINORMAL:
                            break;
                        default:
                            break;
                    }

                    data += vfmt.getElement(j).size;
                }
            }
            mesh->unmap();
        }

        objFile << "\n";

        {
            for( size_t i = 0; i < mesh->getPartCount(); ++i )
            {
                const std::shared_ptr<gameplay::MeshPart>& part = mesh->getPart(i);
                BOOST_ASSERT(part->getPrimitiveType() == gameplay::Mesh::PrimitiveType::TRIANGLES && part->getIndexCount() % 3 == 0);

                objFile << "\ng part_" << i << "\n";
                objFile << "usemtl " << makeMtlName(part->getMaterial()) << "\n";

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
                        write(part->getMaterial(), texIt->first.tileAndFlag & TextureIndexMask, mtlFile, ambientColor);
                        found = true;
                    }

                    if( !found )
                    {
                        texIt = std::find_if(mtlMap2.begin(), mtlMap2.end(), finder);
                        if( texIt != mtlMap2.end() )
                        {
                            write(part->getMaterial(), texIt->first.tileAndFlag & TextureIndexMask, mtlFile, ambientColor);
                            found = true;
                        }
                    }

                    if( !found )
                    {
                        write(part->getMaterial(), mtlFile, ambientColor);
                    }
                }

                switch( part->getIndexFormat() )
                {
                    case gameplay::Mesh::INDEX8:
                        writeTriFaces<uint8_t>(part->map(), part->getIndexCount(), mesh->getVertexCount(), hasNormals, hasTexCoord, objFile);
                        part->unmap();
                        break;
                    case gameplay::Mesh::INDEX16:
                        writeTriFaces<uint16_t>(part->map(), part->getIndexCount(), mesh->getVertexCount(), hasNormals, hasTexCoord, objFile);
                        part->unmap();
                        break;
                    case gameplay::Mesh::INDEX32:
                        writeTriFaces<uint32_t>(part->map(), part->getIndexCount(), mesh->getVertexCount(), hasNormals, hasTexCoord, objFile);
                        part->unmap();
                        break;
                    default:
                        break;
                }
            }
        }
    }
}
