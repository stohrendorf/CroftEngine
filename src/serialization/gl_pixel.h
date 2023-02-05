#pragma once

#include "serialization.h"

#include <gl/pixel.h>

namespace serialization
{
template<typename T,
         glm::length_t Channels,
         gl::api::PixelFormat PixelFormat,
         gl::api::SizedInternalFormat SizedInternalFormat,
         bool Premultiplied,
         typename TContext>
void serialize(gl::Pixel<T, Channels, PixelFormat, SizedInternalFormat, Premultiplied>& data,
               const Serializer<TContext>& ser)
{
  ser.tag("pixel");
  ser.node |= ryml::SEQ;
  for(glm::length_t i = 0; i < Channels; ++i)
  {
    const auto tmp = ser.newChild();
    access<T>::callSerialize(data.channels[i], tmp);
  }
}

template<typename T,
         glm::length_t Channels,
         gl::api::PixelFormat PixelFormat,
         gl::api::SizedInternalFormat SizedInternalFormat,
         bool Premultiplied,
         typename TContext>
void deserialize(gl::Pixel<T, Channels, PixelFormat, SizedInternalFormat, Premultiplied>& data,
                 const Deserializer<TContext>& ser)
{
  ser.tag("pixel");
  Expects(ser.node.is_seq());
  Expects(ser.node.num_children() == Channels);
  for(glm::length_t i = 0; i < Channels; ++i)
  {
    access<T>::callSerialize(data.channels[i], ser.withNode(ser.node[i]));
  }
}
} // namespace serialization
