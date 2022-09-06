#pragma once

#include <algorithm>
#include <boost/throw_exception.hpp>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace util
{

template<typename T>
void insertUnique(std::vector<T>& set, const T& element)
{
  if(std::find(set.begin(), set.end(), element) == set.end())
    set.push_back(element);
}

template<typename T>
bool eraseUnique(std::vector<T>& set, const T& element)
{
  auto it = std::remove(set.begin(), set.end(), element);
  if(it == set.end())
  {
    return false;
  }

  if(it != std::prev(set.end()))
  {
    std::swap(*it, *std::prev(set.end()));
  }
  set.pop_back();
  return true;
}

template<typename T>
bool containsUnique(const std::vector<T>& set, const T& element)
{
  return std::find(set.begin(), set.end(), element) != set.end();
}

template<typename T, typename U>
U& getOrCreate(std::vector<std::pair<T, U>>& map, const T& key)
{
  if(auto it = std::find_if(map.begin(),
                            map.end(),
                            [&key](const auto& element)
                            {
                              return element.first == key;
                            });
     it != map.end())
  {
    return it->second;
  }

  return map.emplace_back(key, U{}).second;
}

template<typename T, typename U>
std::optional<std::reference_wrapper<U>> tryGet(std::vector<std::pair<T, U>>& map, const T& key)
{
  if(auto it = std::find_if(map.begin(),
                            map.end(),
                            [&key](const auto& element)
                            {
                              return element.first == key;
                            });
     it != map.end())
  {
    return it->second;
  }

  return std::nullopt;
}

template<typename T, typename U>
std::optional<std::reference_wrapper<const U>> tryGet(const std::vector<std::pair<T, U>>& map, const T& key)
{
  if(auto it = std::find_if(map.begin(),
                            map.end(),
                            [&key](const auto& element)
                            {
                              return element.first == key;
                            });
     it != map.end())
  {
    return it->second;
  }

  return std::nullopt;
}

template<typename T, typename U>
bool contains(std::vector<std::pair<T, U>>& map, const T& key)
{
  return std::find_if(map.begin(),
                      map.end(),
                      [&key](const auto& element)
                      {
                        return element.first == key;
                      })
         != map.end();
}
} // namespace util
