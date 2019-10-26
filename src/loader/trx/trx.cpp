#include "trx.h"

#include "gsl-lite.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/log/trivial.hpp>
#include <utility>

namespace
{
boost::filesystem::path readSymlink(const boost::filesystem::path& root,
                                    const boost::filesystem::path& ref,
                                    std::chrono::system_clock::time_point& srcTimestamp)
{
  if(ref.extension() != ".txt")
    return root / ref;

  std::ifstream txt{(root / ref).string()};
  if(!txt.is_open())
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to open Glidos text file"));
  }

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
  srcTimestamp = std::max(srcTimestamp, std::chrono::system_clock::from_time_t(last_write_time(root / ref)));
  return readSymlink(root, head, srcTimestamp);
}
} // namespace

namespace loader
{
namespace trx
{
Rectangle::Rectangle(const std::string& serialized)
{
  // Format: (x0--x1)(y0--y1)
  const std::regex fmt(R"(\(([0-9]+)--([0-9]+)\)\(([0-9]+)--([0-9]+)\).*)");
  std::smatch matches;
  if(!std::regex_match(serialized, matches, fmt))
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to parse Glidos texture coordinates"));
  }

  if(matches.size() != 5)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to parse Glidos texture coordinates"));
  }

  m_x0 = boost::lexical_cast<uint32_t>(matches[1].str());
  m_x1 = boost::lexical_cast<uint32_t>(matches[2].str());
  m_y0 = boost::lexical_cast<uint32_t>(matches[3].str());
  m_y1 = boost::lexical_cast<uint32_t>(matches[4].str());

  Expects(m_x0 <= m_x1);
  Expects(m_y0 <= m_y1);
}

TexturePart::TexturePart(const std::string& serialized)
{
  std::vector<std::string> parts;
  split(parts, serialized, boost::is_any_of("\\/"));
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
  boost::filesystem::path base;
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

Equiv::Equiv(const boost::filesystem::path& filename)
{
  std::ifstream file{filename.string()};
  if(!file.is_open())
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to open Glidos equiv file"));
  }

  std::string head;
  std::getline(file, head);
  boost::algorithm::trim(head);
  if(head != "GLIDOS TEXTURE EQUIV")
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Invalid texture equiv header"));
  }

  std::string line;
  boost::filesystem::path base;
  while(std::getline(file, line))
  {
    boost::algorithm::trim(line);
    if(line.empty() || boost::algorithm::starts_with(line, "//"))
      continue;

    if(line == "BeginEquiv")
    {
      m_equivalentSets.emplace_back(file);
    }
  }
}

void Equiv::resolve(const boost::filesystem::path& root,
                    std::map<std::string, std::chrono::system_clock::time_point>& timestamps,
                    std::chrono::system_clock::time_point& rootTimestamp,
                    std::map<TexturePart, boost::filesystem::path>& filesByPart,
                    const std::function<void(const std::string&)>& statusCallback) const
{
  BOOST_LOG_TRIVIAL(info) << "Resolving " << m_equivalentSets.size() << " equiv sets...";

  auto resolved = std::count_if(
    m_equivalentSets.begin(), m_equivalentSets.end(), [](const EquivalenceSet& set) { return set.isResolved(); });

  statusCallback("Glidos - Resolving maps (" + std::to_string(resolved * 100 / m_equivalentSets.size()) + "%)");

  for(const auto& set : m_equivalentSets)
  {
    if(set.isResolved())
      continue;

    // first try to find an entry in the equiv set that actually exists
    boost::filesystem::path partFile;
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
      }
    }

    if(partFile.empty())
    {
      //BOOST_LOG_TRIVIAL( debug ) << "Invalid equiv set: No entry references a known texture part";
      continue;
    }

    // now map the found part to all other parts, i.e. make them actually equivalent
    for(const auto& part : set.getParts())
    {
      auto& ts = timestamps[part.getId()];
      ts = std::max(ts, rootTimestamp);
      const auto linked = readSymlink(root, relative(partFile, root), ts).lexically_normal();
      if(linked.empty())
      {
        BOOST_LOG_TRIVIAL(error) << "Invalid TLNK in " << root / partFile;
        continue;
      }

      auto& existing = filesByPart[part];
      if(!existing.empty() && existing != linked)
      {
        //BOOST_THROW_EXCEPTION( std::runtime_error( "Equiv set references already mapped texture part" ) );
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
      statusCallback("Glidos - Resolving maps (" + std::to_string(resolved * 100 / m_equivalentSets.size()) + "%)");
    }
  }
}

