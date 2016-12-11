#pragma once

#include "gameplay.h"

#include "CImg.h"

#include "gsl/gsl"

#include <glm/gtc/type_ptr.hpp>

#include <boost/filesystem.hpp>
#include <fstream>

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
            img *= 255;
            img.permute_axes("yzcx");

            auto fullPath = m_basePath / makeTextureName(id);
            fullPath.replace_extension("png");

            img.save_png(fullPath.string().c_str());
        }

        void write(const std::shared_ptr<gameplay::Mesh>& mesh,
                   const std::string& baseName,
                   const std::map<loader::TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap1,
                   const std::map<loader::TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap2 = {}) const
        {
            Expects(mesh != nullptr);

            auto fullPath = m_basePath / baseName;

            fullPath.replace_extension("obj");
            std::ofstream objFile{ fullPath.string(), std::ios::out|std::ios::trunc };
            if(!objFile.is_open())
                BOOST_THROW_EXCEPTION(std::runtime_error("Cannot create .obj file"));
            objFile << std::fixed;

            fullPath.replace_extension("mtl");
            std::ofstream mtlFile{ fullPath.string(), std::ios::out|std::ios::trunc };
            if(!mtlFile.is_open())
                BOOST_THROW_EXCEPTION(std::runtime_error("Cannot create .mtl file"));
            mtlFile << std::fixed;

            objFile << "# EdisonEngine Model Dump\n";
            objFile << "mtllib " << baseName << ".mtl\n\n";
            objFile << "o TRModel\n";

            bool hasNormals = false;
            bool hasTexCoord = false;
            {
                const auto& vfmt = mesh->getVertexFormat();
                const size_t count = mesh->getVertexCount();
                const float* data = static_cast<const float*>( mesh->map() );
                for(size_t i=0; i<count; ++i)
                {
                    for(size_t j = 0; j < vfmt.getElementCount(); ++j)
                    {
                        switch(vfmt.getElement(j).usage)
                        {
                            case gameplay::VertexFormat::POSITION:
                                objFile << "v " << data[0] << " " << data[1] << " " << data[2] << "\n";
                                break;
                            case gameplay::VertexFormat::NORMAL:
                                objFile << "vn " << data[0] << " " << data[1] << " " << data[2] << "\n";
                                hasNormals = true;
                                break;
                            case gameplay::VertexFormat::TEXCOORD:
                                objFile << "vt " << data[0] << " " << 1-data[1] << "\n";
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
                            writeTriFaces<uint8_t>(part->map(), part->getIndexCount(), hasNormals, hasTexCoord, objFile);
                            part->unmap();
                            break;
                        case gameplay::Mesh::INDEX16:
                            writeTriFaces<uint16_t>(part->map(), part->getIndexCount(), hasNormals, hasTexCoord, objFile);
                            part->unmap();
                            break;
                        case gameplay::Mesh::INDEX32:
                            writeTriFaces<uint32_t>(part->map(), part->getIndexCount(), hasNormals, hasTexCoord, objFile);
                            part->unmap();
                            break;
                        default:
                            break;
                    }
                }
            }
        }

    private:
        template<typename T>
        static void writeTriFaces(const void* rawIdx, size_t count, bool hasNormals, bool hasTexCoord, std::ostream& obj)
        {
            const T* idx = static_cast<const T*>(rawIdx);

            auto write = [&](uint32_t idx)
            {
                obj << idx+1u << "/";
                if(hasTexCoord)
                    obj << idx + 1u;
                obj << "/";
                if(hasNormals)
                    obj << idx + 1u;
            };

            for(size_t i=0; i<count; i += 3, idx += 3)
            {
                obj << "f ";
                write(idx[0]);
                obj << " ";
                write(idx[1]);
                obj << " ";
                write(idx[2]);
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
    };
}
