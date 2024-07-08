#include "trx.h"

#include "core/i18n.h"
#include "util/helpers.h"

#include <algorithm>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <gsl/gsl-lite.hpp>
#include <map>
#include <regex>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace
{
// NOLINTNEXTLINE(misc-no-recursion)
std::filesystem::path readSymlink(const std::filesystem::path& root,
                                  const std::filesystem::path& ref,
                                  std::filesystem::file_time_type& newestFile)
{
  if(ref.extension() != ".txt")
  {
    if(std::filesystem::is_regular_file(root / ref))
      newestFile = std::max(newestFile, std::filesystem::last_write_time(root / ref));
    return root / ref;
  }

  std::ifstream txt{util::ensureFileExists(root / ref)};
  newestFile = std::max(newestFile, std::filesystem::last_write_time(root / ref));

  std::string head;
  std::getline(txt, head);
  boost::algorithm::trim(head);
  if(!boost::algorithm::starts_with(head, "TLNK:"))
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to parse Glidos texture link file"));
  }

  head.erase(0, head.find(':') + 1);
  boost::algorithm::trim(head);
  boost::algorithm::replace_all(head, "\\", "/");
  return readSymlink(root, head, newestFile);
}
} // namespace

namespace loader::trx
{
Rectangle::Rectangle(const std::string& serialized)
{
  // Format: (x0--x1)(y0--y1)
  static const std::regex fmt(R"(\(([0-9]+)--([0-9]+)\)\(([0-9]+)--([0-9]+)\).*)");
  std::smatch matches;
  if(!std::regex_match(serialized, matches, fmt))
  {
    BOOST_LOG_TRIVIAL(error) << "Failed to parse Glidos texture coordinates: " << serialized;
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to parse Glidos texture coordinates"));
  }

  if(matches.size() != 5)
  {
    BOOST_LOG_TRIVIAL(error) << "Failed to parse Glidos texture coordinates: " << serialized;
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to parse Glidos texture coordinates"));
  }

  m_xy0 = {boost::lexical_cast<int32_t>(matches[1].str()), boost::lexical_cast<int32_t>(matches[3].str())};
  m_xy1 = {boost::lexical_cast<int32_t>(matches[2].str()), boost::lexical_cast<int32_t>(matches[4].str())};

  gsl_Ensures(m_xy0.x < m_xy1.x);
  gsl_Ensures(m_xy0.y < m_xy1.y);
}

TexturePart::TexturePart(const std::string& serialized)
{
  std::vector<std::string> parts;
  boost::algorithm::split(parts, serialized, boost::is_any_of("\\/"));
  if(parts.size() != 2 || parts[0].size() != 32)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to parse Glidos texture part"));
  }

  m_textureId = parts[0];
  m_rect = Rectangle{parts[1]};
}

EquivalenceSet::EquivalenceSet(std::ifstream& file)
{
  std::string line;
  while(std::getline(file, line))
  {
    boost::algorithm::trim(line);
    if(line.empty() || boost::algorithm::starts_with(line, "//"))
      continue;

    if(line == "EndEquiv")
    {
      break;
    }

    m_parts.emplace(line);
  }
}

Equiv::Equiv(const std::filesystem::path& filename,
             const std::function<void(const std::string&)>& statusCallback,
             std::filesystem::file_time_type& newestFile)
{
  std::ifstream file{filename};
  if(!file.is_open())
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to open Glidos equiv file"));
  }

  newestFile = std::max(newestFile, std::filesystem::last_write_time(filename));

  const auto size = std::filesystem::file_size(filename);

  std::string head;
  std::getline(file, head);
  boost::algorithm::trim(head);
  if(head != "GLIDOS TEXTURE EQUIV")
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Invalid texture equiv header"));
  }

  static constexpr auto updateInterval = std::chrono::milliseconds(40);
  auto nextUpdate = std::chrono::high_resolution_clock::now() + updateInterval;
  std::string line;
  while(std::getline(file, line))
  {
    boost::algorithm::trim(line);
    if(line.empty() || boost::algorithm::starts_with(line, "//"))
      continue;

    if(const auto now = std::chrono::high_resolution_clock::now(); now >= nextUpdate)
    {
      nextUpdate = now + updateInterval;
      statusCallback(_("Glidos - Loading equiv.txt (%1%%%)", file.tellg() * 100 / size));
    }

    if(line == "BeginEquiv")
    {
      m_equivalentSets.emplace_back(file);
    }
  }
}

