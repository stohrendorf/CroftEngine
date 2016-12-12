#pragma once

#include "texture.h"

#include "gameplay.h"

#include "gsl/gsl"

#include <glm/gtc/type_ptr.hpp>

#include <boost/filesystem.hpp>


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


        void write(const std::shared_ptr<gameplay::Image>& srcImg, size_t id) const;


        std::shared_ptr<gameplay::Image> readImage(const boost::filesystem::path& path) const;


        std::shared_ptr<gameplay::Texture> readTexture(const boost::filesystem::path& path) const;


        std::shared_ptr<gameplay::Material> readMaterial(const boost::filesystem::path& path, const std::shared_ptr<gameplay::ShaderProgram>& shaderProgram) const;


        std::map<std::string, std::shared_ptr<gameplay::Material>> readMaterialLib(const boost::filesystem::path& path, const std::shared_ptr<gameplay::ShaderProgram>& shaderProgram) const;


        bool exists(const boost::filesystem::path& path) const
        {
            return boost::filesystem::is_regular_file(m_basePath / path);
        }


        std::vector<std::shared_ptr<gameplay::Model>> readModels(const boost::filesystem::path& path, const std::shared_ptr<gameplay::ShaderProgram>& shaderProgram) const;

        void write(const std::shared_ptr<gameplay::Model>& model,
                   const std::string& baseName,
                   const std::map<loader::TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap1,
                   const std::map<loader::TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap2,
                   const glm::vec3& ambientColor) const;


    private:
        template<typename T>
        static void writeTriFaces(const void* rawIdx, size_t faceCount, size_t vertexCount, bool hasNormals, bool hasTexCoord, std::ostream& obj);


        static void write(const std::shared_ptr<gameplay::Material>& material, std::ostream& mtl, const glm::vec3& color);


        static void write(const std::shared_ptr<gameplay::Material>& material, size_t textureId, std::ostream& mtl, const glm::vec3& color);


        static std::string makeMtlName(const std::shared_ptr<gameplay::Material>& material);


        static std::string makeTextureName(size_t id);


        const boost::filesystem::path m_basePath;
        mutable std::map<boost::filesystem::path, std::shared_ptr<gameplay::Image>> m_imageCache;


        std::shared_ptr<gameplay::Mesh> buildMesh(std::map<std::string, std::shared_ptr<gameplay::Material>> mtlLib,
                                                  std::string activeMaterial,
                                                  std::vector<glm::vec2> uvCoords,
                                                  std::vector<glm::vec3> vpos, std::vector<glm::vec3> vnorm,
                                                  std::vector<std::array<std::array<int, 3>, 3>> faces) const;

        void writeMesh(const std::map<loader::TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap1,
                       const std::map<loader::TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap2,
                       std::ofstream& objFile,
                       std::ofstream& mtlFile,
                       const std::shared_ptr<gameplay::Mesh>& mesh,
                       const glm::vec3& ambientColor) const;
    };
}
