#pragma once

#include <gsl/gsl>

#include <boost/throw_exception.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <regex>


namespace loader
{
    namespace trx
    {
        class Rectangle
        {
        public:
            explicit Rectangle() = default;


            explicit Rectangle(const std::string& serialized)
            {
                // Format: (x0--x1)(y0--y1)
                std::regex fmt("\\(([0-9]+)--([0-9]+)\\)\\(([0-9]+)--([0-9]+)\\).*");
                std::smatch matches;
                if( !std::regex_match(serialized, matches, fmt) )
                {
                    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to parse Glidos texture coordinates"));
                }

                if( matches.size() != 5 )
                {
                    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to parse Glidos texture coordinates"));
                }

                m_x0 = boost::lexical_cast<uint32_t>(matches[1].str());
                m_x1 = boost::lexical_cast<uint32_t>(matches[2].str());
                m_y0 = boost::lexical_cast<uint32_t>(matches[3].str());
                m_y1 = boost::lexical_cast<uint32_t>(matches[4].str());

                BOOST_ASSERT(m_x0 <= m_x1);
                BOOST_ASSERT(m_y0 <= m_y1);
            }


            bool operator<(const Rectangle& rhs) const
            {
                if( m_x0 != rhs.m_x0 )
                    return m_x0 < rhs.m_x0;
                if( m_x1 != rhs.m_x1 )
                    return m_x1 < rhs.m_x1;
                if( m_y0 != rhs.m_y0 )
                    return m_y0 < rhs.m_y0;
                return m_y1 < rhs.m_y1;
            }


            uint32_t getX0() const noexcept
            {
                return m_x0;
            }


            uint32_t getX1() const noexcept
            {
                return m_x1;
            }


            uint32_t getY0() const noexcept
            {
                return m_y0;
            }


            uint32_t getY1() const noexcept
            {
                return m_y1;
            }

            int getWidth() const
            {
                return m_x1 - m_x0;
            }

            int getHeight() const
            {
                return m_y1 - m_y0;
            }
        private:
            uint32_t m_x0 = 0;
            uint32_t m_x1 = 0;
            uint32_t m_y0 = 0;
            uint32_t m_y1 = 0;
        };


        inline std::ostream& operator<<(std::ostream& str, const Rectangle& r)
        {
            return str << "(" << r.getX0() << "," << r.getY0() << ")-(" << r.getX1() << "," << r.getY1() << ")";
        }


        class TexturePart
        {
        public:
            explicit TexturePart(const std::string& serialized)
            {
                std::vector<std::string> parts;
                boost::algorithm::split(parts, serialized, boost::is_any_of("\\/"));
                if( parts.size() != 2 || parts[0].size() != 32 )
                {
                    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to parse Glidos texture part"));
                }

                m_textureId = parts[0];
                m_rect = Rectangle{parts[1]};
            }


            explicit TexturePart(const std::string& filename, const Rectangle& r)
                : m_textureId{filename}
                , m_rect{r}
            {
            }


            const std::string& getId() const
            {
                return m_textureId;
            }


            const Rectangle& getRectangle() const
            {
                return m_rect;
            }


            bool operator<(const TexturePart& rhs) const
            {
                if( m_textureId != rhs.m_textureId )
                    return m_textureId < rhs.m_textureId;

                return m_rect < rhs.m_rect;
            }


        private:
            std::string m_textureId;
            Rectangle m_rect{};
        };


        class EquivSet
        {
        public:
            explicit EquivSet() = default;


            void add(const std::string& serialized)
            {
                m_set.insert(TexturePart{serialized});
            }


            const std::set<TexturePart>& getParts() const
            {
                return m_set;
            }


        private:
            std::set<TexturePart> m_set;
        };


        class Equiv
        {
        public:
            explicit Equiv() = default;


            void add(const EquivSet& set)
            {
                m_sets.emplace_back(set);
            }