void Equiv::resolve(const std::filesystem::path& root,
                    std::map<TexturePart, std::filesystem::path>& filesByPart,
                    const std::function<void(const std::string&)>& statusCallback,
                    std::filesystem::file_time_type& newestFile) const
{
  if(m_equivalentSets.empty())
    return;

  BOOST_LOG_TRIVIAL(info) << "Resolving " << m_equivalentSets.size() << " equiv sets...";

  auto resolved = std::count_if(m_equivalentSets.begin(),
                                m_equivalentSets.end(),
                                [](const EquivalenceSet& set)
                                {
                                  return set.isResolved();
                                });

  statusCallback(_("Glidos - Resolving maps (%1%%%)", resolved * 100 / m_equivalentSets.size()));

  for(const auto& set : m_equivalentSets)
  {
    if(set.isResolved())
      continue;

    // first try to find an entry in the equiv set that actually exists
    std::filesystem::path partFile;
    for(const auto& part : set.getParts())
    {
      auto it = filesByPart.find(part);
      if(it != filesByPart.end())
      {
        if(!partFile.empty() && partFile != it->second)
        {
          BOOST_LOG_TRIVIAL(warning) << "Ambiguous source reference in equiv set";
        }
        partFile = it->second;
        if(std::filesystem::is_regular_file(partFile))
          newestFile = std::max(newestFile, std::filesystem::last_write_time(partFile));
      }
    }

    if(partFile.empty())
    {
      const auto first = *set.getParts().begin();
      BOOST_LOG_TRIVIAL(debug) << "Invalid equiv set: No entry references a known texture part (example: "
                               << first.getId() << "/" << first.getRectangle() << ")";
      continue;
    }

    // now map the found part to all other parts, i.e. make them actually equivalent
    for(const auto& part : set.getParts())
    {
      const auto linked = readSymlink(root, relative(partFile, root), newestFile).lexically_normal();
      auto& existing = filesByPart[part];
      if(!existing.empty() && existing != linked)
      {
        BOOST_LOG_TRIVIAL(error) << "Equiv set references already mapped texture part: " << linked << " vs. "
                                 << existing;
      }
      existing = linked;
    }
    set.markResolved();

    ++resolved;
    // avoid too many draw calls
    if(resolved % 10 == 0)
    {
      statusCallback(_("Glidos - Resolving maps (%1%%%)", resolved * 100 / m_equivalentSets.size()));
    }
  }
}

PathMap::PathMap(const std::filesystem::path& baseTxtName,
                 std::map<TexturePart, std::filesystem::path>& filesByPart,
                 std::filesystem::file_time_type& newestFile)
{
  newestFile = std::max(newestFile, std::filesystem::last_write_time(baseTxtName));
  std::ifstream txt{util::ensureFileExists(baseTxtName)};
  const auto baseTxtDir = baseTxtName.parent_path();

  std::string head;
  std::getline(txt, head);
  boost::algorithm::trim(head);
  if(head != "GLIDOS TEXTURE MAPPING")
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Invalid texture mapping header"));
  }

  // contains root+base
  std::map<std::string, std::filesystem::path> dirByTextureId;

  std::string line;
  std::filesystem::path base;
  while(std::getline(txt, line))
  {
    boost::algorithm::trim(line);
    if(line.empty() || boost::algorithm::starts_with(line, "//"))
      continue;

    if(boost::algorithm::starts_with(line, "ROOT:"))
    {
      if(!m_root.empty())
        BOOST_THROW_EXCEPTION(std::runtime_error("Multiple 'ROOT' statements"));

      line.erase(0, line.find(':') + 1);
      boost::algorithm::trim(line);
      m_root = baseTxtDir / boost::algorithm::replace_all_copy(line, "\\", "/");
    }
    else if(boost::algorithm::starts_with(line, "BASE:"))
    {
      line.erase(0, line.find(':') + 1);
      boost::algorithm::trim(line);
      base = boost::algorithm::replace_all_copy(line, "\\", "/");
    }
    else
    {
      std::vector<std::string> parts;
      boost::algorithm::split(parts, line, boost::is_any_of(" \t"));
      if(parts.size() != 2)
      {
        BOOST_THROW_EXCEPTION(std::runtime_error("Failed to parse mapping line"));
      }

      gsl_Assert(parts[0].size() == 32);

      if(dirByTextureId.find(parts[0]) != dirByTextureId.end())
      {
        BOOST_THROW_EXCEPTION(std::runtime_error("Texture path mapping already registered"));
      }

      dirByTextureId[parts[0]] = base / boost::algorithm::replace_all_copy(parts[1], "\\", "/");
    }
  }

  for(const auto& texturePath : dirByTextureId)
  {
    // contains root+base
    const auto fullTexturePath = m_root / texturePath.second;

    if(!is_directory(fullTexturePath))
    {
      BOOST_LOG_TRIVIAL(warning) << "Directory " << fullTexturePath << " does not exist, skipping";
      continue;
    }

    for(const auto& file : std::filesystem::directory_iterator{fullTexturePath})
    {
      Rectangle r;
      try
      {
        r = Rectangle{file.path().filename().string()};
      }
      catch(std::runtime_error&)
      {
        BOOST_LOG_TRIVIAL(debug) << "No texture coordinates in filename " << file.path();
        continue;
      }

      try
      {
        const auto link = readSymlink(m_root, relative(file.path(), m_root), newestFile).lexically_normal();
        filesByPart[TexturePart(texturePath.first, r)] = link;
      }
      catch(std::runtime_error& ex)
      {
        BOOST_LOG_TRIVIAL(debug) << "Failed to follow texture links: " << ex.what();
        continue;
      }
    }
  }
}

