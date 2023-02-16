#pragma once

#include <cstdint>
#include <filesystem>
#include <map>

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

struct GhostManager
{
  GhostManager(const std::filesystem::path& recordingPath, world::World& world);

  ~GhostManager();

  bool askGhostSave(Presenter& presenter, world::World& world);

  std::shared_ptr<ghosting::GhostModel> model;
  std::map<uint64_t, std::shared_ptr<ghosting::GhostModel>> remoteModels;
  const std::filesystem::path readerPath;
  std::unique_ptr<ghosting::GhostDataReader> reader;
  const std::filesystem::path writerPath;
  std::unique_ptr<ghosting::GhostDataWriter> writer;
};
} // namespace engine