            const std::vector<EquivSet>& getSets() const
            {
                return m_sets;
            }


        private:
            std::vector<EquivSet> m_sets;
        };


        class PathMap
        {
        public:
            explicit PathMap() = default;


            void add(const std::string& md5, const boost::filesystem::path& p)
            {
                Expects(md5.size() == 32);

                if( m_map.find(md5) != m_map.end() )
                {
                    BOOST_THROW_EXCEPTION(std::runtime_error("Texture path mapping already registered"));
                }

                m_map[md5] = p;
            }


            const std::map<std::string, boost::filesystem::path>& getMap() const
            {
                return m_map;
            }


        private:
            std::map<std::string, boost::filesystem::path> m_map;
        };


        class Glidos
        {
        public:
            explicit Glidos(const boost::filesystem::path& baseTxtName)
            {
                BOOST_LOG_TRIVIAL(debug) << "Loading Glidos texture pack from " << baseTxtName;

                PathMap pathMap;
                {
                    std::ifstream txt{baseTxtName.string()};
                    if( !txt.is_open() )
                    {
                        BOOST_THROW_EXCEPTION(std::runtime_error("Failed to open Glidos base file"));
                    }

                    std::string head;
                    std::getline(txt, head);
                    boost::algorithm::trim(head);
                    if( head != "GLIDOS TEXTURE MAPPING" )
                    {
                        BOOST_THROW_EXCEPTION(std::runtime_error("Invalid texture mapping header"));
                    }
                    pathMap = readTextureMapping(txt);
                    m_root = baseTxtName.parent_path() / m_root;
                }

                Equiv equiv;
                {
                    std::ifstream txt{(baseTxtName.parent_path() / "equiv.txt").string()};
                    if( !txt.is_open() )
                    {
                        BOOST_THROW_EXCEPTION(std::runtime_error("Failed to open Glidos equiv file"));
                    }

                    std::string head;
                    std::getline(txt, head);
                    boost::algorithm::trim(head);
                    if( head != "GLIDOS TEXTURE EQUIV" )
                    {
                        BOOST_THROW_EXCEPTION(std::runtime_error("Invalid texture equiv header"));
                    }
                    equiv = readEquiv(txt);
                }

                boost::filesystem::directory_iterator end{};

                for(const auto& texturePath : pathMap.getMap())
                {
                    for(boost::filesystem::directory_iterator it{ m_root / texturePath.second }; it != end; ++it)
                    {
                        Rectangle r;
                        try
                        {
                            r = Rectangle{ it->path().filename().string() };
                            BOOST_LOG_TRIVIAL(debug) << "Registering raw texture " << it->path();
                            m_links[TexturePart(texturePath.first, r)] = it->path();
                        }
                        catch(std::runtime_error&)
                        {
                            BOOST_LOG_TRIVIAL(debug) << "No texture coordinates in filename " << it->path();
                            continue;
                        }

                        if(it->path().extension() == ".png" || it->path().extension() == ".bmp")
                        {
                            continue;
                        }

                        // read link
                        std::ifstream txt{ it->path().string() };
                        if(!txt.is_open())
                        {
                            BOOST_THROW_EXCEPTION(std::runtime_error("Failed to open Glidos text file"));
                        }

                        std::string head;
                        std::getline(txt, head);
                        boost::algorithm::trim(head);
                        if(boost::algorithm::starts_with(head, "TLNK:"))
                        {
                            BOOST_LOG_TRIVIAL(debug) << "Loading Glidos texture link from " << it->path();
                            head.erase(0, head.find(":") + 1);
                            boost::algorithm::trim(head);
                            m_links[TexturePart(texturePath.first, r)] = boost::filesystem::path(m_root / head);
                        }
                        else
                        {
                            BOOST_LOG_TRIVIAL(warning) << "Don't know how to handle " << it->path();
                        }
                    }
                }

                resolveEquiv(equiv);
            }


