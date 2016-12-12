#pragma once

#include "gameplay.h"

#include "CImg.h"

#include "gsl/gsl"

#include <glm/gtc/type_ptr.hpp>

#include <boost/filesystem.hpp>
#include <fstream>
#include <boost/algorithm/string.hpp>


namespace loader
{
    class OBJWriter
    {
    public:
        explicit OBJWriter(const boost::filesystem::path& basePath)
            : m_basePath{basePath}
        {
            boost::filesystem::create_directories(m_basePath);
        }


        void write(const std::shared_ptr<gameplay::Image>& srcImg, size_t id) const
        {
            Expects(srcImg != nullptr);

            cimg_library::CImg<float> img(glm::value_ptr(srcImg->getData()[0]), 4, srcImg->getWidth(), srcImg->getHeight(), 1);
            img.permute_axes("yzcx");
            img *= 255;

            auto fullPath = m_basePath / makeTextureName(id);
            fullPath.replace_extension("png");

            img.save_png(fullPath.string().c_str());
        }


        std::shared_ptr<gameplay::Image> readImage(const boost::filesystem::path& path) const
        {
            auto fullPath = m_basePath / path;
            cimg_library::CImg<float> img(fullPath.string().c_str());
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

            return std::make_shared<gameplay::Image>(w, h, reinterpret_cast<const glm::vec4*>(img.data()));
        }


        std::shared_ptr<gameplay::Texture> readTexture(const boost::filesystem::path& path) const
        {
            auto img = readImage(path);
            return std::make_shared<gameplay::Texture>(img);
        }


        std::shared_ptr<gameplay::Material> readMaterial(const boost::filesystem::path& path, const std::shared_ptr<gameplay::ShaderProgram>& shaderProgram) const
        {
            auto texture = readTexture(path);
            auto sampler = std::make_shared<gameplay::Texture::Sampler>(texture);
            sampler->setWrapMode(gameplay::Texture::CLAMP, gameplay::Texture::CLAMP);

            auto material = std::make_shared<gameplay::Material>(shaderProgram);
            material->getParameter("u_diffuseTexture")->set(sampler);
            material->getParameter("u_worldViewProjectionMatrix")->bindWorldViewProjectionMatrix();
            material->initStateBlockDefaults();

            return material;
        }