Glidos::Glidos(std::filesystem::path baseDir, const std::function<void(const std::string&)>& statusCallback)
    : m_baseDir{std::move(baseDir)}
{
  if(!is_directory(m_baseDir))
    BOOST_THROW_EXCEPTION(std::runtime_error("Base path is not a directory"));

  BOOST_LOG_TRIVIAL(debug) << "Loading Glidos texture pack from " << m_baseDir;

  if(is_regular_file(m_baseDir / "equiv.txt"))
  {
    m_newestFile = std::max(m_newestFile, std::filesystem::last_write_time(m_baseDir / "equiv.txt"));
    BOOST_LOG_TRIVIAL(debug) << "Loading equiv.txt";
    const Equiv equiv{util::ensureFileExists(m_baseDir / "equiv.txt"), statusCallback, m_newestFile};

    std::vector<PathMap> maps;

    for(const auto& entry : std::filesystem::directory_iterator{m_baseDir})
    {
      if(!is_regular_file(entry) || entry.path().extension() != ".txt")
        continue;

      if(entry.path().filename() == "equiv.txt")
        continue;

      m_newestFile = std::max(m_newestFile, std::filesystem::last_write_time(entry));
      statusCallback(_("Glidos - Loading %1%", entry.path().filename().string()));
      BOOST_LOG_TRIVIAL(debug) << "Loading part map " << entry.path();
      maps.emplace_back(entry, m_filesByPart, m_newestFile);
    }

    BOOST_LOG_TRIVIAL(debug) << "Resolving links and equiv sets for " << maps.size() << " mappings";
    for(const auto& map : maps)
    {
      equiv.resolve(map.getRoot(), m_filesByPart, statusCallback, m_newestFile);
    }
    statusCallback(_("Glidos - Resolving maps (100%)"));
  }
  else
  {
    static const std::regex md5Expr{"[a-zA-Z0-9]{32}"};
    for(const auto& entry : std::filesystem::directory_iterator{m_baseDir})
    {
      if(!std::filesystem::is_directory(entry))
      {
        continue;
      }

      const std::string& textureId = entry.path().filename().string();
      if(!std::regex_match(textureId, md5Expr))
      {
        continue;
      }

      for(const auto& subEntry : std::filesystem::directory_iterator{entry.path()})
      {
        try
        {
          m_filesByPart.emplace(TexturePart{textureId, Rectangle{subEntry.path().filename().string()}}, subEntry);
          m_newestFile = std::max(m_newestFile, std::filesystem::last_write_time(subEntry));
        }
        catch(std::runtime_error&)
        {
          // ignore invalid file names
        }
      }
    }
  }
}

Glidos::TileMap Glidos::getMappingsForTexture(const std::string& textureId) const
{
  TileMap result;

  for(const auto& [part, file] : m_filesByPart)
  {
    if(part.getId() != textureId)
      continue;

    result[part.getRectangle()] = file;
  }

  return result;
}

void Glidos::insertInternalMapping(const std::string& textureId, const Rectangle& tile)
{
  m_filesByPart.emplace(TexturePart{textureId, tile}, std::filesystem::path{});
}
} // namespace loader::trx
