#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <memory>

namespace engine::world
{
class World;
}

namespace engine::ghosting
{
class GhostModel;
class GhostDataReader;
class GhostDataWriter;
} // namespace engine::ghosting

namespace engine
{
class Presenter;

class GhostManager final
{
public:
  GhostManager(const std::filesystem::path& recordingPath, world::World& world);

  ~GhostManager();

  bool askGhostSave(Presenter& presenter, world::World& world);

  [[nodiscard]] const auto& getModel() const noexcept
  {
    return m_model;
  }

  [[nodiscard]] const auto& getRemoteModels() const noexcept
  {
    return m_remoteModels;
  }

  [[nodiscard]] auto& getRemoteModels() noexcept
  {
    return m_remoteModels;
  }

  [[nodiscard]] const auto& getReader() const noexcept
  {
    return m_reader;
  }

  [[nodiscard]] const auto& getWriter() const noexcept
  {
    return m_writer;
  }

private:
  std::shared_ptr<ghosting::GhostModel> m_model;
  std::map<uint64_t, std::shared_ptr<ghosting::GhostModel>> m_remoteModels;
  std::filesystem::path m_readerPath;
  std::unique_ptr<ghosting::GhostDataReader> m_reader;
  std::filesystem::path m_writerPath;
  std::unique_ptr<ghosting::GhostDataWriter> m_writer;
};
} // namespace engine
