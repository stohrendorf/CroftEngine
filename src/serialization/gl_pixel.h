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
         size_t Alignment,
         typename TContext>
void serialize(gl::Pixel<T, Channels, PixelFormat, SizedInternalFormat, Premultiplied, Alignment>& data,
               const Serializer<TContext>& ser)
{
  ser.tag("pixel");
  ser.node |= ryml::SEQ;
  for(glm::length_t i = 0; i < Channels; ++i)
  {
    const auto tmp = ser.newChild();
    access<T, false>::dispatch(data.channels[i], tmp);
  }
}

template<typename T,
         glm::length_t Channels,
         gl::api::PixelFormat PixelFormat,
         gl::api::SizedInternalFormat SizedInternalFormat,
         bool Premultiplied,
         size_t Alignment,
         typename TContext>
void deserialize(gl::Pixel<T, Channels, PixelFormat, SizedInternalFormat, Premultiplied, Alignment>& data,
                 const Deserializer<TContext>& ser)
{
  ser.tag("pixel");
  gsl_Expects(ser.node.is_seq());
  gsl_Expects(ser.node.num_children() == Channels);
  for(glm::length_t i = 0; i < Channels; ++i)
  {
    access<T, true>::dispatch(data.channels[i], ser.withNode(ser.node[i]));
  }
}
} // namespace serialization
