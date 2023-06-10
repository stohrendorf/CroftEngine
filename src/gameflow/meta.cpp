#include "meta.h"

#include "serialization/serialization.h"
#include "serialization/vector.h"

#include <exception>

namespace gameflow
{
void Meta::serialize(const serialization::Serializer<Meta>& ser) const
{
  ser(S_NV("title", title), S_NV("author", author), S_NV("urls", urls), S_NV("downloadSoundtrack", downloadSoundtrack));
}

void Meta::deserialize(const serialization::Deserializer<Meta>& ser)
{
  ser(S_NV("title", title), S_NV("author", author), S_NV("urls", urls), S_NV("downloadSoundtrack", downloadSoundtrack));
}
} // namespace gameflow
