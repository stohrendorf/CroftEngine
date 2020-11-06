#include "serialization.h"

#include "engine/engine.h"

#include <boost/algorithm/string/replace.hpp>
#include <boost/stacktrace.hpp>

namespace serialization
{
Exception::Exception(const gsl::czstring msg)
    : std::runtime_error{msg}
{
  BOOST_LOG_TRIVIAL(fatal) << "Serialization exception: " << msg;
  BOOST_LOG_TRIVIAL(fatal) << "Stacktrace:\n" << boost::stacktrace::stacktrace();
}

void Serializer::save(const std::string& filename, engine::Engine& engine)
{
  std::ofstream file{filename, std::ios::out | std::ios::trunc};
  Expects(file.is_open());
  ryml::Tree t;
  t.rootref() |= ryml::MAP;
  Serializer ser{t.rootref(), engine, false, nullptr};
  access::callSerializeOrSave(engine, ser);
  ser.processQueues();
  file << ser.node;
}

void Serializer::load(const std::string& filename, engine::Engine& engine)
{
  std::ifstream file{filename, std::ios::in};
  Expects(file.is_open());
  file.seekg(0, std::ios::end);
  const auto size = static_cast<std::size_t>(file.tellg());
  file.seekg(0, std::ios::beg);

  // Read entire file contents.
  std::string buffer;
  buffer.resize(size);
  file.read(&buffer[0], size);
  auto tree = ryml::parse(c4::to_csubstr(filename), c4::to_csubstr(buffer));

  Serializer ser{tree.rootref(), engine, true, nullptr};
  access::callSerializeOrLoad(engine, ser);
  ser.processQueues();
}

void Serializer::processQueues()
{
  while(!m_lazyQueue->empty())
  {
    BOOST_LOG_TRIVIAL(debug) << "Processing serialization queue...";
    auto current = std::exchange(m_lazyQueue, std::make_shared<LazyQueue>());
    while(!current->empty())
    {
      current->front()();
      current->pop();
    }
  }
}

std::string Serializer::getQualifiedKey() const
{
  if(!node.has_key())
  {
    return {};
  }

  std::string result = util::toString(node.key());
  auto cursor = node;
  while(!cursor.is_root() && cursor.has_parent())
  {
    cursor = cursor.parent();
    if(cursor.has_key())
      result = util::toString(cursor.key()) + "::" + result;
    else
      result = "??::" + result;
  }
  return result;
}

void Serializer::ensureIsMap() const
{
  if(!loading)
    node |= ryml::MAP;

  Expects(node.is_map());
}

Serializer Serializer::createMapMemberSerializer(const gsl::not_null<gsl::czstring>& name) const
{
  ensureIsMap();
  auto childNode = node.find_child(c4::to_csubstr(name.get()));
  const bool exists = !childNode.is_seed() && childNode.valid() && childNode.type() != ryml::NOTYPE;
  if(loading)
  {
    if(!exists)
      SERIALIZER_EXCEPTION(std::string{"Node "} + name.get() + " not defined");
  }
  else
  {
    if(exists)
      SERIALIZER_EXCEPTION(std::string{"Node "} + name.get() + " already defined");

    childNode = node.append_child();
    childNode.set_key(node.tree()->copy_to_arena(c4::to_csubstr(name.get())));
  }

  return withNode(childNode);
}

void Serializer::lazy(const LazyCallback& lazyCallback) const
{
  if(loading)
    m_lazyQueue->emplace([lazyCallback = lazyCallback, ser = *this]() { lazyCallback(ser); });
  else
    lazyCallback(*this);
}

Serializer Serializer::operator[](const gsl::czstring name) const
{
  return (*this)[std::string{name}];
}

Serializer Serializer::operator[](const std::string& name) const
{
  ensureIsMap();
  auto existing = node[c4::to_csubstr(name)];
  if(!loading)
  {
    if(existing.is_seed() || !existing.valid() || existing.type() == ryml::NOTYPE)
    {
      auto ser = newChild();
      ser.node.set_key(node.tree()->copy_to_arena(c4::to_csubstr(name)));
      return ser;
    }
    else
    {
      return withNode(existing);
    }
  }
  else
  {
    return withNode(existing);
  }
}

Serializer Serializer::withNode(const ryml::NodeRef& otherNode) const
{
  return Serializer{otherNode, engine, loading, m_lazyQueue};
}

Serializer Serializer::newChild() const
{
  return withNode(node.append_child());
}

void Serializer::tag(const std::string& tag) const
{
  Expects(!tag.empty());

  auto normalizedTag
    = std::string{"!<"}
      + boost::algorithm::replace_all_copy(boost::algorithm::replace_all_copy(tag, ">", "&gt;"), " ", "%20") + ">";
  if(loading)
  {
    if(!node.has_val())
      return;

    if(!node.has_val_tag())
      SERIALIZER_EXCEPTION(("Expected tag \"" + normalizedTag + "\", but got no tag"));

    std::string existingTag = util::toString(node.val_tag());
    if(existingTag != normalizedTag)
      SERIALIZER_EXCEPTION("Expected tag \"" + normalizedTag + "\", but got \"" + existingTag + "\"");
  }
  else
    m_tag = normalizedTag;
}

Serializer::Serializer(const ryml::NodeRef& node,
                       engine::Engine& engine,
                       bool loading,
                       const std::shared_ptr<LazyQueue>& lazyQueue)
    : m_lazyQueue{lazyQueue == nullptr ? std::make_shared<LazyQueue>() : lazyQueue}
    , node{node}
    , engine{engine}
    , loading{loading}
{
  ryml::set_callbacks(ryml::Callbacks{
    nullptr,
    [](size_t length, void* /*hint*/, void* /*user_data*/) -> gsl::owner<void*> { return new char[length]; },
    [](gsl::owner<void*> mem, size_t /*length*/, void* /*user_data*/) { delete[] static_cast<char*>(mem); },
    [](const char* msg, size_t msg_len, void* /*user_data*/) {
      const std::string msgStr{msg, msg_len};
      SERIALIZER_EXCEPTION(msgStr);
    }});
}

Serializer::~Serializer()
{
  if(!loading && node.has_val())
    node.set_val_tag(node.tree()->copy_to_arena(c4::to_csubstr(m_tag)));
}
} // namespace serialization
