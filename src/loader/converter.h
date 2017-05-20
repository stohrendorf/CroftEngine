#pragma once

#include "texture.h"

#include "gameplay.h"

#include <boost/filesystem.hpp>
#include <yaml-cpp/yaml.h>

struct aiNode;

struct aiScene;


namespace loader
{
struct Box;

struct Sector;

struct Room;


class Converter
{
public:
    explicit Converter(const boost::filesystem::path& basePath)
        : m_basePath{basePath}
    {
        create_directories(m_basePath);
    }


    void write(const std::shared_ptr<gameplay::ext::Image<gameplay::gl::RGBA8>>& srcImg, size_t id) const;


    std::shared_ptr<gameplay::gl::Texture> readTexture(const boost::filesystem::path& path) const;


    std::shared_ptr<gameplay::Material> readMaterial(const boost::filesystem::path& path, const std::shared_ptr<gameplay::ShaderProgram>& shaderProgram) const;


    bool exists(const boost::filesystem::path& path) const
    {
        return is_regular_file(m_basePath / path);
    }


    std::shared_ptr<gameplay::Model> readModel(const boost::filesystem::path& path, const std::shared_ptr<gameplay::ShaderProgram>& shaderProgram, const glm::vec3& ambientColor) const;

    void write(const std::shared_ptr<gameplay::Model>& model,
               const std::string& baseName,
               const std::map<TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap1,
               const std::map<TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap2,
               const glm::vec3& ambientColor) const;

    void write(const std::vector<Room>& rooms,
               const std::vector<Box>& boxes,
               const std::string& baseName,
               const std::map<TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap1,
               const std::map<TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap2) const;

    void write(const std::string& filename, const YAML::Node& tree) const;

private:
    static std::string makeTextureName(size_t id);

    aiNode* convert(aiScene& scene,
                    const gameplay::Node& sourceNode,
                    const std::map<TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap1,
                    const std::map<TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap2,
                    const glm::vec3& ambientColor) const;

    aiNode* convert(aiScene& scene,
                    const std::vector<Sector>& sectors,
                    const std::vector<Box>& boxes) const;

    void convert(aiScene& scene,
                 aiNode& outNode,
                 const std::shared_ptr<gameplay::Model>& model,
                 const std::map<TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap1,
                 const std::map<TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>& mtlMap2,
                 const glm::vec3& ambientColor) const;

    const boost::filesystem::path m_basePath;

    mutable std::map<boost::filesystem::path, std::shared_ptr<gameplay::gl::Texture>> m_textureCache;
};
}
