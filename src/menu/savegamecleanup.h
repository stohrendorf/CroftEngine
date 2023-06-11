#pragma once

#include <filesystem>
#include <map>
#include <optional>

namespace engine
{
class Engine;
struct SavegameInfo;
} // namespace engine

namespace menu
{
extern void deleteSave(const engine::Engine& engine, size_t slot);
extern void deleteSavesExcept(const engine::Engine& engine,
                              const std::map<size_t, engine::SavegameInfo>& savegameInfos,
                              size_t slot);
extern void deleteSavesBefore(const engine::Engine& engine,
                              const std::map<size_t, engine::SavegameInfo>& savegameInfos,
                              const std::filesystem::file_time_type& referenceTime);
[[nodiscard]] extern size_t compactSaves(const engine::Engine& engine,
                                         const std::map<size_t, engine::SavegameInfo>& savegameInfos,
                                         const std::optional<size_t>& selectedSlot);
extern void deleteSavesPerLevelExceptNewest(const engine::Engine& engine,
                                            const std::map<size_t, engine::SavegameInfo>& savegameInfos);
extern void orderByDate(const engine::Engine& engine, const std::map<size_t, engine::SavegameInfo>& savegameInfos);
extern void orderByLevelThenDate(const engine::Engine& engine,
                                 const std::map<size_t, engine::SavegameInfo>& savegameInfos);
} // namespace menu