        std::map<std::string, std::shared_ptr<gameplay::Material>> readMaterialLib(const boost::filesystem::path& path, const std::shared_ptr<gameplay::ShaderProgram>& shaderProgram) const
        {
            std::ifstream mtl{(m_basePath / path).string(), std::ios::in};
            if( !mtl.is_open() )
            {
                BOOST_THROW_EXCEPTION(std::runtime_error("Cannot open .mtl file"));
            }

            std::map<std::string, std::shared_ptr<gameplay::Material>> result;

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

        bool exists(const boost::filesystem::path& path)
        {
            return boost::filesystem::is_regular_file(m_basePath / path);
        }

        std::vector<std::shared_ptr<gameplay::Model>> readModels(const boost::filesystem::path& path, const std::shared_ptr<gameplay::ShaderProgram>& shaderProgram) const
        {
            std::ifstream obj{(m_basePath / path).string(), std::ios::in};
            if( !obj.is_open() )
            {
                BOOST_THROW_EXCEPTION(std::runtime_error("Cannot open .obj file"));
            }

            std::map<std::string, std::shared_ptr<gameplay::Material>> mtlLib;
            std::string activeMaterial;

            std::vector<glm::vec2> uvCoords;
            std::vector<glm::vec3> vpos;
            std::vector<glm::vec3> vnorm;
            std::vector<std::array<std::array<int, 3>, 3>> faces;

            std::vector<std::shared_ptr<gameplay::Model>> result;

            std::string line;
            while( std::getline(obj, line) )
            {
                boost::algorithm::trim(line);

                if( line.empty() || line[0] == '#' )
                    continue;

                if( boost::algorithm::istarts_with(line, "mtllib ") )
                {
                    auto space = line.find(' ');
                    std::string mtlLibName;
                    if( space != std::string::npos )
                    {
                        mtlLibName = line.substr(space);
                        boost::algorithm::trim(mtlLibName);
                    }
                    else
                    {
                        mtlLibName.clear();
                    }

                    if( mtlLibName.empty() )
                    {
                        BOOST_THROW_EXCEPTION(std::runtime_error("Missing material library filename after mtllib statement"));
                    }

                    mtlLib = readMaterialLib(mtlLibName, shaderProgram);
                    activeMaterial.clear();

                    continue;
                }

                if( boost::algorithm::istarts_with(line, "usemtl ") )
                {
                    if( !activeMaterial.empty() )
                    {
                        result.back()->addMesh(buildMesh(mtlLib, activeMaterial, uvCoords, vpos, vnorm, faces));
                    }

                    auto space = line.find(' ');
                    if( space != std::string::npos )
                    {
                        activeMaterial = line.substr(space);
                        boost::algorithm::trim(activeMaterial);
                    }
                    else
                    {
                        activeMaterial.clear();
                    }

                    if( activeMaterial.empty() )
                    {
                        BOOST_THROW_EXCEPTION(std::runtime_error("Missing material name after usemtl statement"));
                    }

                    if( mtlLib.find(activeMaterial) == mtlLib.end() )
                    {
                        BOOST_THROW_EXCEPTION(std::runtime_error("Material used in usemtl statement is undefined"));
                    }

                    continue;
                }

                if( boost::algorithm::istarts_with(line, "v ") )
                {
                    std::vector<std::string> parts;
                    boost::algorithm::split(parts, line, [](char c) { return c == ' '; }, boost::token_compress_on);

                    if( parts.size() != 4 )
                    {
                        BOOST_THROW_EXCEPTION(std::runtime_error("Expected 3 space-separated elements after v statement"));
                    }

                    const auto x = boost::lexical_cast<float>(parts[1]) * SectorSize;
                    const auto y = boost::lexical_cast<float>(parts[2]) * SectorSize;
                    const auto z = boost::lexical_cast<float>(parts[3]) * SectorSize;

                    vpos.emplace_back(x, y, z);

                    continue;
                }

                if( boost::algorithm::istarts_with(line, "vn ") )
                {
                    std::vector<std::string> parts;
                    boost::algorithm::split(parts, line, [](char c) { return c == ' '; }, boost::token_compress_on);

                    if( parts.size() != 4 )
                    {
                        BOOST_THROW_EXCEPTION(std::runtime_error("Expected 3 space-separated elements after vn statement"));
                    }

                    const auto x = boost::lexical_cast<float>(parts[1]);
                    const auto y = boost::lexical_cast<float>(parts[2]);
                    const auto z = boost::lexical_cast<float>(parts[3]);

                    vnorm.emplace_back(x, y, z);

                    continue;
                }

                if( boost::algorithm::istarts_with(line, "vt ") )
                {
                    std::vector<std::string> parts;
                    boost::algorithm::split(parts, line, [](char c) { return c == ' '; }, boost::token_compress_on);

                    if( parts.size() != 3 )
                    {
                        BOOST_THROW_EXCEPTION(std::runtime_error("Expected 2 space-separated elements after vt statement"));
                    }

                    const auto x = boost::lexical_cast<float>(parts[1]);
                    const auto y = 1 - boost::lexical_cast<float>(parts[2]);

                    uvCoords.push_back(glm::vec2{ x, y });

                    continue;
                }

                if( boost::algorithm::istarts_with(line, "f ") )
                {
                    if( activeMaterial.empty() )
                    {
                        BOOST_THROW_EXCEPTION(std::runtime_error("Face definition without active material"));
                    }

                    std::vector<std::string> parts;
                    boost::algorithm::split(parts, line, [](char c) { return c == ' '; }, boost::token_compress_on);

                    if( parts.size() != 4 )
                    {
                        BOOST_THROW_EXCEPTION(std::runtime_error("Expected 3 space-separated elements after f statement"));
                    }

                    std::array<std::array<int, 3>, 3> face;
                    for( int i = 0; i < 3; ++i )
                    {
                        std::vector<std::string> stringIdx;
                        boost::algorithm::split(stringIdx, parts[1 + i], [](char c) { return c == '/'; }, boost::token_compress_off);
                        for( auto& str : stringIdx )
                            boost::algorithm::trim(str);

                        // v/vt/vn
                        auto& idx = face[i];

                        for(int j = 0; j < 3; ++j)
                        {
                            if(stringIdx.size() > j && !stringIdx[j].empty())
                                idx[j] = boost::lexical_cast<int>(stringIdx[j]);
                            else if(j != 0)
                                idx[j] = idx[0];
                            else
                                BOOST_THROW_EXCEPTION(std::runtime_error("Missing vertex coordinate index in f statement"));
                        }

                        // v
                        if( idx[0] > 0 )
                            --idx[0];
                        else
                            idx[0] = vpos.size() - idx[0];

                        if( idx[0] >= vpos.size() )
                        {
                            BOOST_THROW_EXCEPTION(std::runtime_error("v index out of bounds"));
                        }

                        // vt
                        if( idx[1] > 0 )
                            --idx[1];
                        else
                            idx[1] = uvCoords.size() - idx[1];

                        if( idx[1] >= uvCoords.size() )
                        {
                            BOOST_THROW_EXCEPTION(std::runtime_error("vt index out of bounds"));
                        }

                        // vn
                        if( idx[2] > 0 )
                            --idx[2];
                        else
                            idx[2] = vnorm.size() - idx[2];

                        if( !vnorm.empty() && idx[2] >= vnorm.size() )
                        {
                            BOOST_THROW_EXCEPTION(std::runtime_error("vn index out of bounds"));
                        }
                    }

                    faces.push_back(face);

                    continue;
                }

                if( boost::algorithm::istarts_with(line, "o ") )
                {
                    result.emplace_back(std::make_shared<gameplay::Model>());

                    continue;
                }
            }

            if(!activeMaterial.empty())
            {
                result.back()->addMesh(buildMesh(mtlLib, activeMaterial, uvCoords, vpos, vnorm, faces));
            }

            return result;
        }


        void write(const std::shared_ptr<gameplay::Model>& model,
                   const std::string& baseName,
                   const std::map<loader::TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap1,
                   const std::map<loader::TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap2 = {}) const
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

            for(const auto& mesh : model->getMeshes())
            {
                writeMesh(mtlMap1, mtlMap2, objFile, mtlFile, mesh);
            }
        }