PathMap::PathMap(const boost::filesystem::path& baseTxtName,
                 std::map<std::string, std::chrono::system_clock::time_point>& timestamps,
                 std::chrono::system_clock::time_point& rootTimestamp,
                 std::map<TexturePart, boost::filesystem::path>& filesByPart)
{
  std::ifstream txt{baseTxtName.string()};
  if(!txt.is_open())
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to open Glidos base file"));
  }

  const auto baseTxtDir = baseTxtName.parent_path();

  std::string head;
  std::getline(txt, head);
  boost::algorithm::trim(head);
  if(head != "GLIDOS TEXTURE MAPPING")
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Invalid texture mapping header"));
  }

  // contains root+base
  std::map<std::string, boost::filesystem::path> dirByTextureId;

  std::string line;
  boost::filesystem::path base;
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
      split(parts, line, boost::is_any_of(" \t"));
      if(parts.size() != 2)
      {
        BOOST_THROW_EXCEPTION(std::runtime_error("Failed to parse mapping line"));
      }

      Expects(parts[0].size() == 32);

      if(dirByTextureId.find(parts[0]) != dirByTextureId.end())
      {
        BOOST_THROW_EXCEPTION(std::runtime_error("Texture path mapping already registered"));
      }

      dirByTextureId[parts[0]] = base / boost::algorithm::replace_all_copy(parts[1], "\\", "/");
    }
  }

  const boost::filesystem::directory_iterator end{};

  for(const auto& texturePath : dirByTextureId)
  {
    // contains root+base
    const auto fullTexturePath = m_root / texturePath.second;

    if(!is_directory(fullTexturePath))
    {
      BOOST_LOG_TRIVIAL(warning) << "Directory " << fullTexturePath << " does not exist, skipping";
      continue;
    }

    for(boost::filesystem::directory_iterator it{fullTexturePath}; it != end; ++it)
    {
      Rectangle r;
      try
      {
        r = Rectangle{it->path().filename().string()};
      }
      catch(std::runtime_error&)
      {
        BOOST_LOG_TRIVIAL(debug) << "No texture coordinates in filename " << it->path();
        continue;
      }

      try
      {
        auto& ts = timestamps[texturePath.first];
        ts = std::max(ts, rootTimestamp);
        const auto link = readSymlink(m_root, relative(it->path(), m_root), ts).lexically_normal();
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

Glidos::Glidos(boost::filesystem::path baseDir, const std::function<void(const std::string&)>& statusCallback)
    : m_baseDir{std::move(baseDir)}
{
  if(!is_directory(m_baseDir))
    BOOST_THROW_EXCEPTION(std::runtime_error("Base path is not a directory"));

  BOOST_LOG_TRIVIAL(debug) << "Loading Glidos texture pack from " << m_baseDir;

  m_rootTimestamp = std::chrono::system_clock::from_time_t(last_write_time(m_baseDir / "equiv.txt"));

  statusCallback("Glidos - Loading equiv.txt");

  BOOST_LOG_TRIVIAL(debug) << "Loading equiv.txt";
  Equiv equiv{m_baseDir / "equiv.txt"};

  std::vector<PathMap> maps;

  const boost::filesystem::directory_iterator end{};
  for(boost::filesystem::directory_iterator it{m_baseDir}; it != end; ++it)
  {
    if(!is_regular_file(it->path()))
      continue;

    if(it->path().filename() == "equiv.txt")
      continue;

    statusCallback("Glidos - Loading " + it->path().filename().string());
    BOOST_LOG_TRIVIAL(debug) << "Loading part map " << it->path();
    maps.emplace_back(it->path(), m_newestTextureSourceTimestamps, m_rootTimestamp, m_filesByPart);
  }

  BOOST_LOG_TRIVIAL(debug) << "Resolving links and equiv sets for " << maps.size() << " mappings";
  for(const auto& map : maps)
  {
    equiv.resolve(map.getRoot(), m_newestTextureSourceTimestamps, m_rootTimestamp, m_filesByPart, statusCallback);
  }
  statusCallback("Glidos - Resolving maps (100%)");
}

void Glidos::dump() const
{
  BOOST_LOG_TRIVIAL(info) << "Glidos database dump";
  std::set<std::string> md5s;
  for(const auto& part : m_filesByPart)
  {
    md5s.insert(part.first.getId());
  }

  for(const auto& md5 : md5s)
  {
    BOOST_LOG_TRIVIAL(info) << "Texture " << md5;
    auto mappings = getMappingsForTexture(md5);
    if(mappings.tiles.empty())
    {
      BOOST_LOG_TRIVIAL(warning) << "  - No mappings";
      continue;
    }

    for(const auto& tile : mappings.tiles)
    {
      BOOST_LOG_TRIVIAL(info) << "  - " << tile.first << " => " << tile.second;
    }
  }
}

Glidos::TileMap Glidos::getMappingsForTexture(const std::string& textureId) const
{
  TileMap result;
  result.newestSource = std::max(m_newestTextureSourceTimestamps[textureId], m_rootTimestamp);
  result.baseDir = m_baseDir;

  for(const auto& link : m_filesByPart)
  {
    if(link.first.getId() != textureId)
      continue;

    result.tiles[link.first.getRectangle()] = link.second;
  }

  return result;
}
} // namespace trx
} // namespace loader