            void dump() const
            {
                BOOST_LOG_TRIVIAL(info) << "Glidos database dump";
                std::set<std::string> md5s;
                for( const auto& part : m_links )
                {
                    md5s.insert(part.first.getId());
                }

                for( const auto& md5 : md5s )
                {
                    BOOST_LOG_TRIVIAL(info) << "Texture " << md5;
                    auto mappings = getMappingsForTexture(md5);
                    if( mappings.empty() )
                    {
                        BOOST_LOG_TRIVIAL(warning) << "  - No mappings";
                        continue;
                    }

                    for( const auto& mapping : mappings )
                    {
                        BOOST_LOG_TRIVIAL(info) << "  - " << mapping.first << " => " << mapping.second;
                    }
                }
            }


            std::map<Rectangle, boost::filesystem::path> getMappingsForTexture(const std::string& textureId) const
            {
                std::map<Rectangle, boost::filesystem::path> result;

                for( const auto& link : m_links )
                {
                    if( link.first.getId() != textureId )
                        continue;

                    result[link.first.getRectangle()] = link.second;
                }

                return result;
            }


        private:
            PathMap readTextureMapping(std::ifstream& txt)
            {
                std::string line;
                boost::filesystem::path base;
                PathMap result;
                while( std::getline(txt, line) )
                {
                    boost::algorithm::trim(line);
                    if( line.empty() || boost::algorithm::starts_with(line, "//") )
                        continue;

                    if( boost::algorithm::starts_with(line, "ROOT:") )
                    {
                        line.erase(0, line.find(":") + 1);
                        boost::algorithm::trim(line);
                        m_root = line;
                    }
                    else if( boost::algorithm::starts_with(line, "BASE:") )
                    {
                        line.erase(0, line.find(":") + 1);
                        boost::algorithm::trim(line);
                        base = line;
                    }
                    else
                    {
                        std::vector<std::string> parts;
                        boost::algorithm::split(parts, line, boost::is_any_of(" \t"));
                        if( parts.size() != 2 )
                        {
                            BOOST_THROW_EXCEPTION(std::runtime_error("Failed to parse mapping line"));
                        }
                        result.add(parts[0], base / parts[1]);
                    }
                }

                return result;
            }


            static Equiv readEquiv(std::ifstream& txt)
            {
                Equiv result;

                std::string line;
                boost::filesystem::path base;
                while( std::getline(txt, line) )
                {
                    boost::algorithm::trim(line);
                    if( line.empty() || boost::algorithm::starts_with(line, "//") )
                        continue;

                    if( line == "BeginEquiv" )
                    {
                        result.add(readEquivSet(txt));
                    }
                }

                return result;
            }


            static EquivSet readEquivSet(std::ifstream& txt)
            {
                std::string line;
                boost::filesystem::path base;
                EquivSet result;
                while( std::getline(txt, line) )
                {
                    boost::algorithm::trim(line);
                    if( line.empty() || boost::algorithm::starts_with(line, "//") )
                        continue;

                    if( line == "EndEquiv" )
                    {
                        break;
                    }

                    result.add(line);
                }

                return result;
            }


            void resolveEquiv(const Equiv& equiv)
            {
                BOOST_LOG_TRIVIAL(info) << "Resolving equiv sets...";
                for( const auto& set : equiv.getSets() )
                {
                    boost::filesystem::path ref;
                    for( const auto& part : set.getParts() )
                    {
                        auto it = m_links.find(part);
                        if( it != m_links.end() )
                        {
                            ref = it->second;
                            break;
                        }
                    }

                    if( ref.empty() )
                    {
                        BOOST_LOG_TRIVIAL(warning) << "Invalid equiv set: No entry references a known texture part";
                        continue;
                    }

                    for( const auto& part : set.getParts() )
                    {
                        m_links[part] = ref;
                    }
                }
            }


            std::map<TexturePart, boost::filesystem::path> m_links;
            boost::filesystem::path m_root;
        };
    }
}
