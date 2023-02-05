#include "meta.h"

#include "serialization/serialization.h"
#include "serialization/vector.h"

namespace gameflow
{
void Meta::serialize(const serialization::Serializer<Meta>& ser) const
{
  ser(S_NV("title", title), S_NV("author", author), S_NV("urls", urls));
}

void Meta::deserialize(const serialization::Deserializer<Meta>& ser)
{
  ser(S_NV("title", title), S_NV("author", author), S_NV("urls", urls));
}
} // namespace gameflow
