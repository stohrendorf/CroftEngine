#include "animation.h"

#include "io/sdlreader.h"

namespace loader::file
{
std::unique_ptr<TransitionCase> TransitionCase::read(io::SDLReader& reader)
{
  std::unique_ptr<TransitionCase> transition{new TransitionCase()};
  transition->firstFrame = core::Frame{static_cast<core::Frame::type>(reader.readU16())};
  transition->lastFrame = core::Frame{static_cast<core::Frame::type>(reader.readU16())};
  transition->targetAnimationIndex = reader.readU16();
  transition->targetFrame = core::Frame{static_cast<core::Frame::type>(reader.readU16())};
  return transition;
}

std::unique_ptr<Transitions> Transitions::read(io::SDLReader& reader)
{
  std::unique_ptr<Transitions> state_change = std::make_unique<Transitions>();
  state_change->stateId = reader.readU16();
  state_change->transitionCaseCount = reader.readU16();
  state_change->firstTransitionCase = reader.readU16();
  return state_change;
}

std::unique_ptr<Animation> Animation::readTr1(io::SDLReader& reader)
{
  return read(reader, false);
}

std::unique_ptr<Animation> Animation::readTr4(io::SDLReader& reader)
{
  return read(reader, true);
}

std::unique_ptr<Animation> Animation::read(io::SDLReader& reader, const bool withLateral)
{
  auto animation = std::make_unique<Animation>();
  animation->poseDataOffset = reader.readU32();
  animation->segmentLength = core::Frame{static_cast<core::Frame::type>(reader.readU8())};
  if(animation->segmentLength == 0_frame)
    animation->segmentLength = 1_frame;
  animation->poseDataSize = reader.readU8();
  animation->state_id = core::AnimStateId{reader.readU16()};

  animation->speed = core::Speed{reader.readI32()};
  animation->acceleration = core::Acceleration{reader.readI32()};
  if(withLateral)
  {
    animation->lateralSpeed = core::Speed{reader.readI32()};
    animation->lateralAcceleration = core::Acceleration{reader.readI32()};
  }

  animation->firstFrame = core::Frame{static_cast<core::Frame::type>(reader.readU16())};
  animation->lastFrame = core::Frame{static_cast<core::Frame::type>(reader.readU16())};
  animation->nextAnimationIndex = reader.readU16();
  animation->nextFrame = core::Frame{static_cast<core::Frame::type>(reader.readU16())};

  animation->transitionsCount = reader.readU16();
  animation->transitionsIndex = reader.readU16();
  animation->animCommandCount = reader.readU16();
  animation->animCommandIndex = reader.readU16();
  return animation;
}

std::unique_ptr<SkeletalModelType> SkeletalModelType::readTr1(io::SDLReader& reader)
{
  std::unique_ptr<SkeletalModelType> moveable{std::make_unique<SkeletalModelType>()};
  moveable->type = static_cast<core::TypeId::type>(reader.readU32());
  moveable->nMeshes = reader.readI16();
  moveable->mesh_base_index = reader.readU16();
  moveable->bone_index = reader.readU32();
  moveable->pose_data_offset = reader.readU32();
  moveable->animation_index = reader.readU16();
  return moveable;
}

std::unique_ptr<SkeletalModelType> SkeletalModelType::readTr5(io::SDLReader& reader)
{
  std::unique_ptr<SkeletalModelType> moveable = readTr1(reader);
  if(reader.readU16() != 0xFFEF)
    BOOST_LOG_TRIVIAL(warning) << "TR5 Moveable: filler has wrong value";
  return moveable;
}
} // namespace loader::file