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


        struct MaterialLibEntry
        {
            std::shared_ptr<gameplay::Material> material;
            std::string texture;
        };


        MaterialLibEntry readMaterial(const boost::filesystem::path& path, const std::shared_ptr<gameplay::ShaderProgram>& shaderProgram) const;


        std::map<std::string, MaterialLibEntry> readMaterialLib(const boost::filesystem::path& path, const std::shared_ptr<gameplay::ShaderProgram>& shaderProgram) const;


        bool exists(const boost::filesystem::path& path) const
        {
            return boost::filesystem::is_regular_file(m_basePath / path);
        }


        std::shared_ptr<gameplay::Model> readModel(const boost::filesystem::path& path, const std::shared_ptr<gameplay::ShaderProgram>& shaderProgram, const glm::vec3& ambientColor) const;

        void write(const std::shared_ptr<gameplay::Model>& model,
                   const std::string& baseName,
                   const std::map<loader::TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap1,
                   const std::map<loader::TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap2,
                   const glm::vec3& ambientColor) const;


    private:
        struct Vertex
        {
            int v;
            int vt;
            int vn;
        };
        using Face = std::array<Vertex, 3>;
        using FaceList = std::vector<Face>;

        using Tri = std::array<glm::vec3, 3>;
        using TriList = std::vector<Tri>;

        template<typename T>
        static void writeTriFaces(const void* rawIdx, size_t faceCount, size_t vertexCount, bool hasNormals, bool hasTexCoord, std::ostream& obj);


        static void write(const std::shared_ptr<gameplay::Material>& material, std::ostream& mtl, const glm::vec3& color);


        static void write(const std::shared_ptr<gameplay::Material>& material, size_t textureId, std::ostream& mtl, const glm::vec3& color);


        static std::string makeMtlName(const std::shared_ptr<gameplay::Material>& material);


        static std::string makeTextureName(size_t id);


        const boost::filesystem::path m_basePath;
        mutable std::map<boost::filesystem::path, std::shared_ptr<gameplay::Image>> m_imageCache;


        std::shared_ptr<gameplay::Mesh> buildMesh(const std::map<std::string, MaterialLibEntry>& mtlLib,
                                                  const std::string& activeMaterial,
                                                  const std::vector<glm::vec2>& uvCoords,
                                                  const std::vector<glm::vec3>& vpos,
                                                  const std::vector<glm::vec3>& vnorm,
                                                  const FaceList& faces,
                                                  const glm::vec3& ambientColor,
                                                  const TriList& tris) const;

        void writeMesh(const std::map<loader::TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap1,
                       const std::map<loader::TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap2,
                       std::ofstream& objFile,
                       std::ofstream& mtlFile,
                       const std::shared_ptr<gameplay::Mesh>& mesh,
                       const glm::vec3& ambientColor) const;

        static void calcColor(glm::vec4& vertexColor, const glm::vec3& vpos, const glm::vec3& ambientColor, const TriList& tris);

        std::shared_ptr<gameplay::Model> readCache(const boost::filesystem::path& cachename, const std::shared_ptr<gameplay::ShaderProgram>& shaderProgram) const;
    };
}