    private:
        template<typename T>
        static void writeTriFaces(const void* rawIdx, size_t faceCount, size_t vertexCount, bool hasNormals, bool hasTexCoord, std::ostream& obj)
        {
            const T* idx = static_cast<const T*>(rawIdx);

            auto writeFace = [&](uint32_t idx)
                {
                    obj << '-' << (vertexCount - idx) << "/";
                    if( hasTexCoord )
                        obj << '-' << (vertexCount - idx);
                    obj << "/";
                    if( hasNormals )
                        obj << '-' << (vertexCount - idx);
                };

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


        static void write(const std::shared_ptr<gameplay::Material>& material, std::ostream& mtl, const glm::vec3& color = {0.8f, 0.8f, 0.8f})
        {
            // write some dummy values, as we don't know which texture is bound to the material.
            mtl << "newmtl " << makeMtlName(material) << "\n";
            mtl << "Kd " << color.r << " " << color.g << " " << color.b << "\n\n";
        }


        static void write(const std::shared_ptr<gameplay::Material>& material, size_t textureId, std::ostream& mtl)
        {
            // write some dummy values, as we don't know which texture is bound to the material.
            mtl << "newmtl " << makeMtlName(material) << "\n";
            mtl << "Kd 0.8 0.8 0.8\n";
            mtl << "map_Kd " << makeTextureName(textureId) << ".png\n";
            mtl << "map_d " << makeTextureName(textureId) << ".png\n\n";
        }


        static std::string makeMtlName(const std::shared_ptr<gameplay::Material>& material)
        {
            return "material_" + std::to_string(reinterpret_cast<uintptr_t>(material.get()));
        }


        static std::string makeTextureName(size_t id)
        {
            return "texture_" + std::to_string(id);
        }


        const boost::filesystem::path m_basePath;

        std::shared_ptr<gameplay::Mesh> buildMesh(std::map<std::string, std::shared_ptr<gameplay::Material>> mtlLib, std::string activeMaterial, std::vector<glm::vec2> uvCoords, std::vector<glm::vec3> vpos, std::vector<glm::vec3> vnorm, std::vector<std::array<std::array<int, 3>, 3>> faces) const
        {
#pragma pack(push, 1)
            struct VDataNormal
            {
                glm::vec2 uv;
                glm::vec3 position;
                glm::vec3 normal;

                static const gameplay::VertexFormat& getFormat()
                {
                    static const gameplay::VertexFormat::Element elems[3] = {
                        { gameplay::VertexFormat::TEXCOORD, 2 },
                        { gameplay::VertexFormat::POSITION, 3 },
                        { gameplay::VertexFormat::NORMAL, 3 }
                    };
                    static const gameplay::VertexFormat fmt{ elems, 3 };

                    Expects(fmt.getVertexSize() == sizeof(VDataNormal));

                    return fmt;
                }
            };
            struct VData
            {
                glm::vec2 uv;
                glm::vec3 position;

                static const gameplay::VertexFormat& getFormat()
                {
                    static const gameplay::VertexFormat::Element elems[2] = {
                        { gameplay::VertexFormat::TEXCOORD, 2 },
                        { gameplay::VertexFormat::POSITION, 3 }
                    };
                    static const gameplay::VertexFormat fmt{ elems, 2 };

                    Expects(fmt.getVertexSize() == sizeof(VData));

                    return fmt;
                }
            };
#pragma pack(pop)

            if(vnorm.empty())
            {
                std::vector<VData> vbuf;
                std::vector<uint32_t> idxbuf;

                for(const auto& face : faces)
                {
                    // v/vt/vn
                    for(const auto& idx : face)
                    {
                        VData vertex;
                        vertex.position = vpos[idx[0]];
                        vertex.uv = uvCoords[idx[1]];

                        idxbuf.push_back(vbuf.size());
                        vbuf.push_back(vertex);
                    }
                }

                auto mesh = std::make_shared<gameplay::Mesh>(VData::getFormat(), 0, false);
                mesh->rebuild(reinterpret_cast<float*>(&vbuf[0]), vbuf.size());

                auto part = mesh->addPart(gameplay::Mesh::TRIANGLES, gameplay::Mesh::INDEX32, idxbuf.size(), false);
                part->setIndexData(idxbuf.data(), 0, idxbuf.size());
                BOOST_ASSERT(mtlLib.find(activeMaterial) != mtlLib.end());
                part->setMaterial(mtlLib[activeMaterial]);

                return mesh;
            }
            else
            {
                std::vector<VDataNormal> vbuf;
                std::vector<uint32_t> idxbuf;

                for(const auto& face : faces)
                {
                    // v/vt/vn
                    for(const auto& idx : face)
                    {
                        VDataNormal vertex;
                        vertex.position = vpos[idx[0]];
                        vertex.uv = uvCoords[idx[1]];
                        vertex.normal = vnorm[idx[2]];

                        idxbuf.push_back(vbuf.size());
                        vbuf.push_back(vertex);
                    }
                }

                auto mesh = std::make_shared<gameplay::Mesh>(VDataNormal::getFormat(), 0, false);
                mesh->rebuild(reinterpret_cast<float*>(&vbuf[0]), vbuf.size());

                auto part = mesh->addPart(gameplay::Mesh::TRIANGLES, gameplay::Mesh::INDEX32, idxbuf.size(), false);
                part->setIndexData(idxbuf.data(), 0, idxbuf.size());
                BOOST_ASSERT(mtlLib.find(activeMaterial) != mtlLib.end());
                part->setMaterial(mtlLib[activeMaterial]);

                return mesh;
            }
        }

        void writeMesh(const std::map<loader::TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap1,
                    const std::map<loader::TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap2,
                    std::ofstream& objFile,
                    std::ofstream& mtlFile,
                    const std::shared_ptr<gameplay::Mesh>& mesh) const
        {
            objFile << "o mesh_" << reinterpret_cast<uintptr_t>(mesh.get()) << "\n";

            bool hasNormals = false;
            bool hasTexCoord = false;
            {
                const auto& vfmt = mesh->getVertexFormat();
                const size_t count = mesh->getVertexCount();
                const float* data = static_cast<const float*>(mesh->map());
                for(size_t i = 0; i < count; ++i)
                {
                    for(size_t j = 0; j < vfmt.getElementCount(); ++j)
                    {
                        switch(vfmt.getElement(j).usage)
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
                for(size_t i = 0; i < mesh->getPartCount(); ++i)
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
                        if(texIt != mtlMap1.end())
                        {
                            write(part->getMaterial(), texIt->first.tileAndFlag & TextureIndexMask, mtlFile);
                            found = true;
                        }

                        if(!found)
                        {
                            texIt = std::find_if(mtlMap2.begin(), mtlMap2.end(), finder);
                            if(texIt != mtlMap2.end())
                            {
                                write(part->getMaterial(), texIt->first.tileAndFlag & TextureIndexMask, mtlFile);
                                found = true;
                            }
                        }

                        if(!found)
                        {
                            write(part->getMaterial(), mtlFile);
                        }
                    }

                    switch(part->getIndexFormat())
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
    };
}
