#include "savegamecleanup.h"

#include "engine/engine.h"
#include "util/fsutil.h"

#include <algorithm>
#include <boost/log/trivial.hpp>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <iterator>
#include <map>
#include <optional>
#include <system_error>
#include <vector>

namespace
{
struct PathLess
{
  [[nodiscard]] bool operator()(const std::filesystem::path& l, const std::filesystem::path& r) const
  {
    if(util::preferredEqual(l, r))
      return false;
    return l < r;
  }
};

[[nodiscard]] std::filesystem::path makeTempFilepath(std::filesystem::path path)
{
  path.replace_extension(path.extension().string() + "tmp");
  return path;
}

void renameToTmpFiles(const engine::Engine& engine, const std::map<size_t, engine::SavegameInfo>& savegameInfos)
{
  for(const auto& [slot, info] : savegameInfos)
  {
    const auto path = engine.getSavegamePath(slot);
    const auto metaPath = engine::makeMetaFilepath(path);
    util::rename(path, makeTempFilepath(path));
    util::rename(metaPath, makeTempFilepath(metaPath));
  }
}

void orderBy(const engine::Engine& engine,
             const std::map<size_t, engine::SavegameInfo>& savegameInfos,
             const std::function<bool(const engine::SavegameInfo&, const engine::SavegameInfo&)>& predicate)
{
  // first, rename all saves to temporary files
  renameToTmpFiles(engine, savegameInfos);

  // then get a list of all used slots
  std::vector<size_t> currentSlots;
  std::transform(savegameInfos.begin(),
                 savegameInfos.end(),
                 std::back_inserter(currentSlots),
                 [](const auto& slotAndInfo)
                 {
                   return slotAndInfo.first;
                 });

  // now order the slots according to the predicate
  auto orderedSlots = currentSlots;
  std::stable_sort(orderedSlots.begin(),
                   orderedSlots.end(),
                   [&savegameInfos, &predicate](auto slotA, auto slotB)
                   {
                     return predicate(savegameInfos.at(slotA), savegameInfos.at(slotB));
                   });

  // now rename the temporary saves to the ordinary saves while getting re-ordered
  for(size_t i = 0; i < currentSlots.size(); ++i)
  {
    util::rename(makeTempFilepath(engine.getSavegamePath(currentSlots.at(i))),
                 engine.getSavegamePath(orderedSlots.at(i)));
    util::rename(makeTempFilepath(engine::makeMetaFilepath(engine.getSavegamePath(currentSlots.at(i)))),
                 engine::makeMetaFilepath(engine.getSavegamePath(orderedSlots.at(i))));
  }
}
} // namespace

namespace menu
{
void deleteSave(const engine::Engine& engine, size_t slot)
{
  std::error_code ec;
  const auto savegamePath = engine.getSavegamePath(slot);
  const auto metaPath = engine::makeMetaFilepath(savegamePath);
  BOOST_LOG_TRIVIAL(info) << "Deleting " << savegamePath << " and " << metaPath;
  std::filesystem::remove(savegamePath, ec);
  std::filesystem::remove(metaPath, ec);
}

void deleteSavesExcept(const engine::Engine& engine,
                       const std::map<size_t, engine::SavegameInfo>& savegameInfos,
                       size_t slot)
{
  for(const auto& [n, info] : savegameInfos)
  {
    if(n != slot)
    {
      deleteSave(engine, n);
    }
  }
}

void deleteSavesBefore(const engine::Engine& engine,
                       const std::map<size_t, engine::SavegameInfo>& savegameInfos,
                       const std::filesystem::file_time_type& referenceTime)
{
  for(const auto& [n, info] : savegameInfos)
  {
    if(info.saveTime < referenceTime)
    {
      deleteSave(engine, n);
    }
  }
}

size_t compactSaves(const engine::Engine& engine,
                    const std::map<size_t, engine::SavegameInfo>& savegameInfos,
                    const std::optional<size_t>& selectedSlot)
{
  size_t dstSlot = 0;
  size_t newSelectedSlot = 0;
  for(const auto& [srcSlot, info] : savegameInfos)
  {
    const auto srcPath = engine.getSavegamePath(srcSlot);
    const auto srcMetaPath = engine::makeMetaFilepath(srcPath);
    const auto dstPath = engine.getSavegamePath(dstSlot);
    const auto dstMetaPath = engine::makeMetaFilepath(dstPath);
    ++dstSlot;

    util::rename(srcPath, dstPath);
    util::rename(srcMetaPath, dstMetaPath);
    if(srcSlot == selectedSlot)
    {
      newSelectedSlot = dstSlot;
    }
  }

  return newSelectedSlot;
}

void deleteSavesPerLevelExceptNewest(const engine::Engine& engine,
                                     const std::map<size_t, engine::SavegameInfo>& savegameInfos)
{
  struct SlotTimestamp
  {
    size_t slot;
    std::filesystem::file_time_type timeStamp;
  };

  std::map<std::filesystem::path, SlotTimestamp, PathLess> newestPerLevel;
  for(const auto& [slot, savegameInfo] : savegameInfos)
  {
    const auto it = newestPerLevel.find(savegameInfo.meta.filename);
    if(it == newestPerLevel.end())
    {
      newestPerLevel.emplace(savegameInfo.meta.filename, SlotTimestamp{slot, savegameInfo.saveTime});
    }
    else if(it->second.timeStamp < savegameInfo.saveTime)
    {
      it->second = SlotTimestamp{slot, savegameInfo.saveTime};
    }
  }

  for(const auto& [slot, savegameInfo] : savegameInfos)
  {
    const auto& newestLevelTs = newestPerLevel.at(savegameInfo.meta.filename);
    if(savegameInfo.saveTime < newestLevelTs.timeStamp)
    {
      std::error_code ec;
      const auto savegamePath = engine.getSavegamePath(slot);
      BOOST_LOG_TRIVIAL(info) << "Deleting slot " << slot;
      std::filesystem::remove(savegamePath, ec);
      std::filesystem::remove(engine::makeMetaFilepath(savegamePath), ec);
    }
  }
}

void orderByDate(const engine::Engine& engine, const std::map<size_t, engine::SavegameInfo>& savegameInfos)
{
  orderBy(engine,
          savegameInfos,
          [](const engine::SavegameInfo& a, const engine::SavegameInfo& b)
          {
            return a.saveTime < b.saveTime;
          });
}

void orderByLevelThenDate(const engine::Engine& engine, const std::map<size_t, engine::SavegameInfo>& savegameInfos)
{
  orderBy(engine,
          savegameInfos,
          [](const engine::SavegameInfo& a, const engine::SavegameInfo& b)
          {
            if(!util::preferredEqual(a.meta.filename, b.meta.filename))
              return a.meta.filename < b.meta.filename;

            return a.saveTime < b.saveTime;
          });
}
} // namespace menu
