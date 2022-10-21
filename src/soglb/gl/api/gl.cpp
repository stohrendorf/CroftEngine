#include "gl.hpp"

#include "gl_api_provider.hpp"
namespace gl::api
{
void blendFunc(BlendingFactor sfactor, BlendingFactor dfactor)
{
  return glBlendFunc(static_cast<GLenum>(sfactor), static_cast<GLenum>(dfactor));
}
void clear(core::Bitfield<ClearBufferMask> mask)
{
  return glClear(mask.value());
}
void clearColor(float red, float green, float blue, float alpha)
{
  return glClearColor(
    static_cast<GLfloat>(red), static_cast<GLfloat>(green), static_cast<GLfloat>(blue), static_cast<GLfloat>(alpha));
}
void clearDepth(double depth)
{
  return glClearDepth(static_cast<GLdouble>(depth));
}
void clearStencil(int32_t s)
{
  return glClearStencil(static_cast<GLint>(s));
}
SyncStatus clientWaitSync(core::Sync sync, core::Bitfield<SyncObjectMask> flags, uint64_t timeout)
{
  return static_cast<SyncStatus>(
    glClientWaitSync(static_cast<GLsync>(sync), flags.value(), static_cast<GLuint64>(timeout)));
}
void colorMask(bool red, bool green, bool blue, bool alpha)
{
  return glColorMask(static_cast<GLboolean>(red),
                     static_cast<GLboolean>(green),
                     static_cast<GLboolean>(blue),
                     static_cast<GLboolean>(alpha));
}
void cullFace(TriangleFace mode)
{
  return glCullFace(static_cast<GLenum>(mode));
}
void deleteSync(core::Sync sync)
{
  return glDeleteSync(static_cast<GLsync>(sync));
}
void depthFunc(DepthFunction func)
{
  return glDepthFunc(static_cast<GLenum>(func));
}
void depthMask(bool flag)
{
  return glDepthMask(static_cast<GLboolean>(flag));
}
void depthRange(double n, double f)
{
  return glDepthRange(static_cast<GLdouble>(n), static_cast<GLdouble>(f));
}
void disable(EnableCap cap)
{
  return glDisable(static_cast<GLenum>(cap));
}
void drawBuffer(DrawBufferMode buf)
{
  return glDrawBuffer(static_cast<GLenum>(buf));
}
void enable(EnableCap cap)
{
  return glEnable(static_cast<GLenum>(cap));
}
core::Sync fenceSync(SyncCondition condition, core::Bitfield<SyncBehaviorFlags> flags)
{
  return static_cast<core::Sync>(glFenceSync(static_cast<GLenum>(condition), flags.value()));
}
void finish()
{
  return glFinish();
}
void flush()
{
  return glFlush();
}
void frontFace(FrontFaceDirection mode)
{
  return glFrontFace(static_cast<GLenum>(mode));
}
void getBoolean(GetPName pname, bool* data)
{
  return glGetBooleanv(static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLboolean*>(data)));
}
void getDouble(GetPName pname, double* data)
{
  return glGetDoublev(static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLdouble*>(data)));
}
ErrorCode getError()
{
  return static_cast<ErrorCode>(glGetError());
}
void getFloat(GetPName pname, float* data)
{
  return glGetFloatv(static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLfloat*>(data)));
}
uint64_t getImageHandle(uint32_t texture, int32_t level, bool layered, int32_t layer, PixelFormat format)
{
  return static_cast<uint64_t>(glGetImageHandleARB(static_cast<GLuint>(texture),
                                                   static_cast<GLint>(level),
                                                   static_cast<GLboolean>(layered),
                                                   static_cast<GLint>(layer),
                                                   static_cast<GLenum>(format)));
}
void getInteger64(GetPName pname, int64_t* data)
{
  return glGetInteger64v(static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint64*>(data)));
}
void getInteger(GetPName pname, int32_t* data)
{
  return glGetIntegerv(static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint*>(data)));
}
const uint8_t* getString(StringName name)
{
  return static_cast<const uint8_t*>(glGetString(static_cast<GLenum>(name)));
}
void getSync(core::Sync sync, SyncParameterName pname, core::SizeType count, core::SizeType* length, int32_t* values)
{
  return glGetSynciv(static_cast<GLsync>(sync),
                     static_cast<GLenum>(pname),
                     static_cast<GLsizei>(count),
                     detail::constAway(reinterpret_cast<GLsizei*>(length)),
                     detail::constAway(reinterpret_cast<GLint*>(values)));
}
void getTexImage(TextureTarget target, int32_t level, PixelFormat format, PixelType type, void* pixels)
{
  return glGetTexImage(static_cast<GLenum>(target),
                       static_cast<GLint>(level),
                       static_cast<GLenum>(format),
                       static_cast<GLenum>(type),
                       detail::constAway(pixels));
}
void getTexLevelParameter(TextureTarget target, int32_t level, GetTextureParameter pname, float* params)
{
  return glGetTexLevelParameterfv(static_cast<GLenum>(target),
                                  static_cast<GLint>(level),
                                  static_cast<GLenum>(pname),
                                  detail::constAway(reinterpret_cast<GLfloat*>(params)));
}
void getTexLevelParameter(TextureTarget target, int32_t level, GetTextureParameter pname, int32_t* params)
{
  return glGetTexLevelParameteriv(static_cast<GLenum>(target),
                                  static_cast<GLint>(level),
                                  static_cast<GLenum>(pname),
                                  detail::constAway(reinterpret_cast<GLint*>(params)));
}
void getTexParameter(TextureTarget target, GetTextureParameter pname, float* params)
{
  return glGetTexParameterfv(
    static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLfloat*>(params)));
}
void getTexParameter(TextureTarget target, GetTextureParameter pname, int32_t* params)
{
  return glGetTexParameteriv(
    static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint*>(params)));
}
uint64_t getTextureHandle(uint32_t texture)
{
  return static_cast<uint64_t>(glGetTextureHandleARB(static_cast<GLuint>(texture)));
}
uint64_t getTextureSamplerHandle(uint32_t texture, uint32_t sampler)
{
  return static_cast<uint64_t>(
    glGetTextureSamplerHandleARB(static_cast<GLuint>(texture), static_cast<GLuint>(sampler)));
}
void getVertexAttribL(uint32_t index, VertexAttribEnum pname, uint64_t* params)
{
  return glGetVertexAttribLui64vARB(
    static_cast<GLuint>(index), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLuint64EXT*>(params)));
}
void hint(HintTarget target, HintMode mode)
{
  return glHint(static_cast<GLenum>(target), static_cast<GLenum>(mode));
}
bool isEnabled(EnableCap cap)
{
  return static_cast<bool>(glIsEnabled(static_cast<GLenum>(cap)));
}
bool isImageHandleResident(uint64_t handle)
{
  return static_cast<bool>(glIsImageHandleResidentARB(static_cast<GLuint64>(handle)));
}
bool isSync(core::Sync sync)
{
  return static_cast<bool>(glIsSync(static_cast<GLsync>(sync)));
}
bool isTextureHandleResident(uint64_t handle)
{
  return static_cast<bool>(glIsTextureHandleResidentARB(static_cast<GLuint64>(handle)));
}
void lineWidth(float width)
{
  return glLineWidth(static_cast<GLfloat>(width));
}
void logicOp(LogicOp opcode)
{
  return glLogicOp(static_cast<GLenum>(opcode));
}
void makeImageHandleNonResident(uint64_t handle)
{
  return glMakeImageHandleNonResidentARB(static_cast<GLuint64>(handle));
}
void makeImageHandleResident(uint64_t handle, core::EnumType access)
{
  return glMakeImageHandleResidentARB(static_cast<GLuint64>(handle), static_cast<GLenum>(access));
}
void makeTextureHandleNonResident(uint64_t handle)
{
  return glMakeTextureHandleNonResidentARB(static_cast<GLuint64>(handle));
}
void makeTextureHandleResident(uint64_t handle)
{
  return glMakeTextureHandleResidentARB(static_cast<GLuint64>(handle));
}
void pixelStore(PixelStoreParameter pname, float param)
{
  return glPixelStoref(static_cast<GLenum>(pname), static_cast<GLfloat>(param));
}
void pixelStore(PixelStoreParameter pname, int32_t param)
{
  return glPixelStorei(static_cast<GLenum>(pname), static_cast<GLint>(param));
}
void pointSize(float size)
{
  return glPointSize(static_cast<GLfloat>(size));
}
void polygonMode(TriangleFace face, PolygonMode mode)
{
  return glPolygonMode(static_cast<GLenum>(face), static_cast<GLenum>(mode));
}
void programUniformHandle(uint32_t program, int32_t location, uint64_t value)
{
  return glProgramUniformHandleui64ARB(
    static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLuint64>(value));
}
void programUniformHandle(uint32_t program, int32_t location, core::SizeType count, const uint64_t* values)
{
  return glProgramUniformHandleui64vARB(static_cast<GLuint>(program),
                                        static_cast<GLint>(location),
                                        static_cast<GLsizei>(count),
                                        detail::constAway(reinterpret_cast<const GLuint64*>(values)));
}
void readBuffer(ReadBufferMode src)
{
  return glReadBuffer(static_cast<GLenum>(src));
}
void readPixel(
  int32_t x, int32_t y, core::SizeType width, core::SizeType height, PixelFormat format, PixelType type, void* pixels)
{
  return glReadPixels(static_cast<GLint>(x),
                      static_cast<GLint>(y),
                      static_cast<GLsizei>(width),
                      static_cast<GLsizei>(height),
                      static_cast<GLenum>(format),
                      static_cast<GLenum>(type),
                      detail::constAway(pixels));
}
void scissor(int32_t x, int32_t y, core::SizeType width, core::SizeType height)
{
  return glScissor(
    static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(width), static_cast<GLsizei>(height));
}
void stencilFunc(StencilFunction func, int32_t ref, uint32_t mask)
{
  return glStencilFunc(static_cast<GLenum>(func), static_cast<GLint>(ref), static_cast<GLuint>(mask));
}
void stencilMask(uint32_t mask)
{
  return glStencilMask(static_cast<GLuint>(mask));
}
void stencilOp(StencilOp fail, StencilOp zfail, StencilOp zpass)
{
  return glStencilOp(static_cast<GLenum>(fail), static_cast<GLenum>(zfail), static_cast<GLenum>(zpass));
}
void texImage1D(TextureTarget target,
                int32_t level,
                int32_t internalformat,
                core::SizeType width,
                int32_t border,
                PixelFormat format,
                PixelType type,
                const void* pixels)
{
  return glTexImage1D(static_cast<GLenum>(target),
                      static_cast<GLint>(level),
                      static_cast<GLint>(internalformat),
                      static_cast<GLsizei>(width),
                      static_cast<GLint>(border),
                      static_cast<GLenum>(format),
                      static_cast<GLenum>(type),
                      detail::constAway(pixels));
}
void texImage2D(TextureTarget target,
                int32_t level,
                InternalFormat internalformat,
                core::SizeType width,
                core::SizeType height,
                int32_t border,
                PixelFormat format,
                PixelType type,
                const void* pixels)
{
  return glTexImage2D(static_cast<GLenum>(target),
                      static_cast<GLint>(level),
                      static_cast<GLenum>(internalformat),
                      static_cast<GLsizei>(width),
                      static_cast<GLsizei>(height),
                      static_cast<GLint>(border),
                      static_cast<GLenum>(format),
                      static_cast<GLenum>(type),
                      detail::constAway(pixels));
}
void texParameter(TextureTarget target, TextureParameterName pname, float param)
{
  return glTexParameterf(static_cast<GLenum>(target), static_cast<GLenum>(pname), static_cast<GLfloat>(param));
}
void texParameter(TextureTarget target, TextureParameterName pname, const float* params)
{
  return glTexParameterfv(static_cast<GLenum>(target),
                          static_cast<GLenum>(pname),
                          detail::constAway(reinterpret_cast<const GLfloat*>(params)));
}
void texParameter(TextureTarget target, TextureParameterName pname, int32_t param)
{
  return glTexParameteri(static_cast<GLenum>(target), static_cast<GLenum>(pname), static_cast<GLint>(param));
}
void texParameter(TextureTarget target, TextureParameterName pname, const int32_t* params)
{
  return glTexParameteriv(
    static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<const GLint*>(params)));
}
void uniformHandle(int32_t location, uint64_t value)
{
  return glUniformHandleui64ARB(static_cast<GLint>(location), static_cast<GLuint64>(value));
}
void uniformHandle(int32_t location, core::SizeType count, const uint64_t* value)
{
  return glUniformHandleui64vARB(static_cast<GLint>(location),
                                 static_cast<GLsizei>(count),
                                 detail::constAway(reinterpret_cast<const GLuint64*>(value)));
}
void vertexAttribL1(uint32_t index, uint64_t x)
{
  return glVertexAttribL1ui64ARB(static_cast<GLuint>(index), static_cast<GLuint64EXT>(x));
}
void vertexAttribL1(uint32_t index, const uint64_t* v)
{
  return glVertexAttribL1ui64vARB(static_cast<GLuint>(index),
                                  detail::constAway(reinterpret_cast<const GLuint64EXT*>(v)));
}
void viewport(int32_t x, int32_t y, core::SizeType width, core::SizeType height)
{
  return glViewport(
    static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(width), static_cast<GLsizei>(height));
}
void waitSync(core::Sync sync, core::Bitfield<SyncBehaviorFlags> flags, uint64_t timeout)
{
  return glWaitSync(static_cast<GLsync>(sync), flags.value(), static_cast<GLuint64>(timeout));
}
void bindTexture(TextureTarget target, uint32_t texture)
{
  return glBindTexture(static_cast<GLenum>(target), static_cast<GLuint>(texture));
}
void copyTexImage1D(TextureTarget target,
                    int32_t level,
                    InternalFormat internalformat,
                    int32_t x,
                    int32_t y,
                    core::SizeType width,
                    int32_t border)
{
  return glCopyTexImage1D(static_cast<GLenum>(target),
                          static_cast<GLint>(level),
                          static_cast<GLenum>(internalformat),
                          static_cast<GLint>(x),
                          static_cast<GLint>(y),
                          static_cast<GLsizei>(width),
                          static_cast<GLint>(border));
}
void copyTexImage2D(TextureTarget target,
                    int32_t level,
                    InternalFormat internalformat,
                    int32_t x,
                    int32_t y,
                    core::SizeType width,
                    core::SizeType height,
                    int32_t border)
{
  return glCopyTexImage2D(static_cast<GLenum>(target),
                          static_cast<GLint>(level),
                          static_cast<GLenum>(internalformat),
                          static_cast<GLint>(x),
                          static_cast<GLint>(y),
                          static_cast<GLsizei>(width),
                          static_cast<GLsizei>(height),
                          static_cast<GLint>(border));
}
void copyTexSubImage1D(TextureTarget target, int32_t level, int32_t xoffset, int32_t x, int32_t y, core::SizeType width)
{
  return glCopyTexSubImage1D(static_cast<GLenum>(target),
                             static_cast<GLint>(level),
                             static_cast<GLint>(xoffset),
                             static_cast<GLint>(x),
                             static_cast<GLint>(y),
                             static_cast<GLsizei>(width));
}
void copyTexSubImage2D(TextureTarget target,
                       int32_t level,
                       int32_t xoffset,
                       int32_t yoffset,
                       int32_t x,
                       int32_t y,
                       core::SizeType width,
                       core::SizeType height)
{
  return glCopyTexSubImage2D(static_cast<GLenum>(target),
                             static_cast<GLint>(level),
                             static_cast<GLint>(xoffset),
                             static_cast<GLint>(yoffset),
                             static_cast<GLint>(x),
                             static_cast<GLint>(y),
                             static_cast<GLsizei>(width),
                             static_cast<GLsizei>(height));
}
void deleteTextures(core::SizeType n, const uint32_t* textures)
{
  return glDeleteTextures(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<const GLuint*>(textures)));
}
void drawArrays(PrimitiveType mode, int32_t first, core::SizeType count)
{
  return glDrawArrays(static_cast<GLenum>(mode), static_cast<GLint>(first), static_cast<GLsizei>(count));
}
void drawElements(PrimitiveType mode, core::SizeType count, DrawElementsType type, const void* indices)
{
  return glDrawElements(
    static_cast<GLenum>(mode), static_cast<GLsizei>(count), static_cast<GLenum>(type), detail::constAway(indices));
}
void genTextures(core::SizeType n, uint32_t* textures)
{
  return glGenTextures(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<GLuint*>(textures)));
}
bool isTexture(uint32_t texture)
{
  return static_cast<bool>(glIsTexture(static_cast<GLuint>(texture)));
}
void polygonOffset(float factor, float units)
{
  return glPolygonOffset(static_cast<GLfloat>(factor), static_cast<GLfloat>(units));
}
void texSubImage1D(TextureTarget target,
                   int32_t level,
                   int32_t xoffset,
                   core::SizeType width,
                   PixelFormat format,
                   PixelType type,
                   const void* pixels)
{
  return glTexSubImage1D(static_cast<GLenum>(target),
                         static_cast<GLint>(level),
                         static_cast<GLint>(xoffset),
                         static_cast<GLsizei>(width),
                         static_cast<GLenum>(format),
                         static_cast<GLenum>(type),
                         detail::constAway(pixels));
}
void texSubImage2D(TextureTarget target,
                   int32_t level,
                   int32_t xoffset,
                   int32_t yoffset,
                   core::SizeType width,
                   core::SizeType height,
                   PixelFormat format,
                   PixelType type,
                   const void* pixels)
{
  return glTexSubImage2D(static_cast<GLenum>(target),
                         static_cast<GLint>(level),
                         static_cast<GLint>(xoffset),
                         static_cast<GLint>(yoffset),
                         static_cast<GLsizei>(width),
                         static_cast<GLsizei>(height),
                         static_cast<GLenum>(format),
                         static_cast<GLenum>(type),
                         detail::constAway(pixels));
}
void getPointer(GetPointervPName pname, void** params)
{
  return glGetPointerv(static_cast<GLenum>(pname), detail::constAway(params));
}
void copyTexSubImage3D(TextureTarget target,
                       int32_t level,
                       int32_t xoffset,
                       int32_t yoffset,
                       int32_t zoffset,
                       int32_t x,
                       int32_t y,
                       core::SizeType width,
                       core::SizeType height)
{
  return glCopyTexSubImage3D(static_cast<GLenum>(target),
                             static_cast<GLint>(level),
                             static_cast<GLint>(xoffset),
                             static_cast<GLint>(yoffset),
                             static_cast<GLint>(zoffset),
                             static_cast<GLint>(x),
                             static_cast<GLint>(y),
                             static_cast<GLsizei>(width),
                             static_cast<GLsizei>(height));
}
void drawRangeElements(
  PrimitiveType mode, uint32_t start, uint32_t end, core::SizeType count, DrawElementsType type, const void* indices)
{
  return glDrawRangeElements(static_cast<GLenum>(mode),
                             static_cast<GLuint>(start),
                             static_cast<GLuint>(end),
                             static_cast<GLsizei>(count),
                             static_cast<GLenum>(type),
                             detail::constAway(indices));
}
void texImage3D(TextureTarget target,
                int32_t level,
                InternalFormat internalformat,
                core::SizeType width,
                core::SizeType height,
                core::SizeType depth,
                int32_t border,
                PixelFormat format,
                PixelType type,
                const void* pixels)
{
  return glTexImage3D(static_cast<GLenum>(target),
                      static_cast<GLint>(level),
                      static_cast<GLenum>(internalformat),
                      static_cast<GLsizei>(width),
                      static_cast<GLsizei>(height),
                      static_cast<GLsizei>(depth),
                      static_cast<GLint>(border),
                      static_cast<GLenum>(format),
                      static_cast<GLenum>(type),
                      detail::constAway(pixels));
}
void texSubImage3D(TextureTarget target,
                   int32_t level,
                   int32_t xoffset,
                   int32_t yoffset,
                   int32_t zoffset,
                   core::SizeType width,
                   core::SizeType height,
                   core::SizeType depth,
                   PixelFormat format,
                   PixelType type,
                   const void* pixels)
{
  return glTexSubImage3D(static_cast<GLenum>(target),
                         static_cast<GLint>(level),
                         static_cast<GLint>(xoffset),
                         static_cast<GLint>(yoffset),
                         static_cast<GLint>(zoffset),
                         static_cast<GLsizei>(width),
                         static_cast<GLsizei>(height),
                         static_cast<GLsizei>(depth),
                         static_cast<GLenum>(format),
                         static_cast<GLenum>(type),
                         detail::constAway(pixels));
}
void activeTexture(TextureUnit texture)
{
  return glActiveTexture(static_cast<GLenum>(texture));
}
void compressedTexImage1D(TextureTarget target,
                          int32_t level,
                          InternalFormat internalformat,
                          core::SizeType width,
                          int32_t border,
                          core::SizeType imageSize,
                          const void* data)
{
  return glCompressedTexImage1D(static_cast<GLenum>(target),
                                static_cast<GLint>(level),
                                static_cast<GLenum>(internalformat),
                                static_cast<GLsizei>(width),
                                static_cast<GLint>(border),
                                static_cast<GLsizei>(imageSize),
                                detail::constAway(data));
}
void compressedTexImage2D(TextureTarget target,
                          int32_t level,
                          InternalFormat internalformat,
                          core::SizeType width,
                          core::SizeType height,
                          int32_t border,
                          core::SizeType imageSize,
                          const void* data)
{
  return glCompressedTexImage2D(static_cast<GLenum>(target),
                                static_cast<GLint>(level),
                                static_cast<GLenum>(internalformat),
                                static_cast<GLsizei>(width),
                                static_cast<GLsizei>(height),
                                static_cast<GLint>(border),
                                static_cast<GLsizei>(imageSize),
                                detail::constAway(data));
}
void compressedTexImage3D(TextureTarget target,
                          int32_t level,
                          InternalFormat internalformat,
                          core::SizeType width,
                          core::SizeType height,
                          core::SizeType depth,
                          int32_t border,
                          core::SizeType imageSize,
                          const void* data)
{
  return glCompressedTexImage3D(static_cast<GLenum>(target),
                                static_cast<GLint>(level),
                                static_cast<GLenum>(internalformat),
                                static_cast<GLsizei>(width),
                                static_cast<GLsizei>(height),
                                static_cast<GLsizei>(depth),
                                static_cast<GLint>(border),
                                static_cast<GLsizei>(imageSize),
                                detail::constAway(data));
}
void compressedTexSubImage1D(TextureTarget target,
                             int32_t level,
                             int32_t xoffset,
                             core::SizeType width,
                             InternalFormat format,
                             core::SizeType imageSize,
                             const void* data)
{
  return glCompressedTexSubImage1D(static_cast<GLenum>(target),
                                   static_cast<GLint>(level),
                                   static_cast<GLint>(xoffset),
                                   static_cast<GLsizei>(width),
                                   static_cast<GLenum>(format),
                                   static_cast<GLsizei>(imageSize),
                                   detail::constAway(data));
}
void compressedTexSubImage2D(TextureTarget target,
                             int32_t level,
                             int32_t xoffset,
                             int32_t yoffset,
                             core::SizeType width,
                             core::SizeType height,
                             InternalFormat format,
                             core::SizeType imageSize,
                             const void* data)
{
  return glCompressedTexSubImage2D(static_cast<GLenum>(target),
                                   static_cast<GLint>(level),
                                   static_cast<GLint>(xoffset),
                                   static_cast<GLint>(yoffset),
                                   static_cast<GLsizei>(width),
                                   static_cast<GLsizei>(height),
                                   static_cast<GLenum>(format),
                                   static_cast<GLsizei>(imageSize),
                                   detail::constAway(data));
}
void compressedTexSubImage3D(TextureTarget target,
                             int32_t level,
                             int32_t xoffset,
                             int32_t yoffset,
                             int32_t zoffset,
                             core::SizeType width,
                             core::SizeType height,
                             core::SizeType depth,
                             InternalFormat format,
                             core::SizeType imageSize,
                             const void* data)
{
  return glCompressedTexSubImage3D(static_cast<GLenum>(target),
                                   static_cast<GLint>(level),
                                   static_cast<GLint>(xoffset),
                                   static_cast<GLint>(yoffset),
                                   static_cast<GLint>(zoffset),
                                   static_cast<GLsizei>(width),
                                   static_cast<GLsizei>(height),
                                   static_cast<GLsizei>(depth),
                                   static_cast<GLenum>(format),
                                   static_cast<GLsizei>(imageSize),
                                   detail::constAway(data));
}
void getCompressedTexImage(TextureTarget target, int32_t level, void* img)
{
  return glGetCompressedTexImage(static_cast<GLenum>(target), static_cast<GLint>(level), detail::constAway(img));
}
void sampleCoverage(float value, bool invert)
{
  return glSampleCoverage(static_cast<GLfloat>(value), static_cast<GLboolean>(invert));
}
void blendColor(float red, float green, float blue, float alpha)
{
  return glBlendColor(
    static_cast<GLfloat>(red), static_cast<GLfloat>(green), static_cast<GLfloat>(blue), static_cast<GLfloat>(alpha));
}
void blendEquation(BlendEquationModeEXT mode)
{
  return glBlendEquation(static_cast<GLenum>(mode));
}
void blendFuncSeparate(BlendingFactor sfactorRGB,
                       BlendingFactor dfactorRGB,
                       BlendingFactor sfactorAlpha,
                       BlendingFactor dfactorAlpha)
{
  return glBlendFuncSeparate(static_cast<GLenum>(sfactorRGB),
                             static_cast<GLenum>(dfactorRGB),
                             static_cast<GLenum>(sfactorAlpha),
                             static_cast<GLenum>(dfactorAlpha));
}
void multiDrawArrays(PrimitiveType mode, const int32_t* first, const core::SizeType* count, core::SizeType drawcount)
{
  return glMultiDrawArrays(static_cast<GLenum>(mode),
                           detail::constAway(reinterpret_cast<const GLint*>(first)),
                           detail::constAway(reinterpret_cast<const GLsizei*>(count)),
                           static_cast<GLsizei>(drawcount));
}
void multiDrawElements(PrimitiveType mode,
                       const core::SizeType* count,
                       DrawElementsType type,
                       const void* const* indices,
                       core::SizeType drawcount)
{
  return glMultiDrawElements(static_cast<GLenum>(mode),
                             detail::constAway(reinterpret_cast<const GLsizei*>(count)),
                             static_cast<GLenum>(type),
                             detail::constAway(indices),
                             static_cast<GLsizei>(drawcount));
}
void pointParameter(PointParameterName pname, float param)
{
  return glPointParameterf(static_cast<GLenum>(pname), static_cast<GLfloat>(param));
}
void pointParameter(PointParameterName pname, const float* params)
{
  return glPointParameterfv(static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<const GLfloat*>(params)));
}
void pointParameter(PointParameterName pname, int32_t param)
{
  return glPointParameteri(static_cast<GLenum>(pname), static_cast<GLint>(param));
}
void pointParameter(PointParameterName pname, const int32_t* params)
{
  return glPointParameteriv(static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<const GLint*>(params)));
}
void beginQuery(QueryTarget target, uint32_t id)
{
  return glBeginQuery(static_cast<GLenum>(target), static_cast<GLuint>(id));
}
void bindBuffer(BufferTarget target, uint32_t buffer)
{
  return glBindBuffer(static_cast<GLenum>(target), static_cast<GLuint>(buffer));
}
void bufferData(BufferTarget target, std::size_t size, const void* data, BufferUsage usage)
{
  return glBufferData(
    static_cast<GLenum>(target), static_cast<GLsizeiptr>(size), detail::constAway(data), static_cast<GLenum>(usage));
}
void bufferSubData(BufferTarget target, std::intptr_t offset, std::size_t size, const void* data)
{
  return glBufferSubData(
    static_cast<GLenum>(target), static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), detail::constAway(data));
}
void deleteBuffers(core::SizeType n, const uint32_t* buffers)
{
  return glDeleteBuffers(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<const GLuint*>(buffers)));
}
void deleteQueries(core::SizeType n, const uint32_t* ids)
{
  return glDeleteQueries(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<const GLuint*>(ids)));
}
void endQuery(QueryTarget target)
{
  return glEndQuery(static_cast<GLenum>(target));
}
void genBuffers(core::SizeType n, uint32_t* buffers)
{
  return glGenBuffers(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<GLuint*>(buffers)));
}
void genQueries(core::SizeType n, uint32_t* ids)
{
  return glGenQueries(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<GLuint*>(ids)));
}
void getBufferParameter(BufferTarget target, BufferPName pname, int32_t* params)
{
  return glGetBufferParameteriv(
    static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint*>(params)));
}
void getBufferPointer(BufferTarget target, BufferPointerName pname, void** params)
{
  return glGetBufferPointerv(static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(params));
}
void getBufferSubData(BufferTarget target, std::intptr_t offset, std::size_t size, void* data)
{
  return glGetBufferSubData(
    static_cast<GLenum>(target), static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), detail::constAway(data));
}
void getQueryObject(uint32_t id, QueryObjectParameterName pname, int32_t* params)
{
  return glGetQueryObjectiv(
    static_cast<GLuint>(id), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint*>(params)));
}
void getQueryObject(uint32_t id, QueryObjectParameterName pname, uint32_t* params)
{
  return glGetQueryObjectuiv(
    static_cast<GLuint>(id), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLuint*>(params)));
}
void getQuery(QueryTarget target, QueryParameterName pname, int32_t* params)
{
  return glGetQueryiv(
    static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint*>(params)));
}
bool isBuffer(uint32_t buffer)
{
  return static_cast<bool>(glIsBuffer(static_cast<GLuint>(buffer)));
}
bool isQuery(uint32_t id)
{
  return static_cast<bool>(glIsQuery(static_cast<GLuint>(id)));
}
void* mapBuffer(BufferTarget target, BufferAccess access)
{
  return glMapBuffer(static_cast<GLenum>(target), static_cast<GLenum>(access));
}
bool unmapBuffer(BufferTarget target)
{
  return static_cast<bool>(glUnmapBuffer(static_cast<GLenum>(target)));
}
void attachShader(uint32_t program, uint32_t shader)
{
  return glAttachShader(static_cast<GLuint>(program), static_cast<GLuint>(shader));
}
void bindAttribLocation(uint32_t program, uint32_t index, const char* name)
{
  return glBindAttribLocation(
    static_cast<GLuint>(program), static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLchar*>(name)));
}
void blendEquationSeparate(BlendEquationModeEXT modeRGB, BlendEquationModeEXT modeAlpha)
{
  return glBlendEquationSeparate(static_cast<GLenum>(modeRGB), static_cast<GLenum>(modeAlpha));
}
void compileShader(uint32_t shader)
{
  return glCompileShader(static_cast<GLuint>(shader));
}
uint32_t createProgram()
{
  return static_cast<uint32_t>(glCreateProgram());
}
uint32_t createShader(ShaderType type)
{
  return static_cast<uint32_t>(glCreateShader(static_cast<GLenum>(type)));
}
void deleteProgram(uint32_t program)
{
  return glDeleteProgram(static_cast<GLuint>(program));
}
void deleteShader(uint32_t shader)
{
  return glDeleteShader(static_cast<GLuint>(shader));
}
void detachShader(uint32_t program, uint32_t shader)
{
  return glDetachShader(static_cast<GLuint>(program), static_cast<GLuint>(shader));
}
void disableVertexAttribArray(uint32_t index)
{
  return glDisableVertexAttribArray(static_cast<GLuint>(index));
}
void drawBuffers(core::SizeType n, const DrawBufferMode* bufs)
{
  return glDrawBuffers(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<const GLenum*>(bufs)));
}
void enableVertexAttribArray(uint32_t index)
{
  return glEnableVertexAttribArray(static_cast<GLuint>(index));
}
void getActiveAttrib(uint32_t program,
                     uint32_t index,
                     core::SizeType bufSize,
                     core::SizeType* length,
                     int32_t* size,
                     AttributeType* type,
                     char* name)
{
  return glGetActiveAttrib(static_cast<GLuint>(program),
                           static_cast<GLuint>(index),
                           static_cast<GLsizei>(bufSize),
                           detail::constAway(reinterpret_cast<GLsizei*>(length)),
                           detail::constAway(reinterpret_cast<GLint*>(size)),
                           detail::constAway(reinterpret_cast<GLenum*>(type)),
                           detail::constAway(reinterpret_cast<GLchar*>(name)));
}
void getActiveUniform(uint32_t program,
                      uint32_t index,
                      core::SizeType bufSize,
                      core::SizeType* length,
                      int32_t* size,
                      UniformType* type,
                      char* name)
{
  return glGetActiveUniform(static_cast<GLuint>(program),
                            static_cast<GLuint>(index),
                            static_cast<GLsizei>(bufSize),
                            detail::constAway(reinterpret_cast<GLsizei*>(length)),
                            detail::constAway(reinterpret_cast<GLint*>(size)),
                            detail::constAway(reinterpret_cast<GLenum*>(type)),
                            detail::constAway(reinterpret_cast<GLchar*>(name)));
}
void getAttachedShaders(uint32_t program, core::SizeType maxCount, core::SizeType* count, uint32_t* shaders)
{
  return glGetAttachedShaders(static_cast<GLuint>(program),
                              static_cast<GLsizei>(maxCount),
                              detail::constAway(reinterpret_cast<GLsizei*>(count)),
                              detail::constAway(reinterpret_cast<GLuint*>(shaders)));
}
int32_t getAttribLocation(uint32_t program, const char* name)
{
  return static_cast<int32_t>(
    glGetAttribLocation(static_cast<GLuint>(program), detail::constAway(reinterpret_cast<const GLchar*>(name))));
}
void getProgramInfoLog(uint32_t program, core::SizeType bufSize, core::SizeType* length, char* infoLog)
{
  return glGetProgramInfoLog(static_cast<GLuint>(program),
                             static_cast<GLsizei>(bufSize),
                             detail::constAway(reinterpret_cast<GLsizei*>(length)),
                             detail::constAway(reinterpret_cast<GLchar*>(infoLog)));
}
void getProgram(uint32_t program, ProgramProperty pname, int32_t* params)
{
  return glGetProgramiv(
    static_cast<GLuint>(program), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint*>(params)));
}
void getShaderInfoLog(uint32_t shader, core::SizeType bufSize, core::SizeType* length, char* infoLog)
{
  return glGetShaderInfoLog(static_cast<GLuint>(shader),
                            static_cast<GLsizei>(bufSize),
                            detail::constAway(reinterpret_cast<GLsizei*>(length)),
                            detail::constAway(reinterpret_cast<GLchar*>(infoLog)));
}
void getShaderSource(uint32_t shader, core::SizeType bufSize, core::SizeType* length, char* source)
{
  return glGetShaderSource(static_cast<GLuint>(shader),
                           static_cast<GLsizei>(bufSize),
                           detail::constAway(reinterpret_cast<GLsizei*>(length)),
                           detail::constAway(reinterpret_cast<GLchar*>(source)));
}
void getShader(uint32_t shader, ShaderParameterName pname, int32_t* params)
{
  return glGetShaderiv(
    static_cast<GLuint>(shader), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint*>(params)));
}
int32_t getUniformLocation(uint32_t program, const char* name)
{
  return static_cast<int32_t>(
    glGetUniformLocation(static_cast<GLuint>(program), detail::constAway(reinterpret_cast<const GLchar*>(name))));
}
void getUniform(uint32_t program, int32_t location, float* params)
{
  return glGetUniformfv(
    static_cast<GLuint>(program), static_cast<GLint>(location), detail::constAway(reinterpret_cast<GLfloat*>(params)));
}
void getUniform(uint32_t program, int32_t location, int32_t* params)
{
  return glGetUniformiv(
    static_cast<GLuint>(program), static_cast<GLint>(location), detail::constAway(reinterpret_cast<GLint*>(params)));
}
void getVertexAttribPointer(uint32_t index, VertexAttribPointerProperty pname, void** pointer)
{
  return glGetVertexAttribPointerv(static_cast<GLuint>(index), static_cast<GLenum>(pname), detail::constAway(pointer));
}
void getVertexAttrib(uint32_t index, VertexAttribProperty pname, double* params)
{
  return glGetVertexAttribdv(
    static_cast<GLuint>(index), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLdouble*>(params)));
}
void getVertexAttrib(uint32_t index, VertexAttribProperty pname, float* params)
{
  return glGetVertexAttribfv(
    static_cast<GLuint>(index), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLfloat*>(params)));
}
void getVertexAttrib(uint32_t index, VertexAttribProperty pname, int32_t* params)
{
  return glGetVertexAttribiv(
    static_cast<GLuint>(index), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint*>(params)));
}
bool isProgram(uint32_t program)
{
  return static_cast<bool>(glIsProgram(static_cast<GLuint>(program)));
}
bool isShader(uint32_t shader)
{
  return static_cast<bool>(glIsShader(static_cast<GLuint>(shader)));
}
void linkProgram(uint32_t program)
{
  return glLinkProgram(static_cast<GLuint>(program));
}
void shaderSource(uint32_t shader, core::SizeType count, const char* const* string, const int32_t* length)
{
  return glShaderSource(static_cast<GLuint>(shader),
                        static_cast<GLsizei>(count),
                        detail::constAway(reinterpret_cast<const GLchar* const*>(string)),
                        detail::constAway(reinterpret_cast<const GLint*>(length)));
}
void stencilFuncSeparate(TriangleFace face, StencilFunction func, int32_t ref, uint32_t mask)
{
  return glStencilFuncSeparate(
    static_cast<GLenum>(face), static_cast<GLenum>(func), static_cast<GLint>(ref), static_cast<GLuint>(mask));
}
void stencilMaskSeparate(TriangleFace face, uint32_t mask)
{
  return glStencilMaskSeparate(static_cast<GLenum>(face), static_cast<GLuint>(mask));
}
void stencilOpSeparate(TriangleFace face, StencilOp sfail, StencilOp dpfail, StencilOp dppass)
{
  return glStencilOpSeparate(
    static_cast<GLenum>(face), static_cast<GLenum>(sfail), static_cast<GLenum>(dpfail), static_cast<GLenum>(dppass));
}
void uniform1(int32_t location, float v0)
{
  return glUniform1f(static_cast<GLint>(location), static_cast<GLfloat>(v0));
}
void uniform1(int32_t location, core::SizeType count, const float* value)
{
  return glUniform1fv(static_cast<GLint>(location),
                      static_cast<GLsizei>(count),
                      detail::constAway(reinterpret_cast<const GLfloat*>(value)));
}
void uniform1(int32_t location, int32_t v0)
{
  return glUniform1i(static_cast<GLint>(location), static_cast<GLint>(v0));
}
void uniform1(int32_t location, core::SizeType count, const int32_t* value)
{
  return glUniform1iv(static_cast<GLint>(location),
                      static_cast<GLsizei>(count),
                      detail::constAway(reinterpret_cast<const GLint*>(value)));
}
void uniform2(int32_t location, float v0, float v1)
{
  return glUniform2f(static_cast<GLint>(location), static_cast<GLfloat>(v0), static_cast<GLfloat>(v1));
}
void uniform2(int32_t location, core::SizeType count, const float* value)
{
  return glUniform2fv(static_cast<GLint>(location),
                      static_cast<GLsizei>(count),
                      detail::constAway(reinterpret_cast<const GLfloat*>(value)));
}
void uniform2(int32_t location, int32_t v0, int32_t v1)
{
  return glUniform2i(static_cast<GLint>(location), static_cast<GLint>(v0), static_cast<GLint>(v1));
}
void uniform2(int32_t location, core::SizeType count, const int32_t* value)
{
  return glUniform2iv(static_cast<GLint>(location),
                      static_cast<GLsizei>(count),
                      detail::constAway(reinterpret_cast<const GLint*>(value)));
}
void uniform3(int32_t location, float v0, float v1, float v2)
{
  return glUniform3f(
    static_cast<GLint>(location), static_cast<GLfloat>(v0), static_cast<GLfloat>(v1), static_cast<GLfloat>(v2));
}
void uniform3(int32_t location, core::SizeType count, const float* value)
{
  return glUniform3fv(static_cast<GLint>(location),
                      static_cast<GLsizei>(count),
                      detail::constAway(reinterpret_cast<const GLfloat*>(value)));
}
void uniform3(int32_t location, int32_t v0, int32_t v1, int32_t v2)
{
  return glUniform3i(
    static_cast<GLint>(location), static_cast<GLint>(v0), static_cast<GLint>(v1), static_cast<GLint>(v2));
}
void uniform3(int32_t location, core::SizeType count, const int32_t* value)
{
  return glUniform3iv(static_cast<GLint>(location),
                      static_cast<GLsizei>(count),
                      detail::constAway(reinterpret_cast<const GLint*>(value)));
}
void uniform4(int32_t location, float v0, float v1, float v2, float v3)
{
  return glUniform4f(static_cast<GLint>(location),
                     static_cast<GLfloat>(v0),
                     static_cast<GLfloat>(v1),
                     static_cast<GLfloat>(v2),
                     static_cast<GLfloat>(v3));
}
void uniform4(int32_t location, core::SizeType count, const float* value)
{
  return glUniform4fv(static_cast<GLint>(location),
                      static_cast<GLsizei>(count),
                      detail::constAway(reinterpret_cast<const GLfloat*>(value)));
}
void uniform4(int32_t location, int32_t v0, int32_t v1, int32_t v2, int32_t v3)
{
  return glUniform4i(static_cast<GLint>(location),
                     static_cast<GLint>(v0),
                     static_cast<GLint>(v1),
                     static_cast<GLint>(v2),
                     static_cast<GLint>(v3));
}
void uniform4(int32_t location, core::SizeType count, const int32_t* value)
{
  return glUniform4iv(static_cast<GLint>(location),
                      static_cast<GLsizei>(count),
                      detail::constAway(reinterpret_cast<const GLint*>(value)));
}
void uniformMatrix2(int32_t location, core::SizeType count, bool transpose, const float* value)
{
  return glUniformMatrix2fv(static_cast<GLint>(location),
                            static_cast<GLsizei>(count),
                            static_cast<GLboolean>(transpose),
                            detail::constAway(reinterpret_cast<const GLfloat*>(value)));
}
void uniformMatrix3(int32_t location, core::SizeType count, bool transpose, const float* value)
{
  return glUniformMatrix3fv(static_cast<GLint>(location),
                            static_cast<GLsizei>(count),
                            static_cast<GLboolean>(transpose),
                            detail::constAway(reinterpret_cast<const GLfloat*>(value)));
}
void uniformMatrix4(int32_t location, core::SizeType count, bool transpose, const float* value)
{
  return glUniformMatrix4fv(static_cast<GLint>(location),
                            static_cast<GLsizei>(count),
                            static_cast<GLboolean>(transpose),
                            detail::constAway(reinterpret_cast<const GLfloat*>(value)));
}
void useProgram(uint32_t program)
{
  return glUseProgram(static_cast<GLuint>(program));
}
void validateProgram(uint32_t program)
{
  return glValidateProgram(static_cast<GLuint>(program));
}
void vertexAttrib1(uint32_t index, double x)
{
  return glVertexAttrib1d(static_cast<GLuint>(index), static_cast<GLdouble>(x));
}
void vertexAttrib1(uint32_t index, const double* v)
{
  return glVertexAttrib1dv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLdouble*>(v)));
}
void vertexAttrib1(uint32_t index, float x)
{
  return glVertexAttrib1f(static_cast<GLuint>(index), static_cast<GLfloat>(x));
}
void vertexAttrib1(uint32_t index, const float* v)
{
  return glVertexAttrib1fv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLfloat*>(v)));
}
void vertexAttrib1(uint32_t index, int16_t x)
{
  return glVertexAttrib1s(static_cast<GLuint>(index), static_cast<GLshort>(x));
}
void vertexAttrib1(uint32_t index, const int16_t* v)
{
  return glVertexAttrib1sv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLshort*>(v)));
}
void vertexAttrib2(uint32_t index, double x, double y)
{
  return glVertexAttrib2d(static_cast<GLuint>(index), static_cast<GLdouble>(x), static_cast<GLdouble>(y));
}
void vertexAttrib2(uint32_t index, const double* v)
{
  return glVertexAttrib2dv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLdouble*>(v)));
}
void vertexAttrib2(uint32_t index, float x, float y)
{
  return glVertexAttrib2f(static_cast<GLuint>(index), static_cast<GLfloat>(x), static_cast<GLfloat>(y));
}
void vertexAttrib2(uint32_t index, const float* v)
{
  return glVertexAttrib2fv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLfloat*>(v)));
}
void vertexAttrib2(uint32_t index, int16_t x, int16_t y)
{
  return glVertexAttrib2s(static_cast<GLuint>(index), static_cast<GLshort>(x), static_cast<GLshort>(y));
}
void vertexAttrib2(uint32_t index, const int16_t* v)
{
  return glVertexAttrib2sv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLshort*>(v)));
}
void vertexAttrib3(uint32_t index, double x, double y, double z)
{
  return glVertexAttrib3d(
    static_cast<GLuint>(index), static_cast<GLdouble>(x), static_cast<GLdouble>(y), static_cast<GLdouble>(z));
}
void vertexAttrib3(uint32_t index, const double* v)
{
  return glVertexAttrib3dv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLdouble*>(v)));
}
void vertexAttrib3(uint32_t index, float x, float y, float z)
{
  return glVertexAttrib3f(
    static_cast<GLuint>(index), static_cast<GLfloat>(x), static_cast<GLfloat>(y), static_cast<GLfloat>(z));
}
void vertexAttrib3(uint32_t index, const float* v)
{
  return glVertexAttrib3fv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLfloat*>(v)));
}
void vertexAttrib3(uint32_t index, int16_t x, int16_t y, int16_t z)
{
  return glVertexAttrib3s(
    static_cast<GLuint>(index), static_cast<GLshort>(x), static_cast<GLshort>(y), static_cast<GLshort>(z));
}
void vertexAttrib3(uint32_t index, const int16_t* v)
{
  return glVertexAttrib3sv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLshort*>(v)));
}
void vertexAttrib4N(uint32_t index, const int8_t* v)
{
  return glVertexAttrib4Nbv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLbyte*>(v)));
}
void vertexAttrib4N(uint32_t index, const int32_t* v)
{
  return glVertexAttrib4Niv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLint*>(v)));
}
void vertexAttrib4N(uint32_t index, const int16_t* v)
{
  return glVertexAttrib4Nsv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLshort*>(v)));
}
void vertexAttrib4N(uint32_t index, uint8_t x, uint8_t y, uint8_t z, uint8_t w)
{
  return glVertexAttrib4Nub(static_cast<GLuint>(index),
                            static_cast<GLubyte>(x),
                            static_cast<GLubyte>(y),
                            static_cast<GLubyte>(z),
                            static_cast<GLubyte>(w));
}
void vertexAttrib4N(uint32_t index, const uint8_t* v)
{
  return glVertexAttrib4Nubv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLubyte*>(v)));
}
void vertexAttrib4N(uint32_t index, const uint32_t* v)
{
  return glVertexAttrib4Nuiv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLuint*>(v)));
}
void vertexAttrib4N(uint32_t index, const uint16_t* v)
{
  return glVertexAttrib4Nusv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLushort*>(v)));
}
void vertexAttrib4(uint32_t index, const int8_t* v)
{
  return glVertexAttrib4bv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLbyte*>(v)));
}
void vertexAttrib4(uint32_t index, double x, double y, double z, double w)
{
  return glVertexAttrib4d(static_cast<GLuint>(index),
                          static_cast<GLdouble>(x),
                          static_cast<GLdouble>(y),
                          static_cast<GLdouble>(z),
                          static_cast<GLdouble>(w));
}
void vertexAttrib4(uint32_t index, const double* v)
{
  return glVertexAttrib4dv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLdouble*>(v)));
}
void vertexAttrib4(uint32_t index, float x, float y, float z, float w)
{
  return glVertexAttrib4f(static_cast<GLuint>(index),
                          static_cast<GLfloat>(x),
                          static_cast<GLfloat>(y),
                          static_cast<GLfloat>(z),
                          static_cast<GLfloat>(w));
}
void vertexAttrib4(uint32_t index, const float* v)
{
  return glVertexAttrib4fv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLfloat*>(v)));
}
void vertexAttrib4(uint32_t index, const int32_t* v)
{
  return glVertexAttrib4iv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLint*>(v)));
}
void vertexAttrib4(uint32_t index, int16_t x, int16_t y, int16_t z, int16_t w)
{
  return glVertexAttrib4s(static_cast<GLuint>(index),
                          static_cast<GLshort>(x),
                          static_cast<GLshort>(y),
                          static_cast<GLshort>(z),
                          static_cast<GLshort>(w));
}
void vertexAttrib4(uint32_t index, const int16_t* v)
{
  return glVertexAttrib4sv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLshort*>(v)));
}
void vertexAttrib4(uint32_t index, const uint8_t* v)
{
  return glVertexAttrib4ubv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLubyte*>(v)));
}
void vertexAttrib4(uint32_t index, const uint32_t* v)
{
  return glVertexAttrib4uiv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLuint*>(v)));
}
void vertexAttrib4(uint32_t index, const uint16_t* v)
{
  return glVertexAttrib4usv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLushort*>(v)));
}
void vertexAttribPointer(uint32_t index,
                         int32_t size,
                         VertexAttribPointerType type,
                         bool normalized,
                         core::SizeType stride,
                         const void* pointer)
{
  return glVertexAttribPointer(static_cast<GLuint>(index),
                               static_cast<GLint>(size),
                               static_cast<GLenum>(type),
                               static_cast<GLboolean>(normalized),
                               static_cast<GLsizei>(stride),
                               detail::constAway(pointer));
}
void uniformMatrix2x3(int32_t location, core::SizeType count, bool transpose, const float* value)
{
  return glUniformMatrix2x3fv(static_cast<GLint>(location),
                              static_cast<GLsizei>(count),
                              static_cast<GLboolean>(transpose),
                              detail::constAway(reinterpret_cast<const GLfloat*>(value)));
}
void uniformMatrix2x4(int32_t location, core::SizeType count, bool transpose, const float* value)
{
  return glUniformMatrix2x4fv(static_cast<GLint>(location),
                              static_cast<GLsizei>(count),
                              static_cast<GLboolean>(transpose),
                              detail::constAway(reinterpret_cast<const GLfloat*>(value)));
}
void uniformMatrix3x2(int32_t location, core::SizeType count, bool transpose, const float* value)
{
  return glUniformMatrix3x2fv(static_cast<GLint>(location),
                              static_cast<GLsizei>(count),
                              static_cast<GLboolean>(transpose),
                              detail::constAway(reinterpret_cast<const GLfloat*>(value)));
}
void uniformMatrix3x4(int32_t location, core::SizeType count, bool transpose, const float* value)
{
  return glUniformMatrix3x4fv(static_cast<GLint>(location),
                              static_cast<GLsizei>(count),
                              static_cast<GLboolean>(transpose),
                              detail::constAway(reinterpret_cast<const GLfloat*>(value)));
}
void uniformMatrix4x2(int32_t location, core::SizeType count, bool transpose, const float* value)
{
  return glUniformMatrix4x2fv(static_cast<GLint>(location),
                              static_cast<GLsizei>(count),
                              static_cast<GLboolean>(transpose),
                              detail::constAway(reinterpret_cast<const GLfloat*>(value)));
}
void uniformMatrix4x3(int32_t location, core::SizeType count, bool transpose, const float* value)
{
  return glUniformMatrix4x3fv(static_cast<GLint>(location),
                              static_cast<GLsizei>(count),
                              static_cast<GLboolean>(transpose),
                              detail::constAway(reinterpret_cast<const GLfloat*>(value)));
}
void beginConditionalRender(uint32_t id, ConditionalRenderMode mode)
{
  return glBeginConditionalRender(static_cast<GLuint>(id), static_cast<GLenum>(mode));
}
void beginTransformFeedback(PrimitiveType primitiveMode)
{
  return glBeginTransformFeedback(static_cast<GLenum>(primitiveMode));
}
void bindBufferBase(BufferTarget target, uint32_t index, uint32_t buffer)
{
  return glBindBufferBase(static_cast<GLenum>(target), static_cast<GLuint>(index), static_cast<GLuint>(buffer));
}
void bindBufferRange(BufferTarget target, uint32_t index, uint32_t buffer, std::intptr_t offset, std::size_t size)
{
  return glBindBufferRange(static_cast<GLenum>(target),
                           static_cast<GLuint>(index),
                           static_cast<GLuint>(buffer),
                           static_cast<GLintptr>(offset),
                           static_cast<GLsizeiptr>(size));
}
void bindFragDataLocation(uint32_t program, uint32_t color, const char* name)
{
  return glBindFragDataLocation(
    static_cast<GLuint>(program), static_cast<GLuint>(color), detail::constAway(reinterpret_cast<const GLchar*>(name)));
}
void bindFramebuffer(FramebufferTarget target, uint32_t framebuffer)
{
  return glBindFramebuffer(static_cast<GLenum>(target), static_cast<GLuint>(framebuffer));
}
void bindRenderbuffer(RenderbufferTarget target, uint32_t renderbuffer)
{
  return glBindRenderbuffer(static_cast<GLenum>(target), static_cast<GLuint>(renderbuffer));
}
void bindVertexArray(uint32_t array)
{
  return glBindVertexArray(static_cast<GLuint>(array));
}
void blitFramebuffer(int32_t srcX0,
                     int32_t srcY0,
                     int32_t srcX1,
                     int32_t srcY1,
                     int32_t dstX0,
                     int32_t dstY0,
                     int32_t dstX1,
                     int32_t dstY1,
                     core::Bitfield<ClearBufferMask> mask,
                     BlitFramebufferFilter filter)
{
  return glBlitFramebuffer(static_cast<GLint>(srcX0),
                           static_cast<GLint>(srcY0),
                           static_cast<GLint>(srcX1),
                           static_cast<GLint>(srcY1),
                           static_cast<GLint>(dstX0),
                           static_cast<GLint>(dstY0),
                           static_cast<GLint>(dstX1),
                           static_cast<GLint>(dstY1),
                           mask.value(),
                           static_cast<GLenum>(filter));
}
FramebufferStatus checkFramebufferStatus(FramebufferTarget target)
{
  return static_cast<FramebufferStatus>(glCheckFramebufferStatus(static_cast<GLenum>(target)));
}
void clampColor(ClampColorTarget target, ClampColorMode clamp)
{
  return glClampColor(static_cast<GLenum>(target), static_cast<GLenum>(clamp));
}
void clearBufferf(Buffer buffer, int32_t drawbuffer, float depth, int32_t stencil)
{
  return glClearBufferfi(static_cast<GLenum>(buffer),
                         static_cast<GLint>(drawbuffer),
                         static_cast<GLfloat>(depth),
                         static_cast<GLint>(stencil));
}
void clearBuffer(Buffer buffer, int32_t drawbuffer, const float* value)
{
  return glClearBufferfv(static_cast<GLenum>(buffer),
                         static_cast<GLint>(drawbuffer),
                         detail::constAway(reinterpret_cast<const GLfloat*>(value)));
}
void clearBuffer(Buffer buffer, int32_t drawbuffer, const int32_t* value)
{
  return glClearBufferiv(static_cast<GLenum>(buffer),
                         static_cast<GLint>(drawbuffer),
                         detail::constAway(reinterpret_cast<const GLint*>(value)));
}
void clearBuffer(Buffer buffer, int32_t drawbuffer, const uint32_t* value)
{
  return glClearBufferuiv(static_cast<GLenum>(buffer),
                          static_cast<GLint>(drawbuffer),
                          detail::constAway(reinterpret_cast<const GLuint*>(value)));
}
void colorMask(uint32_t index, bool r, bool g, bool b, bool a)
{
  return glColorMaski(static_cast<GLuint>(index),
                      static_cast<GLboolean>(r),
                      static_cast<GLboolean>(g),
                      static_cast<GLboolean>(b),
                      static_cast<GLboolean>(a));
}
void deleteFramebuffers(core::SizeType n, const uint32_t* framebuffers)
{
  return glDeleteFramebuffers(static_cast<GLsizei>(n),
                              detail::constAway(reinterpret_cast<const GLuint*>(framebuffers)));
}
void deleteRenderbuffers(core::SizeType n, const uint32_t* renderbuffers)
{
  return glDeleteRenderbuffers(static_cast<GLsizei>(n),
                               detail::constAway(reinterpret_cast<const GLuint*>(renderbuffers)));
}
void deleteVertexArrays(core::SizeType n, const uint32_t* arrays)
{
  return glDeleteVertexArrays(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<const GLuint*>(arrays)));
}
void disable(EnableCap target, uint32_t index)
{
  return glDisablei(static_cast<GLenum>(target), static_cast<GLuint>(index));
}
void enable(EnableCap target, uint32_t index)
{
  return glEnablei(static_cast<GLenum>(target), static_cast<GLuint>(index));
}
void endConditionalRender()
{
  return glEndConditionalRender();
}
void endTransformFeedback()
{
  return glEndTransformFeedback();
}
void flushMappedBufferRange(BufferTarget target, std::intptr_t offset, std::size_t length)
{
  return glFlushMappedBufferRange(
    static_cast<GLenum>(target), static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(length));
}
void framebufferRenderbuffer(FramebufferTarget target,
                             FramebufferAttachment attachment,
                             RenderbufferTarget renderbuffertarget,
                             uint32_t renderbuffer)
{
  return glFramebufferRenderbuffer(static_cast<GLenum>(target),
                                   static_cast<GLenum>(attachment),
                                   static_cast<GLenum>(renderbuffertarget),
                                   static_cast<GLuint>(renderbuffer));
}
void framebufferTexture1D(
  FramebufferTarget target, FramebufferAttachment attachment, TextureTarget textarget, uint32_t texture, int32_t level)
{
  return glFramebufferTexture1D(static_cast<GLenum>(target),
                                static_cast<GLenum>(attachment),
                                static_cast<GLenum>(textarget),
                                static_cast<GLuint>(texture),
                                static_cast<GLint>(level));
}
void framebufferTexture2D(
  FramebufferTarget target, FramebufferAttachment attachment, TextureTarget textarget, uint32_t texture, int32_t level)
{
  return glFramebufferTexture2D(static_cast<GLenum>(target),
                                static_cast<GLenum>(attachment),
                                static_cast<GLenum>(textarget),
                                static_cast<GLuint>(texture),
                                static_cast<GLint>(level));
}
void framebufferTexture3D(FramebufferTarget target,
                          FramebufferAttachment attachment,
                          TextureTarget textarget,
                          uint32_t texture,
                          int32_t level,
                          int32_t zoffset)
{
  return glFramebufferTexture3D(static_cast<GLenum>(target),
                                static_cast<GLenum>(attachment),
                                static_cast<GLenum>(textarget),
                                static_cast<GLuint>(texture),
                                static_cast<GLint>(level),
                                static_cast<GLint>(zoffset));
}
void framebufferTextureLayer(
  FramebufferTarget target, FramebufferAttachment attachment, uint32_t texture, int32_t level, int32_t layer)
{
  return glFramebufferTextureLayer(static_cast<GLenum>(target),
                                   static_cast<GLenum>(attachment),
                                   static_cast<GLuint>(texture),
                                   static_cast<GLint>(level),
                                   static_cast<GLint>(layer));
}
void genFramebuffers(core::SizeType n, uint32_t* framebuffers)
{
  return glGenFramebuffers(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<GLuint*>(framebuffers)));
}
void genRenderbuffers(core::SizeType n, uint32_t* renderbuffers)
{
  return glGenRenderbuffers(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<GLuint*>(renderbuffers)));
}
void genVertexArrays(core::SizeType n, uint32_t* arrays)
{
  return glGenVertexArrays(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<GLuint*>(arrays)));
}
void generateMipmap(TextureTarget target)
{
  return glGenerateMipmap(static_cast<GLenum>(target));
}
void getBooleanI(BufferTarget target, uint32_t index, bool* data)
{
  return glGetBooleani_v(
    static_cast<GLenum>(target), static_cast<GLuint>(index), detail::constAway(reinterpret_cast<GLboolean*>(data)));
}
int32_t getFragDataLocation(uint32_t program, const char* name)
{
  return static_cast<int32_t>(
    glGetFragDataLocation(static_cast<GLuint>(program), detail::constAway(reinterpret_cast<const GLchar*>(name))));
}
void getFramebufferAttachmentParameter(FramebufferTarget target,
                                       FramebufferAttachment attachment,
                                       FramebufferAttachmentParameterName pname,
                                       int32_t* params)
{
  return glGetFramebufferAttachmentParameteriv(static_cast<GLenum>(target),
                                               static_cast<GLenum>(attachment),
                                               static_cast<GLenum>(pname),
                                               detail::constAway(reinterpret_cast<GLint*>(params)));
}
void getIntegerI(GetPName target, uint32_t index, int32_t* data)
{
  return glGetIntegeri_v(
    static_cast<GLenum>(target), static_cast<GLuint>(index), detail::constAway(reinterpret_cast<GLint*>(data)));
}
void getRenderbufferParameter(RenderbufferTarget target, RenderbufferParameterName pname, int32_t* params)
{
  return glGetRenderbufferParameteriv(
    static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint*>(params)));
}
const uint8_t* getString(StringName name, uint32_t index)
{
  return static_cast<const uint8_t*>(glGetStringi(static_cast<GLenum>(name), static_cast<GLuint>(index)));
}
void getTexParameterI(TextureTarget target, GetTextureParameter pname, int32_t* params)
{
  return glGetTexParameterIiv(
    static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint*>(params)));
}
void getTexParameterI(TextureTarget target, GetTextureParameter pname, uint32_t* params)
{
  return glGetTexParameterIuiv(
    static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLuint*>(params)));
}
void getTransformFeedbackVarying(uint32_t program,
                                 uint32_t index,
                                 core::SizeType bufSize,
                                 core::SizeType* length,
                                 core::SizeType* size,
                                 AttributeType* type,
                                 char* name)
{
  return glGetTransformFeedbackVarying(static_cast<GLuint>(program),
                                       static_cast<GLuint>(index),
                                       static_cast<GLsizei>(bufSize),
                                       detail::constAway(reinterpret_cast<GLsizei*>(length)),
                                       detail::constAway(reinterpret_cast<GLsizei*>(size)),
                                       detail::constAway(reinterpret_cast<GLenum*>(type)),
                                       detail::constAway(reinterpret_cast<GLchar*>(name)));
}
void getUniform(uint32_t program, int32_t location, uint32_t* params)
{
  return glGetUniformuiv(
    static_cast<GLuint>(program), static_cast<GLint>(location), detail::constAway(reinterpret_cast<GLuint*>(params)));
}
void getVertexAttribI(uint32_t index, VertexAttribEnum pname, int32_t* params)
{
  return glGetVertexAttribIiv(
    static_cast<GLuint>(index), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint*>(params)));
}
void getVertexAttribI(uint32_t index, VertexAttribEnum pname, uint32_t* params)
{
  return glGetVertexAttribIuiv(
    static_cast<GLuint>(index), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLuint*>(params)));
}
bool isEnabled(EnableCap target, uint32_t index)
{
  return static_cast<bool>(glIsEnabledi(static_cast<GLenum>(target), static_cast<GLuint>(index)));
}
bool isFramebuffer(uint32_t framebuffer)
{
  return static_cast<bool>(glIsFramebuffer(static_cast<GLuint>(framebuffer)));
}
bool isRenderbuffer(uint32_t renderbuffer)
{
  return static_cast<bool>(glIsRenderbuffer(static_cast<GLuint>(renderbuffer)));
}
bool isVertexArray(uint32_t array)
{
  return static_cast<bool>(glIsVertexArray(static_cast<GLuint>(array)));
}
void* mapBufferRange(BufferTarget target,
                     std::intptr_t offset,
                     std::size_t length,
                     core::Bitfield<MapBufferAccessMask> access)
{
  return glMapBufferRange(
    static_cast<GLenum>(target), static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(length), access.value());
}
void renderbufferStorage(RenderbufferTarget target,
                         InternalFormat internalformat,
                         core::SizeType width,
                         core::SizeType height)
{
  return glRenderbufferStorage(static_cast<GLenum>(target),
                               static_cast<GLenum>(internalformat),
                               static_cast<GLsizei>(width),
                               static_cast<GLsizei>(height));
}
void renderbufferStorageMultisample(RenderbufferTarget target,
                                    core::SizeType samples,
                                    InternalFormat internalformat,
                                    core::SizeType width,
                                    core::SizeType height)
{
  return glRenderbufferStorageMultisample(static_cast<GLenum>(target),
                                          static_cast<GLsizei>(samples),
                                          static_cast<GLenum>(internalformat),
                                          static_cast<GLsizei>(width),
                                          static_cast<GLsizei>(height));
}
void texParameterI(TextureTarget target, TextureParameterName pname, const int32_t* params)
{
  return glTexParameterIiv(
    static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<const GLint*>(params)));
}
void texParameterI(TextureTarget target, TextureParameterName pname, const uint32_t* params)
{
  return glTexParameterIuiv(static_cast<GLenum>(target),
                            static_cast<GLenum>(pname),
                            detail::constAway(reinterpret_cast<const GLuint*>(params)));
}
void transformFeedbackVarying(uint32_t program,
                              core::SizeType count,
                              const char* const* varyings,
                              TransformFeedbackBufferMode bufferMode)
{
  return glTransformFeedbackVaryings(static_cast<GLuint>(program),
                                     static_cast<GLsizei>(count),
                                     detail::constAway(reinterpret_cast<const GLchar* const*>(varyings)),
                                     static_cast<GLenum>(bufferMode));
}
void uniform1(int32_t location, uint32_t v0)
{
  return glUniform1ui(static_cast<GLint>(location), static_cast<GLuint>(v0));
}
void uniform1(int32_t location, core::SizeType count, const uint32_t* value)
{
  return glUniform1uiv(static_cast<GLint>(location),
                       static_cast<GLsizei>(count),
                       detail::constAway(reinterpret_cast<const GLuint*>(value)));
}
void uniform2(int32_t location, uint32_t v0, uint32_t v1)
{
  return glUniform2ui(static_cast<GLint>(location), static_cast<GLuint>(v0), static_cast<GLuint>(v1));
}
void uniform2(int32_t location, core::SizeType count, const uint32_t* value)
{
  return glUniform2uiv(static_cast<GLint>(location),
                       static_cast<GLsizei>(count),
                       detail::constAway(reinterpret_cast<const GLuint*>(value)));
}
void uniform3(int32_t location, uint32_t v0, uint32_t v1, uint32_t v2)
{
  return glUniform3ui(
    static_cast<GLint>(location), static_cast<GLuint>(v0), static_cast<GLuint>(v1), static_cast<GLuint>(v2));
}
void uniform3(int32_t location, core::SizeType count, const uint32_t* value)
{
  return glUniform3uiv(static_cast<GLint>(location),
                       static_cast<GLsizei>(count),
                       detail::constAway(reinterpret_cast<const GLuint*>(value)));
}
void uniform4(int32_t location, uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3)
{
  return glUniform4ui(static_cast<GLint>(location),
                      static_cast<GLuint>(v0),
                      static_cast<GLuint>(v1),
                      static_cast<GLuint>(v2),
                      static_cast<GLuint>(v3));
}
void uniform4(int32_t location, core::SizeType count, const uint32_t* value)
{
  return glUniform4uiv(static_cast<GLint>(location),
                       static_cast<GLsizei>(count),
                       detail::constAway(reinterpret_cast<const GLuint*>(value)));
}
void vertexAttribI1(uint32_t index, int32_t x)
{
  return glVertexAttribI1i(static_cast<GLuint>(index), static_cast<GLint>(x));
}
void vertexAttribI1(uint32_t index, const int32_t* v)
{
  return glVertexAttribI1iv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLint*>(v)));
}
void vertexAttribI1(uint32_t index, uint32_t x)
{
  return glVertexAttribI1ui(static_cast<GLuint>(index), static_cast<GLuint>(x));
}
void vertexAttribI1(uint32_t index, const uint32_t* v)
{
  return glVertexAttribI1uiv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLuint*>(v)));
}
void vertexAttribI2(uint32_t index, int32_t x, int32_t y)
{
  return glVertexAttribI2i(static_cast<GLuint>(index), static_cast<GLint>(x), static_cast<GLint>(y));
}
void vertexAttribI2(uint32_t index, const int32_t* v)
{
  return glVertexAttribI2iv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLint*>(v)));
}
void vertexAttribI2(uint32_t index, uint32_t x, uint32_t y)
{
  return glVertexAttribI2ui(static_cast<GLuint>(index), static_cast<GLuint>(x), static_cast<GLuint>(y));
}
void vertexAttribI2(uint32_t index, const uint32_t* v)
{
  return glVertexAttribI2uiv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLuint*>(v)));
}
void vertexAttribI3(uint32_t index, int32_t x, int32_t y, int32_t z)
{
  return glVertexAttribI3i(
    static_cast<GLuint>(index), static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLint>(z));
}
void vertexAttribI3(uint32_t index, const int32_t* v)
{
  return glVertexAttribI3iv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLint*>(v)));
}
void vertexAttribI3(uint32_t index, uint32_t x, uint32_t y, uint32_t z)
{
  return glVertexAttribI3ui(
    static_cast<GLuint>(index), static_cast<GLuint>(x), static_cast<GLuint>(y), static_cast<GLuint>(z));
}
void vertexAttribI3(uint32_t index, const uint32_t* v)
{
  return glVertexAttribI3uiv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLuint*>(v)));
}
void vertexAttribI4(uint32_t index, const int8_t* v)
{
  return glVertexAttribI4bv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLbyte*>(v)));
}
void vertexAttribI4(uint32_t index, int32_t x, int32_t y, int32_t z, int32_t w)
{
  return glVertexAttribI4i(static_cast<GLuint>(index),
                           static_cast<GLint>(x),
                           static_cast<GLint>(y),
                           static_cast<GLint>(z),
                           static_cast<GLint>(w));
}
void vertexAttribI4(uint32_t index, const int32_t* v)
{
  return glVertexAttribI4iv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLint*>(v)));
}
void vertexAttribI4(uint32_t index, const int16_t* v)
{
  return glVertexAttribI4sv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLshort*>(v)));
}
void vertexAttribI4(uint32_t index, const uint8_t* v)
{
  return glVertexAttribI4ubv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLubyte*>(v)));
}
void vertexAttribI4(uint32_t index, uint32_t x, uint32_t y, uint32_t z, uint32_t w)
{
  return glVertexAttribI4ui(static_cast<GLuint>(index),
                            static_cast<GLuint>(x),
                            static_cast<GLuint>(y),
                            static_cast<GLuint>(z),
                            static_cast<GLuint>(w));
}
void vertexAttribI4(uint32_t index, const uint32_t* v)
{
  return glVertexAttribI4uiv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLuint*>(v)));
}
void vertexAttribI4(uint32_t index, const uint16_t* v)
{
  return glVertexAttribI4usv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLushort*>(v)));
}
void vertexAttribIPointer(
  uint32_t index, int32_t size, VertexAttribIType type, core::SizeType stride, const void* pointer)
{
  return glVertexAttribIPointer(static_cast<GLuint>(index),
                                static_cast<GLint>(size),
                                static_cast<GLenum>(type),
                                static_cast<GLsizei>(stride),
                                detail::constAway(pointer));
}
void copyBufferSubData(CopyBufferSubDataTarget readTarget,
                       CopyBufferSubDataTarget writeTarget,
                       std::intptr_t readOffset,
                       std::intptr_t writeOffset,
                       std::size_t size)
{
  return glCopyBufferSubData(static_cast<GLenum>(readTarget),
                             static_cast<GLenum>(writeTarget),
                             static_cast<GLintptr>(readOffset),
                             static_cast<GLintptr>(writeOffset),
                             static_cast<GLsizeiptr>(size));
}
void drawArraysInstanced(PrimitiveType mode, int32_t first, core::SizeType count, core::SizeType instancecount)
{
  return glDrawArraysInstanced(static_cast<GLenum>(mode),
                               static_cast<GLint>(first),
                               static_cast<GLsizei>(count),
                               static_cast<GLsizei>(instancecount));
}
void drawElementsInstanced(
  PrimitiveType mode, core::SizeType count, DrawElementsType type, const void* indices, core::SizeType instancecount)
{
  return glDrawElementsInstanced(static_cast<GLenum>(mode),
                                 static_cast<GLsizei>(count),
                                 static_cast<GLenum>(type),
                                 detail::constAway(indices),
                                 static_cast<GLsizei>(instancecount));
}
void getActiveUniformBlockName(
  uint32_t program, uint32_t uniformBlockIndex, core::SizeType bufSize, core::SizeType* length, char* uniformBlockName)
{
  return glGetActiveUniformBlockName(static_cast<GLuint>(program),
                                     static_cast<GLuint>(uniformBlockIndex),
                                     static_cast<GLsizei>(bufSize),
                                     detail::constAway(reinterpret_cast<GLsizei*>(length)),
                                     detail::constAway(reinterpret_cast<GLchar*>(uniformBlockName)));
}
void getActiveUniformBlock(uint32_t program, uint32_t uniformBlockIndex, UniformBlockPName pname, int32_t* params)
{
  return glGetActiveUniformBlockiv(static_cast<GLuint>(program),
                                   static_cast<GLuint>(uniformBlockIndex),
                                   static_cast<GLenum>(pname),
                                   detail::constAway(reinterpret_cast<GLint*>(params)));
}
void getActiveUniformName(
  uint32_t program, uint32_t uniformIndex, core::SizeType bufSize, core::SizeType* length, char* uniformName)
{
  return glGetActiveUniformName(static_cast<GLuint>(program),
                                static_cast<GLuint>(uniformIndex),
                                static_cast<GLsizei>(bufSize),
                                detail::constAway(reinterpret_cast<GLsizei*>(length)),
                                detail::constAway(reinterpret_cast<GLchar*>(uniformName)));
}
void getActiveUniforms(
  uint32_t program, core::SizeType uniformCount, const uint32_t* uniformIndices, UniformPName pname, int32_t* params)
{
  return glGetActiveUniformsiv(static_cast<GLuint>(program),
                               static_cast<GLsizei>(uniformCount),
                               detail::constAway(reinterpret_cast<const GLuint*>(uniformIndices)),
                               static_cast<GLenum>(pname),
                               detail::constAway(reinterpret_cast<GLint*>(params)));
}
uint32_t getUniformBlockIndex(uint32_t program, const char* uniformBlockName)
{
  return static_cast<uint32_t>(glGetUniformBlockIndex(
    static_cast<GLuint>(program), detail::constAway(reinterpret_cast<const GLchar*>(uniformBlockName))));
}
void getUniformIndice(uint32_t program,
                      core::SizeType uniformCount,
                      const char* const* uniformNames,
                      uint32_t* uniformIndices)
{
  return glGetUniformIndices(static_cast<GLuint>(program),
                             static_cast<GLsizei>(uniformCount),
                             detail::constAway(reinterpret_cast<const GLchar* const*>(uniformNames)),
                             detail::constAway(reinterpret_cast<GLuint*>(uniformIndices)));
}
void primitiveRestartIndex(uint32_t index)
{
  return glPrimitiveRestartIndex(static_cast<GLuint>(index));
}
void texBuffer(TextureTarget target, SizedInternalFormat internalformat, uint32_t buffer)
{
  return glTexBuffer(static_cast<GLenum>(target), static_cast<GLenum>(internalformat), static_cast<GLuint>(buffer));
}
void uniformBlockBinding(uint32_t program, uint32_t uniformBlockIndex, uint32_t uniformBlockBinding)
{
  return glUniformBlockBinding(
    static_cast<GLuint>(program), static_cast<GLuint>(uniformBlockIndex), static_cast<GLuint>(uniformBlockBinding));
}
void drawElementsBaseVertex(
  PrimitiveType mode, core::SizeType count, DrawElementsType type, const void* indices, int32_t basevertex)
{
  return glDrawElementsBaseVertex(static_cast<GLenum>(mode),
                                  static_cast<GLsizei>(count),
                                  static_cast<GLenum>(type),
                                  detail::constAway(indices),
                                  static_cast<GLint>(basevertex));
}
void drawElementsInstancedBaseVertex(PrimitiveType mode,
                                     core::SizeType count,
                                     DrawElementsType type,
                                     const void* indices,
                                     core::SizeType instancecount,
                                     int32_t basevertex)
{
  return glDrawElementsInstancedBaseVertex(static_cast<GLenum>(mode),
                                           static_cast<GLsizei>(count),
                                           static_cast<GLenum>(type),
                                           detail::constAway(indices),
                                           static_cast<GLsizei>(instancecount),
                                           static_cast<GLint>(basevertex));
}
void drawRangeElementsBaseVertex(PrimitiveType mode,
                                 uint32_t start,
                                 uint32_t end,
                                 core::SizeType count,
                                 DrawElementsType type,
                                 const void* indices,
                                 int32_t basevertex)
{
  return glDrawRangeElementsBaseVertex(static_cast<GLenum>(mode),
                                       static_cast<GLuint>(start),
                                       static_cast<GLuint>(end),
                                       static_cast<GLsizei>(count),
                                       static_cast<GLenum>(type),
                                       detail::constAway(indices),
                                       static_cast<GLint>(basevertex));
}
void framebufferTexture(FramebufferTarget target, FramebufferAttachment attachment, uint32_t texture, int32_t level)
{
  return glFramebufferTexture(static_cast<GLenum>(target),
                              static_cast<GLenum>(attachment),
                              static_cast<GLuint>(texture),
                              static_cast<GLint>(level));
}
void getBufferParameter(BufferTarget target, BufferPName pname, int64_t* params)
{
  return glGetBufferParameteri64v(
    static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint64*>(params)));
}
void getInteger64I(GetPName target, uint32_t index, int64_t* data)
{
  return glGetInteger64i_v(
    static_cast<GLenum>(target), static_cast<GLuint>(index), detail::constAway(reinterpret_cast<GLint64*>(data)));
}
void getMultisample(GetMultisamplePNameNV pname, uint32_t index, float* val)
{
  return glGetMultisamplefv(
    static_cast<GLenum>(pname), static_cast<GLuint>(index), detail::constAway(reinterpret_cast<GLfloat*>(val)));
}
void multiDrawElementsBaseVertex(PrimitiveType mode,
                                 const core::SizeType* count,
                                 DrawElementsType type,
                                 const void* const* indices,
                                 core::SizeType drawcount,
                                 const int32_t* basevertex)
{
  return glMultiDrawElementsBaseVertex(static_cast<GLenum>(mode),
                                       detail::constAway(reinterpret_cast<const GLsizei*>(count)),
                                       static_cast<GLenum>(type),
                                       detail::constAway(indices),
                                       static_cast<GLsizei>(drawcount),
                                       detail::constAway(reinterpret_cast<const GLint*>(basevertex)));
}
void provokingVertex(VertexProvokingMode mode)
{
  return glProvokingVertex(static_cast<GLenum>(mode));
}
void sampleMask(uint32_t maskNumber, uint32_t mask)
{
  return glSampleMaski(static_cast<GLuint>(maskNumber), static_cast<GLbitfield>(mask));
}
void texImage2DMultisample(TextureTarget target,
                           core::SizeType samples,
                           InternalFormat internalformat,
                           core::SizeType width,
                           core::SizeType height,
                           bool fixedsamplelocations)
{
  return glTexImage2DMultisample(static_cast<GLenum>(target),
                                 static_cast<GLsizei>(samples),
                                 static_cast<GLenum>(internalformat),
                                 static_cast<GLsizei>(width),
                                 static_cast<GLsizei>(height),
                                 static_cast<GLboolean>(fixedsamplelocations));
}
void texImage3DMultisample(TextureTarget target,
                           core::SizeType samples,
                           InternalFormat internalformat,
                           core::SizeType width,
                           core::SizeType height,
                           core::SizeType depth,
                           bool fixedsamplelocations)
{
  return glTexImage3DMultisample(static_cast<GLenum>(target),
                                 static_cast<GLsizei>(samples),
                                 static_cast<GLenum>(internalformat),
                                 static_cast<GLsizei>(width),
                                 static_cast<GLsizei>(height),
                                 static_cast<GLsizei>(depth),
                                 static_cast<GLboolean>(fixedsamplelocations));
}
void bindFragDataLocationIndexed(uint32_t program, uint32_t colorNumber, uint32_t index, const char* name)
{
  return glBindFragDataLocationIndexed(static_cast<GLuint>(program),
                                       static_cast<GLuint>(colorNumber),
                                       static_cast<GLuint>(index),
                                       detail::constAway(reinterpret_cast<const GLchar*>(name)));
}
void bindSampler(uint32_t unit, uint32_t sampler)
{
  return glBindSampler(static_cast<GLuint>(unit), static_cast<GLuint>(sampler));
}
void deleteSampler(core::SizeType count, const uint32_t* samplers)
{
  return glDeleteSamplers(static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLuint*>(samplers)));
}
void genSampler(core::SizeType count, uint32_t* samplers)
{
  return glGenSamplers(static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<GLuint*>(samplers)));
}
int32_t getFragDataIndex(uint32_t program, const char* name)
{
  return static_cast<int32_t>(
    glGetFragDataIndex(static_cast<GLuint>(program), detail::constAway(reinterpret_cast<const GLchar*>(name))));
}
void getQueryObject(uint32_t id, QueryObjectParameterName pname, int64_t* params)
{
  return glGetQueryObjecti64v(
    static_cast<GLuint>(id), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint64*>(params)));
}
void getQueryObject(uint32_t id, QueryObjectParameterName pname, uint64_t* params)
{
  return glGetQueryObjectui64v(
    static_cast<GLuint>(id), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLuint64*>(params)));
}
void getSamplerParameterI(uint32_t sampler, SamplerParameterI pname, int32_t* params)
{
  return glGetSamplerParameterIiv(
    static_cast<GLuint>(sampler), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint*>(params)));
}
void getSamplerParameterI(uint32_t sampler, SamplerParameterI pname, uint32_t* params)
{
  return glGetSamplerParameterIuiv(
    static_cast<GLuint>(sampler), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLuint*>(params)));
}
void getSamplerParameter(uint32_t sampler, SamplerParameterF pname, float* params)
{
  return glGetSamplerParameterfv(
    static_cast<GLuint>(sampler), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLfloat*>(params)));
}
void getSamplerParameter(uint32_t sampler, SamplerParameterI pname, int32_t* params)
{
  return glGetSamplerParameteriv(
    static_cast<GLuint>(sampler), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint*>(params)));
}
bool isSampler(uint32_t sampler)
{
  return static_cast<bool>(glIsSampler(static_cast<GLuint>(sampler)));
}
void queryCounter(uint32_t id, QueryCounterTarget target)
{
  return glQueryCounter(static_cast<GLuint>(id), static_cast<GLenum>(target));
}
void samplerParameterI(uint32_t sampler, SamplerParameterI pname, const int32_t* param)
{
  return glSamplerParameterIiv(
    static_cast<GLuint>(sampler), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<const GLint*>(param)));
}
void samplerParameterI(uint32_t sampler, SamplerParameterI pname, const uint32_t* param)
{
  return glSamplerParameterIuiv(static_cast<GLuint>(sampler),
                                static_cast<GLenum>(pname),
                                detail::constAway(reinterpret_cast<const GLuint*>(param)));
}
void samplerParameter(uint32_t sampler, SamplerParameterF pname, float param)
{
  return glSamplerParameterf(static_cast<GLuint>(sampler), static_cast<GLenum>(pname), static_cast<GLfloat>(param));
}
void samplerParameter(uint32_t sampler, SamplerParameterF pname, const float* param)
{
  return glSamplerParameterfv(static_cast<GLuint>(sampler),
                              static_cast<GLenum>(pname),
                              detail::constAway(reinterpret_cast<const GLfloat*>(param)));
}
void samplerParameter(uint32_t sampler, SamplerParameterI pname, int32_t param)
{
  return glSamplerParameteri(static_cast<GLuint>(sampler), static_cast<GLenum>(pname), static_cast<GLint>(param));
}
void samplerParameter(uint32_t sampler, SamplerParameterI pname, const int32_t* param)
{
  return glSamplerParameteriv(
    static_cast<GLuint>(sampler), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<const GLint*>(param)));
}
void vertexAttribDivisor(uint32_t index, uint32_t divisor)
{
  return glVertexAttribDivisor(static_cast<GLuint>(index), static_cast<GLuint>(divisor));
}
void vertexAttribP1(uint32_t index, VertexAttribPointerType type, bool normalized, uint32_t value)
{
  return glVertexAttribP1ui(static_cast<GLuint>(index),
                            static_cast<GLenum>(type),
                            static_cast<GLboolean>(normalized),
                            static_cast<GLuint>(value));
}
void vertexAttribP1(uint32_t index, VertexAttribPointerType type, bool normalized, const uint32_t* value)
{
  return glVertexAttribP1uiv(static_cast<GLuint>(index),
                             static_cast<GLenum>(type),
                             static_cast<GLboolean>(normalized),
                             detail::constAway(reinterpret_cast<const GLuint*>(value)));
}
void vertexAttribP2(uint32_t index, VertexAttribPointerType type, bool normalized, uint32_t value)
{
  return glVertexAttribP2ui(static_cast<GLuint>(index),
                            static_cast<GLenum>(type),
                            static_cast<GLboolean>(normalized),
                            static_cast<GLuint>(value));
}
void vertexAttribP2(uint32_t index, VertexAttribPointerType type, bool normalized, const uint32_t* value)
{
  return glVertexAttribP2uiv(static_cast<GLuint>(index),
                             static_cast<GLenum>(type),
                             static_cast<GLboolean>(normalized),
                             detail::constAway(reinterpret_cast<const GLuint*>(value)));
}
void vertexAttribP3(uint32_t index, VertexAttribPointerType type, bool normalized, uint32_t value)
{
  return glVertexAttribP3ui(static_cast<GLuint>(index),
                            static_cast<GLenum>(type),
                            static_cast<GLboolean>(normalized),
                            static_cast<GLuint>(value));
}
void vertexAttribP3(uint32_t index, VertexAttribPointerType type, bool normalized, const uint32_t* value)
{
  return glVertexAttribP3uiv(static_cast<GLuint>(index),
                             static_cast<GLenum>(type),
                             static_cast<GLboolean>(normalized),
                             detail::constAway(reinterpret_cast<const GLuint*>(value)));
}
void vertexAttribP4(uint32_t index, VertexAttribPointerType type, bool normalized, uint32_t value)
{
  return glVertexAttribP4ui(static_cast<GLuint>(index),
                            static_cast<GLenum>(type),
                            static_cast<GLboolean>(normalized),
                            static_cast<GLuint>(value));
}
void vertexAttribP4(uint32_t index, VertexAttribPointerType type, bool normalized, const uint32_t* value)
{
  return glVertexAttribP4uiv(static_cast<GLuint>(index),
                             static_cast<GLenum>(type),
                             static_cast<GLboolean>(normalized),
                             detail::constAway(reinterpret_cast<const GLuint*>(value)));
}
void beginQueryIndexed(QueryTarget target, uint32_t index, uint32_t id)
{
  return glBeginQueryIndexed(static_cast<GLenum>(target), static_cast<GLuint>(index), static_cast<GLuint>(id));
}
void bindTransformFeedback(BindTransformFeedbackTarget target, uint32_t id)
{
  return glBindTransformFeedback(static_cast<GLenum>(target), static_cast<GLuint>(id));
}
void blendEquationSeparate(uint32_t buf, BlendEquationModeEXT modeRGB, BlendEquationModeEXT modeAlpha)
{
  return glBlendEquationSeparatei(
    static_cast<GLuint>(buf), static_cast<GLenum>(modeRGB), static_cast<GLenum>(modeAlpha));
}
void blendEquation(uint32_t buf, BlendEquationModeEXT mode)
{
  return glBlendEquationi(static_cast<GLuint>(buf), static_cast<GLenum>(mode));
}
void blendFuncSeparate(
  uint32_t buf, BlendingFactor srcRGB, BlendingFactor dstRGB, BlendingFactor srcAlpha, BlendingFactor dstAlpha)
{
  return glBlendFuncSeparatei(static_cast<GLuint>(buf),
                              static_cast<GLenum>(srcRGB),
                              static_cast<GLenum>(dstRGB),
                              static_cast<GLenum>(srcAlpha),
                              static_cast<GLenum>(dstAlpha));
}
void blendFunc(uint32_t buf, BlendingFactor src, BlendingFactor dst)
{
  return glBlendFunci(static_cast<GLuint>(buf), static_cast<GLenum>(src), static_cast<GLenum>(dst));
}
void deleteTransformFeedback(core::SizeType n, const uint32_t* ids)
{
  return glDeleteTransformFeedbacks(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<const GLuint*>(ids)));
}
void drawArraysIndirect(PrimitiveType mode, const void* indirect)
{
  return glDrawArraysIndirect(static_cast<GLenum>(mode), detail::constAway(indirect));
}
void drawElementsIndirect(PrimitiveType mode, DrawElementsType type, const void* indirect)
{
  return glDrawElementsIndirect(static_cast<GLenum>(mode), static_cast<GLenum>(type), detail::constAway(indirect));
}
void drawTransformFeedback(PrimitiveType mode, uint32_t id)
{
  return glDrawTransformFeedback(static_cast<GLenum>(mode), static_cast<GLuint>(id));
}
void drawTransformFeedbackStream(PrimitiveType mode, uint32_t id, uint32_t stream)
{
  return glDrawTransformFeedbackStream(static_cast<GLenum>(mode), static_cast<GLuint>(id), static_cast<GLuint>(stream));
}
void endQueryIndexed(QueryTarget target, uint32_t index)
{
  return glEndQueryIndexed(static_cast<GLenum>(target), static_cast<GLuint>(index));
}
void genTransformFeedback(core::SizeType n, uint32_t* ids)
{
  return glGenTransformFeedbacks(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<GLuint*>(ids)));
}
void getActiveSubroutineName(
  uint32_t program, ShaderType shadertype, uint32_t index, core::SizeType bufSize, core::SizeType* length, char* name)
{
  return glGetActiveSubroutineName(static_cast<GLuint>(program),
                                   static_cast<GLenum>(shadertype),
                                   static_cast<GLuint>(index),
                                   static_cast<GLsizei>(bufSize),
                                   detail::constAway(reinterpret_cast<GLsizei*>(length)),
                                   detail::constAway(reinterpret_cast<GLchar*>(name)));
}
void getActiveSubroutineUniformName(
  uint32_t program, ShaderType shadertype, uint32_t index, core::SizeType bufSize, core::SizeType* length, char* name)
{
  return glGetActiveSubroutineUniformName(static_cast<GLuint>(program),
                                          static_cast<GLenum>(shadertype),
                                          static_cast<GLuint>(index),
                                          static_cast<GLsizei>(bufSize),
                                          detail::constAway(reinterpret_cast<GLsizei*>(length)),
                                          detail::constAway(reinterpret_cast<GLchar*>(name)));
}
void getActiveSubroutineUniform(
  uint32_t program, ShaderType shadertype, uint32_t index, SubroutineParameterName pname, int32_t* values)
{
  return glGetActiveSubroutineUniformiv(static_cast<GLuint>(program),
                                        static_cast<GLenum>(shadertype),
                                        static_cast<GLuint>(index),
                                        static_cast<GLenum>(pname),
                                        detail::constAway(reinterpret_cast<GLint*>(values)));
}
void getProgramStage(uint32_t program, ShaderType shadertype, ProgramStagePName pname, int32_t* values)
{
  return glGetProgramStageiv(static_cast<GLuint>(program),
                             static_cast<GLenum>(shadertype),
                             static_cast<GLenum>(pname),
                             detail::constAway(reinterpret_cast<GLint*>(values)));
}
void getQueryIndexed(QueryTarget target, uint32_t index, QueryParameterName pname, int32_t* params)
{
  return glGetQueryIndexediv(static_cast<GLenum>(target),
                             static_cast<GLuint>(index),
                             static_cast<GLenum>(pname),
                             detail::constAway(reinterpret_cast<GLint*>(params)));
}
uint32_t getSubroutineIndex(uint32_t program, ShaderType shadertype, const char* name)
{
  return static_cast<uint32_t>(glGetSubroutineIndex(static_cast<GLuint>(program),
                                                    static_cast<GLenum>(shadertype),
                                                    detail::constAway(reinterpret_cast<const GLchar*>(name))));
}
int32_t getSubroutineUniformLocation(uint32_t program, ShaderType shadertype, const char* name)
{
  return static_cast<int32_t>(glGetSubroutineUniformLocation(static_cast<GLuint>(program),
                                                             static_cast<GLenum>(shadertype),
                                                             detail::constAway(reinterpret_cast<const GLchar*>(name))));
}
void getUniformSubroutine(ShaderType shadertype, int32_t location, uint32_t* params)
{
  return glGetUniformSubroutineuiv(static_cast<GLenum>(shadertype),
                                   static_cast<GLint>(location),
                                   detail::constAway(reinterpret_cast<GLuint*>(params)));
}
void getUniform(uint32_t program, int32_t location, double* params)
{
  return glGetUniformdv(
    static_cast<GLuint>(program), static_cast<GLint>(location), detail::constAway(reinterpret_cast<GLdouble*>(params)));
}
bool isTransformFeedback(uint32_t id)
{
  return static_cast<bool>(glIsTransformFeedback(static_cast<GLuint>(id)));
}
void minSampleShading(float value)
{
  return glMinSampleShading(static_cast<GLfloat>(value));
}
void patchParameter(PatchParameterName pname, const float* values)
{
  return glPatchParameterfv(static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<const GLfloat*>(values)));
}
void patchParameter(PatchParameterName pname, int32_t value)
{
  return glPatchParameteri(static_cast<GLenum>(pname), static_cast<GLint>(value));
}
void pauseTransformFeedback()
{
  return glPauseTransformFeedback();
}
void resumeTransformFeedback()
{
  return glResumeTransformFeedback();
}
void uniform1(int32_t location, double x)
{
  return glUniform1d(static_cast<GLint>(location), static_cast<GLdouble>(x));
}
void uniform1(int32_t location, core::SizeType count, const double* value)
{
  return glUniform1dv(static_cast<GLint>(location),
                      static_cast<GLsizei>(count),
                      detail::constAway(reinterpret_cast<const GLdouble*>(value)));
}
void uniform2(int32_t location, double x, double y)
{
  return glUniform2d(static_cast<GLint>(location), static_cast<GLdouble>(x), static_cast<GLdouble>(y));
}
void uniform2(int32_t location, core::SizeType count, const double* value)
{
  return glUniform2dv(static_cast<GLint>(location),
                      static_cast<GLsizei>(count),
                      detail::constAway(reinterpret_cast<const GLdouble*>(value)));
}
void uniform3(int32_t location, double x, double y, double z)
{
  return glUniform3d(
    static_cast<GLint>(location), static_cast<GLdouble>(x), static_cast<GLdouble>(y), static_cast<GLdouble>(z));
}
void uniform3(int32_t location, core::SizeType count, const double* value)
{
  return glUniform3dv(static_cast<GLint>(location),
                      static_cast<GLsizei>(count),
                      detail::constAway(reinterpret_cast<const GLdouble*>(value)));
}
void uniform4(int32_t location, double x, double y, double z, double w)
{
  return glUniform4d(static_cast<GLint>(location),
                     static_cast<GLdouble>(x),
                     static_cast<GLdouble>(y),
                     static_cast<GLdouble>(z),
                     static_cast<GLdouble>(w));
}
void uniform4(int32_t location, core::SizeType count, const double* value)
{
  return glUniform4dv(static_cast<GLint>(location),
                      static_cast<GLsizei>(count),
                      detail::constAway(reinterpret_cast<const GLdouble*>(value)));
}
void uniformMatrix2(int32_t location, core::SizeType count, bool transpose, const double* value)
{
  return glUniformMatrix2dv(static_cast<GLint>(location),
                            static_cast<GLsizei>(count),
                            static_cast<GLboolean>(transpose),
                            detail::constAway(reinterpret_cast<const GLdouble*>(value)));
}
void uniformMatrix2x3(int32_t location, core::SizeType count, bool transpose, const double* value)
{
  return glUniformMatrix2x3dv(static_cast<GLint>(location),
                              static_cast<GLsizei>(count),
                              static_cast<GLboolean>(transpose),
                              detail::constAway(reinterpret_cast<const GLdouble*>(value)));
}
void uniformMatrix2x4(int32_t location, core::SizeType count, bool transpose, const double* value)
{
  return glUniformMatrix2x4dv(static_cast<GLint>(location),
                              static_cast<GLsizei>(count),
                              static_cast<GLboolean>(transpose),
                              detail::constAway(reinterpret_cast<const GLdouble*>(value)));
}
void uniformMatrix3(int32_t location, core::SizeType count, bool transpose, const double* value)
{
  return glUniformMatrix3dv(static_cast<GLint>(location),
                            static_cast<GLsizei>(count),
                            static_cast<GLboolean>(transpose),
                            detail::constAway(reinterpret_cast<const GLdouble*>(value)));
}
void uniformMatrix3x2(int32_t location, core::SizeType count, bool transpose, const double* value)
{
  return glUniformMatrix3x2dv(static_cast<GLint>(location),
                              static_cast<GLsizei>(count),
                              static_cast<GLboolean>(transpose),
                              detail::constAway(reinterpret_cast<const GLdouble*>(value)));
}
void uniformMatrix3x4(int32_t location, core::SizeType count, bool transpose, const double* value)
{
  return glUniformMatrix3x4dv(static_cast<GLint>(location),
                              static_cast<GLsizei>(count),
                              static_cast<GLboolean>(transpose),
                              detail::constAway(reinterpret_cast<const GLdouble*>(value)));
}
void uniformMatrix4(int32_t location, core::SizeType count, bool transpose, const double* value)
{
  return glUniformMatrix4dv(static_cast<GLint>(location),
                            static_cast<GLsizei>(count),
                            static_cast<GLboolean>(transpose),
                            detail::constAway(reinterpret_cast<const GLdouble*>(value)));
}
void uniformMatrix4x2(int32_t location, core::SizeType count, bool transpose, const double* value)
{
  return glUniformMatrix4x2dv(static_cast<GLint>(location),
                              static_cast<GLsizei>(count),
                              static_cast<GLboolean>(transpose),
                              detail::constAway(reinterpret_cast<const GLdouble*>(value)));
}
void uniformMatrix4x3(int32_t location, core::SizeType count, bool transpose, const double* value)
{
  return glUniformMatrix4x3dv(static_cast<GLint>(location),
                              static_cast<GLsizei>(count),
                              static_cast<GLboolean>(transpose),
                              detail::constAway(reinterpret_cast<const GLdouble*>(value)));
}
void uniformSubroutines(ShaderType shadertype, core::SizeType count, const uint32_t* indices)
{
  return glUniformSubroutinesuiv(static_cast<GLenum>(shadertype),
                                 static_cast<GLsizei>(count),
                                 detail::constAway(reinterpret_cast<const GLuint*>(indices)));
}
void activeShaderProgram(uint32_t pipeline, uint32_t program)
{
  return glActiveShaderProgram(static_cast<GLuint>(pipeline), static_cast<GLuint>(program));
}
void bindProgramPipeline(uint32_t pipeline)
{
  return glBindProgramPipeline(static_cast<GLuint>(pipeline));
}
void clearDepth(float d)
{
  return glClearDepthf(static_cast<GLfloat>(d));
}
uint32_t createShaderProgram(ShaderType type, core::SizeType count, const char* const* strings)
{
  return static_cast<uint32_t>(
    glCreateShaderProgramv(static_cast<GLenum>(type),
                           static_cast<GLsizei>(count),
                           detail::constAway(reinterpret_cast<const GLchar* const*>(strings))));
}
void deleteProgramPipeline(core::SizeType n, const uint32_t* pipelines)
{
  return glDeleteProgramPipelines(static_cast<GLsizei>(n),
                                  detail::constAway(reinterpret_cast<const GLuint*>(pipelines)));
}
void depthRangeArray(uint32_t first, core::SizeType count, const double* v)
{
  return glDepthRangeArrayv(
    static_cast<GLuint>(first), static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLdouble*>(v)));
}
void depthRangeIndexed(uint32_t index, double n, double f)
{
  return glDepthRangeIndexed(static_cast<GLuint>(index), static_cast<GLdouble>(n), static_cast<GLdouble>(f));
}
void depthRange(float n, float f)
{
  return glDepthRangef(static_cast<GLfloat>(n), static_cast<GLfloat>(f));
}
void genProgramPipeline(core::SizeType n, uint32_t* pipelines)
{
  return glGenProgramPipelines(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<GLuint*>(pipelines)));
}
void getDoubleI(GetPName target, uint32_t index, double* data)
{
  return glGetDoublei_v(
    static_cast<GLenum>(target), static_cast<GLuint>(index), detail::constAway(reinterpret_cast<GLdouble*>(data)));
}
void getFloatI(GetPName target, uint32_t index, float* data)
{
  return glGetFloati_v(
    static_cast<GLenum>(target), static_cast<GLuint>(index), detail::constAway(reinterpret_cast<GLfloat*>(data)));
}
void getProgramBinary(
  uint32_t program, core::SizeType bufSize, core::SizeType* length, core::EnumType* binaryFormat, void* binary)
{
  return glGetProgramBinary(static_cast<GLuint>(program),
                            static_cast<GLsizei>(bufSize),
                            detail::constAway(reinterpret_cast<GLsizei*>(length)),
                            detail::constAway(reinterpret_cast<GLenum*>(binaryFormat)),
                            detail::constAway(binary));
}
void getProgramPipelineInfoLog(uint32_t pipeline, core::SizeType bufSize, core::SizeType* length, char* infoLog)
{
  return glGetProgramPipelineInfoLog(static_cast<GLuint>(pipeline),
                                     static_cast<GLsizei>(bufSize),
                                     detail::constAway(reinterpret_cast<GLsizei*>(length)),
                                     detail::constAway(reinterpret_cast<GLchar*>(infoLog)));
}
void getProgramPipeline(uint32_t pipeline, PipelineParameterName pname, int32_t* params)
{
  return glGetProgramPipelineiv(
    static_cast<GLuint>(pipeline), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint*>(params)));
}
void getShaderPrecisionFormat(ShaderType shadertype, PrecisionType precisiontype, int32_t* range, int32_t* precision)
{
  return glGetShaderPrecisionFormat(static_cast<GLenum>(shadertype),
                                    static_cast<GLenum>(precisiontype),
                                    detail::constAway(reinterpret_cast<GLint*>(range)),
                                    detail::constAway(reinterpret_cast<GLint*>(precision)));
}
void getVertexAttribL(uint32_t index, VertexAttribEnum pname, double* params)
{
  return glGetVertexAttribLdv(
    static_cast<GLuint>(index), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLdouble*>(params)));
}
bool isProgramPipeline(uint32_t pipeline)
{
  return static_cast<bool>(glIsProgramPipeline(static_cast<GLuint>(pipeline)));
}
void programBinary(uint32_t program, core::EnumType binaryFormat, const void* binary, core::SizeType length)
{
  return glProgramBinary(static_cast<GLuint>(program),
                         static_cast<GLenum>(binaryFormat),
                         detail::constAway(binary),
                         static_cast<GLsizei>(length));
}
void programParameter(uint32_t program, ProgramParameterPName pname, int32_t value)
{
  return glProgramParameteri(static_cast<GLuint>(program), static_cast<GLenum>(pname), static_cast<GLint>(value));
}
void programUniform1(uint32_t program, int32_t location, double v0)
{
  return glProgramUniform1d(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLdouble>(v0));
}
void programUniform1(uint32_t program, int32_t location, core::SizeType count, const double* value)
{
  return glProgramUniform1dv(static_cast<GLuint>(program),
                             static_cast<GLint>(location),
                             static_cast<GLsizei>(count),
                             detail::constAway(reinterpret_cast<const GLdouble*>(value)));
}
void programUniform1(uint32_t program, int32_t location, float v0)
{
  return glProgramUniform1f(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLfloat>(v0));
}
void programUniform1(uint32_t program, int32_t location, core::SizeType count, const float* value)
{
  return glProgramUniform1fv(static_cast<GLuint>(program),
                             static_cast<GLint>(location),
                             static_cast<GLsizei>(count),
                             detail::constAway(reinterpret_cast<const GLfloat*>(value)));
}
void programUniform1(uint32_t program, int32_t location, int32_t v0)
{
  return glProgramUniform1i(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLint>(v0));
}
void programUniform1(uint32_t program, int32_t location, core::SizeType count, const int32_t* value)
{
  return glProgramUniform1iv(static_cast<GLuint>(program),
                             static_cast<GLint>(location),
                             static_cast<GLsizei>(count),
                             detail::constAway(reinterpret_cast<const GLint*>(value)));
}
void programUniform1(uint32_t program, int32_t location, uint32_t v0)
{
  return glProgramUniform1ui(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLuint>(v0));
}
void programUniform1(uint32_t program, int32_t location, core::SizeType count, const uint32_t* value)
{
  return glProgramUniform1uiv(static_cast<GLuint>(program),
                              static_cast<GLint>(location),
                              static_cast<GLsizei>(count),
                              detail::constAway(reinterpret_cast<const GLuint*>(value)));
}
void programUniform2(uint32_t program, int32_t location, double v0, double v1)
{
  return glProgramUniform2d(
    static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLdouble>(v0), static_cast<GLdouble>(v1));
}
void programUniform2(uint32_t program, int32_t location, core::SizeType count, const double* value)
{
  return glProgramUniform2dv(static_cast<GLuint>(program),
                             static_cast<GLint>(location),
                             static_cast<GLsizei>(count),
                             detail::constAway(reinterpret_cast<const GLdouble*>(value)));
}
void programUniform2(uint32_t program, int32_t location, float v0, float v1)
{
  return glProgramUniform2f(
    static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLfloat>(v0), static_cast<GLfloat>(v1));
}
void programUniform2(uint32_t program, int32_t location, core::SizeType count, const float* value)
{
  return glProgramUniform2fv(static_cast<GLuint>(program),
                             static_cast<GLint>(location),
                             static_cast<GLsizei>(count),
                             detail::constAway(reinterpret_cast<const GLfloat*>(value)));
}
void programUniform2(uint32_t program, int32_t location, int32_t v0, int32_t v1)
{
  return glProgramUniform2i(
    static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLint>(v0), static_cast<GLint>(v1));
}
void programUniform2(uint32_t program, int32_t location, core::SizeType count, const int32_t* value)
{
  return glProgramUniform2iv(static_cast<GLuint>(program),
                             static_cast<GLint>(location),
                             static_cast<GLsizei>(count),
                             detail::constAway(reinterpret_cast<const GLint*>(value)));
}
void programUniform2(uint32_t program, int32_t location, uint32_t v0, uint32_t v1)
{
  return glProgramUniform2ui(
    static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLuint>(v0), static_cast<GLuint>(v1));
}
void programUniform2(uint32_t program, int32_t location, core::SizeType count, const uint32_t* value)
{
  return glProgramUniform2uiv(static_cast<GLuint>(program),
                              static_cast<GLint>(location),
                              static_cast<GLsizei>(count),
                              detail::constAway(reinterpret_cast<const GLuint*>(value)));
}
void programUniform3(uint32_t program, int32_t location, double v0, double v1, double v2)
{
  return glProgramUniform3d(static_cast<GLuint>(program),
                            static_cast<GLint>(location),
                            static_cast<GLdouble>(v0),
                            static_cast<GLdouble>(v1),
                            static_cast<GLdouble>(v2));
}
void programUniform3(uint32_t program, int32_t location, core::SizeType count, const double* value)
{
  return glProgramUniform3dv(static_cast<GLuint>(program),
                             static_cast<GLint>(location),
                             static_cast<GLsizei>(count),
                             detail::constAway(reinterpret_cast<const GLdouble*>(value)));
}
void programUniform3(uint32_t program, int32_t location, float v0, float v1, float v2)
{
  return glProgramUniform3f(static_cast<GLuint>(program),
                            static_cast<GLint>(location),
                            static_cast<GLfloat>(v0),
                            static_cast<GLfloat>(v1),
                            static_cast<GLfloat>(v2));
}
void programUniform3(uint32_t program, int32_t location, core::SizeType count, const float* value)
{
  return glProgramUniform3fv(static_cast<GLuint>(program),
                             static_cast<GLint>(location),
                             static_cast<GLsizei>(count),
                             detail::constAway(reinterpret_cast<const GLfloat*>(value)));
}
void programUniform3(uint32_t program, int32_t location, int32_t v0, int32_t v1, int32_t v2)
{
  return glProgramUniform3i(static_cast<GLuint>(program),
                            static_cast<GLint>(location),
                            static_cast<GLint>(v0),
                            static_cast<GLint>(v1),
                            static_cast<GLint>(v2));
}
void programUniform3(uint32_t program, int32_t location, core::SizeType count, const int32_t* value)
{
  return glProgramUniform3iv(static_cast<GLuint>(program),
                             static_cast<GLint>(location),
                             static_cast<GLsizei>(count),
                             detail::constAway(reinterpret_cast<const GLint*>(value)));
}
void programUniform3(uint32_t program, int32_t location, uint32_t v0, uint32_t v1, uint32_t v2)
{
  return glProgramUniform3ui(static_cast<GLuint>(program),
                             static_cast<GLint>(location),
                             static_cast<GLuint>(v0),
                             static_cast<GLuint>(v1),
                             static_cast<GLuint>(v2));
}
void programUniform3(uint32_t program, int32_t location, core::SizeType count, const uint32_t* value)
{
  return glProgramUniform3uiv(static_cast<GLuint>(program),
                              static_cast<GLint>(location),
                              static_cast<GLsizei>(count),
                              detail::constAway(reinterpret_cast<const GLuint*>(value)));
}
void programUniform4(uint32_t program, int32_t location, double v0, double v1, double v2, double v3)
{
  return glProgramUniform4d(static_cast<GLuint>(program),
                            static_cast<GLint>(location),
                            static_cast<GLdouble>(v0),
                            static_cast<GLdouble>(v1),
                            static_cast<GLdouble>(v2),
                            static_cast<GLdouble>(v3));
}
void programUniform4(uint32_t program, int32_t location, core::SizeType count, const double* value)
{
  return glProgramUniform4dv(static_cast<GLuint>(program),
                             static_cast<GLint>(location),
                             static_cast<GLsizei>(count),
                             detail::constAway(reinterpret_cast<const GLdouble*>(value)));
}
void programUniform4(uint32_t program, int32_t location, float v0, float v1, float v2, float v3)
{
  return glProgramUniform4f(static_cast<GLuint>(program),
                            static_cast<GLint>(location),
                            static_cast<GLfloat>(v0),
                            static_cast<GLfloat>(v1),
                            static_cast<GLfloat>(v2),
                            static_cast<GLfloat>(v3));
}
void programUniform4(uint32_t program, int32_t location, core::SizeType count, const float* value)
{
  return glProgramUniform4fv(static_cast<GLuint>(program),
                             static_cast<GLint>(location),
                             static_cast<GLsizei>(count),
                             detail::constAway(reinterpret_cast<const GLfloat*>(value)));
}
void programUniform4(uint32_t program, int32_t location, int32_t v0, int32_t v1, int32_t v2, int32_t v3)
{
  return glProgramUniform4i(static_cast<GLuint>(program),
                            static_cast<GLint>(location),
                            static_cast<GLint>(v0),
                            static_cast<GLint>(v1),
                            static_cast<GLint>(v2),
                            static_cast<GLint>(v3));
}
void programUniform4(uint32_t program, int32_t location, core::SizeType count, const int32_t* value)
{
  return glProgramUniform4iv(static_cast<GLuint>(program),
                             static_cast<GLint>(location),
                             static_cast<GLsizei>(count),
                             detail::constAway(reinterpret_cast<const GLint*>(value)));
}
void programUniform4(uint32_t program, int32_t location, uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3)
{
  return glProgramUniform4ui(static_cast<GLuint>(program),
                             static_cast<GLint>(location),
                             static_cast<GLuint>(v0),
                             static_cast<GLuint>(v1),
                             static_cast<GLuint>(v2),
                             static_cast<GLuint>(v3));
}
void programUniform4(uint32_t program, int32_t location, core::SizeType count, const uint32_t* value)
{
  return glProgramUniform4uiv(static_cast<GLuint>(program),
                              static_cast<GLint>(location),
                              static_cast<GLsizei>(count),
                              detail::constAway(reinterpret_cast<const GLuint*>(value)));
}
void programUniformMatrix2(
  uint32_t program, int32_t location, core::SizeType count, bool transpose, const double* value)
{
  return glProgramUniformMatrix2dv(static_cast<GLuint>(program),
                                   static_cast<GLint>(location),
                                   static_cast<GLsizei>(count),
                                   static_cast<GLboolean>(transpose),
                                   detail::constAway(reinterpret_cast<const GLdouble*>(value)));
}
void programUniformMatrix2(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float* value)
{
  return glProgramUniformMatrix2fv(static_cast<GLuint>(program),
                                   static_cast<GLint>(location),
                                   static_cast<GLsizei>(count),
                                   static_cast<GLboolean>(transpose),
                                   detail::constAway(reinterpret_cast<const GLfloat*>(value)));
}
void programUniformMatrix2x3(
  uint32_t program, int32_t location, core::SizeType count, bool transpose, const double* value)
{
  return glProgramUniformMatrix2x3dv(static_cast<GLuint>(program),
                                     static_cast<GLint>(location),
                                     static_cast<GLsizei>(count),
                                     static_cast<GLboolean>(transpose),
                                     detail::constAway(reinterpret_cast<const GLdouble*>(value)));
}
void programUniformMatrix2x3(
  uint32_t program, int32_t location, core::SizeType count, bool transpose, const float* value)
{
  return glProgramUniformMatrix2x3fv(static_cast<GLuint>(program),
                                     static_cast<GLint>(location),
                                     static_cast<GLsizei>(count),
                                     static_cast<GLboolean>(transpose),
                                     detail::constAway(reinterpret_cast<const GLfloat*>(value)));
}
void programUniformMatrix2x4(
  uint32_t program, int32_t location, core::SizeType count, bool transpose, const double* value)
{
  return glProgramUniformMatrix2x4dv(static_cast<GLuint>(program),
                                     static_cast<GLint>(location),
                                     static_cast<GLsizei>(count),
                                     static_cast<GLboolean>(transpose),
                                     detail::constAway(reinterpret_cast<const GLdouble*>(value)));
}
void programUniformMatrix2x4(
  uint32_t program, int32_t location, core::SizeType count, bool transpose, const float* value)
{
  return glProgramUniformMatrix2x4fv(static_cast<GLuint>(program),
                                     static_cast<GLint>(location),
                                     static_cast<GLsizei>(count),
                                     static_cast<GLboolean>(transpose),
                                     detail::constAway(reinterpret_cast<const GLfloat*>(value)));
}
void programUniformMatrix3(
  uint32_t program, int32_t location, core::SizeType count, bool transpose, const double* value)
{
  return glProgramUniformMatrix3dv(static_cast<GLuint>(program),
                                   static_cast<GLint>(location),
                                   static_cast<GLsizei>(count),
                                   static_cast<GLboolean>(transpose),
                                   detail::constAway(reinterpret_cast<const GLdouble*>(value)));
}
void programUniformMatrix3(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float* value)
{
  return glProgramUniformMatrix3fv(static_cast<GLuint>(program),
                                   static_cast<GLint>(location),
                                   static_cast<GLsizei>(count),
                                   static_cast<GLboolean>(transpose),
                                   detail::constAway(reinterpret_cast<const GLfloat*>(value)));
}
void programUniformMatrix3x2(
  uint32_t program, int32_t location, core::SizeType count, bool transpose, const double* value)
{
  return glProgramUniformMatrix3x2dv(static_cast<GLuint>(program),
                                     static_cast<GLint>(location),
                                     static_cast<GLsizei>(count),
                                     static_cast<GLboolean>(transpose),
                                     detail::constAway(reinterpret_cast<const GLdouble*>(value)));
}
void programUniformMatrix3x2(
  uint32_t program, int32_t location, core::SizeType count, bool transpose, const float* value)
{
  return glProgramUniformMatrix3x2fv(static_cast<GLuint>(program),
                                     static_cast<GLint>(location),
                                     static_cast<GLsizei>(count),
                                     static_cast<GLboolean>(transpose),
                                     detail::constAway(reinterpret_cast<const GLfloat*>(value)));
}
void programUniformMatrix3x4(
  uint32_t program, int32_t location, core::SizeType count, bool transpose, const double* value)
{
  return glProgramUniformMatrix3x4dv(static_cast<GLuint>(program),
                                     static_cast<GLint>(location),
                                     static_cast<GLsizei>(count),
                                     static_cast<GLboolean>(transpose),
                                     detail::constAway(reinterpret_cast<const GLdouble*>(value)));
}
void programUniformMatrix3x4(
  uint32_t program, int32_t location, core::SizeType count, bool transpose, const float* value)
{
  return glProgramUniformMatrix3x4fv(static_cast<GLuint>(program),
                                     static_cast<GLint>(location),
                                     static_cast<GLsizei>(count),
                                     static_cast<GLboolean>(transpose),
                                     detail::constAway(reinterpret_cast<const GLfloat*>(value)));
}
void programUniformMatrix4(
  uint32_t program, int32_t location, core::SizeType count, bool transpose, const double* value)
{
  return glProgramUniformMatrix4dv(static_cast<GLuint>(program),
                                   static_cast<GLint>(location),
                                   static_cast<GLsizei>(count),
                                   static_cast<GLboolean>(transpose),
                                   detail::constAway(reinterpret_cast<const GLdouble*>(value)));
}
void programUniformMatrix4(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float* value)
{
  return glProgramUniformMatrix4fv(static_cast<GLuint>(program),
                                   static_cast<GLint>(location),
                                   static_cast<GLsizei>(count),
                                   static_cast<GLboolean>(transpose),
                                   detail::constAway(reinterpret_cast<const GLfloat*>(value)));
}
void programUniformMatrix4x2(
  uint32_t program, int32_t location, core::SizeType count, bool transpose, const double* value)
{
  return glProgramUniformMatrix4x2dv(static_cast<GLuint>(program),
                                     static_cast<GLint>(location),
                                     static_cast<GLsizei>(count),
                                     static_cast<GLboolean>(transpose),
                                     detail::constAway(reinterpret_cast<const GLdouble*>(value)));
}
void programUniformMatrix4x2(
  uint32_t program, int32_t location, core::SizeType count, bool transpose, const float* value)
{
  return glProgramUniformMatrix4x2fv(static_cast<GLuint>(program),
                                     static_cast<GLint>(location),
                                     static_cast<GLsizei>(count),
                                     static_cast<GLboolean>(transpose),
                                     detail::constAway(reinterpret_cast<const GLfloat*>(value)));
}
void programUniformMatrix4x3(
  uint32_t program, int32_t location, core::SizeType count, bool transpose, const double* value)
{
  return glProgramUniformMatrix4x3dv(static_cast<GLuint>(program),
                                     static_cast<GLint>(location),
                                     static_cast<GLsizei>(count),
                                     static_cast<GLboolean>(transpose),
                                     detail::constAway(reinterpret_cast<const GLdouble*>(value)));
}
void programUniformMatrix4x3(
  uint32_t program, int32_t location, core::SizeType count, bool transpose, const float* value)
{
  return glProgramUniformMatrix4x3fv(static_cast<GLuint>(program),
                                     static_cast<GLint>(location),
                                     static_cast<GLsizei>(count),
                                     static_cast<GLboolean>(transpose),
                                     detail::constAway(reinterpret_cast<const GLfloat*>(value)));
}
void releaseShaderCompiler()
{
  return glReleaseShaderCompiler();
}
void scissorArray(uint32_t first, core::SizeType count, const int32_t* v)
{
  return glScissorArrayv(
    static_cast<GLuint>(first), static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLint*>(v)));
}
void scissorIndexed(uint32_t index, int32_t left, int32_t bottom, core::SizeType width, core::SizeType height)
{
  return glScissorIndexed(static_cast<GLuint>(index),
                          static_cast<GLint>(left),
                          static_cast<GLint>(bottom),
                          static_cast<GLsizei>(width),
                          static_cast<GLsizei>(height));
}
void scissorIndexed(uint32_t index, const int32_t* v)
{
  return glScissorIndexedv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLint*>(v)));
}
void useProgramStage(uint32_t pipeline, core::Bitfield<UseProgramStageMask> stages, uint32_t program)
{
  return glUseProgramStages(static_cast<GLuint>(pipeline), stages.value(), static_cast<GLuint>(program));
}
void validateProgramPipeline(uint32_t pipeline)
{
  return glValidateProgramPipeline(static_cast<GLuint>(pipeline));
}
void vertexAttribL1(uint32_t index, double x)
{
  return glVertexAttribL1d(static_cast<GLuint>(index), static_cast<GLdouble>(x));
}
void vertexAttribL1(uint32_t index, const double* v)
{
  return glVertexAttribL1dv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLdouble*>(v)));
}
void vertexAttribL2(uint32_t index, double x, double y)
{
  return glVertexAttribL2d(static_cast<GLuint>(index), static_cast<GLdouble>(x), static_cast<GLdouble>(y));
}
void vertexAttribL2(uint32_t index, const double* v)
{
  return glVertexAttribL2dv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLdouble*>(v)));
}
void vertexAttribL3(uint32_t index, double x, double y, double z)
{
  return glVertexAttribL3d(
    static_cast<GLuint>(index), static_cast<GLdouble>(x), static_cast<GLdouble>(y), static_cast<GLdouble>(z));
}
void vertexAttribL3(uint32_t index, const double* v)
{
  return glVertexAttribL3dv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLdouble*>(v)));
}
void vertexAttribL4(uint32_t index, double x, double y, double z, double w)
{
  return glVertexAttribL4d(static_cast<GLuint>(index),
                           static_cast<GLdouble>(x),
                           static_cast<GLdouble>(y),
                           static_cast<GLdouble>(z),
                           static_cast<GLdouble>(w));
}
void vertexAttribL4(uint32_t index, const double* v)
{
  return glVertexAttribL4dv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLdouble*>(v)));
}
void vertexAttribLPointer(
  uint32_t index, int32_t size, VertexAttribLType type, core::SizeType stride, const void* pointer)
{
  return glVertexAttribLPointer(static_cast<GLuint>(index),
                                static_cast<GLint>(size),
                                static_cast<GLenum>(type),
                                static_cast<GLsizei>(stride),
                                detail::constAway(pointer));
}
void viewportArray(uint32_t first, core::SizeType count, const float* v)
{
  return glViewportArrayv(
    static_cast<GLuint>(first), static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLfloat*>(v)));
}
void viewportIndexed(uint32_t index, float x, float y, float w, float h)
{
  return glViewportIndexedf(static_cast<GLuint>(index),
                            static_cast<GLfloat>(x),
                            static_cast<GLfloat>(y),
                            static_cast<GLfloat>(w),
                            static_cast<GLfloat>(h));
}
void viewportIndexed(uint32_t index, const float* v)
{
  return glViewportIndexedfv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLfloat*>(v)));
}
void bindImageTexture(uint32_t unit,
                      uint32_t texture,
                      int32_t level,
                      bool layered,
                      int32_t layer,
                      BufferAccess access,
                      InternalFormat format)
{
  return glBindImageTexture(static_cast<GLuint>(unit),
                            static_cast<GLuint>(texture),
                            static_cast<GLint>(level),
                            static_cast<GLboolean>(layered),
                            static_cast<GLint>(layer),
                            static_cast<GLenum>(access),
                            static_cast<GLenum>(format));
}
void drawArraysInstancedBaseInstance(
  PrimitiveType mode, int32_t first, core::SizeType count, core::SizeType instancecount, uint32_t baseinstance)
{
  return glDrawArraysInstancedBaseInstance(static_cast<GLenum>(mode),
                                           static_cast<GLint>(first),
                                           static_cast<GLsizei>(count),
                                           static_cast<GLsizei>(instancecount),
                                           static_cast<GLuint>(baseinstance));
}
void drawElementsInstancedBaseInstance(PrimitiveType mode,
                                       core::SizeType count,
                                       PrimitiveType type,
                                       const void* indices,
                                       core::SizeType instancecount,
                                       uint32_t baseinstance)
{
  return glDrawElementsInstancedBaseInstance(static_cast<GLenum>(mode),
                                             static_cast<GLsizei>(count),
                                             static_cast<GLenum>(type),
                                             detail::constAway(indices),
                                             static_cast<GLsizei>(instancecount),
                                             static_cast<GLuint>(baseinstance));
}
void drawElementsInstancedBaseVertexBaseInstance(PrimitiveType mode,
                                                 core::SizeType count,
                                                 DrawElementsType type,
                                                 const void* indices,
                                                 core::SizeType instancecount,
                                                 int32_t basevertex,
                                                 uint32_t baseinstance)
{
  return glDrawElementsInstancedBaseVertexBaseInstance(static_cast<GLenum>(mode),
                                                       static_cast<GLsizei>(count),
                                                       static_cast<GLenum>(type),
                                                       detail::constAway(indices),
                                                       static_cast<GLsizei>(instancecount),
                                                       static_cast<GLint>(basevertex),
                                                       static_cast<GLuint>(baseinstance));
}
void drawTransformFeedbackInstanced(PrimitiveType mode, uint32_t id, core::SizeType instancecount)
{
  return glDrawTransformFeedbackInstanced(
    static_cast<GLenum>(mode), static_cast<GLuint>(id), static_cast<GLsizei>(instancecount));
}
void drawTransformFeedbackStreamInstanced(PrimitiveType mode,
                                          uint32_t id,
                                          uint32_t stream,
                                          core::SizeType instancecount)
{
  return glDrawTransformFeedbackStreamInstanced(static_cast<GLenum>(mode),
                                                static_cast<GLuint>(id),
                                                static_cast<GLuint>(stream),
                                                static_cast<GLsizei>(instancecount));
}
void getActiveAtomicCounterBuffer(uint32_t program,
                                  uint32_t bufferIndex,
                                  AtomicCounterBufferPName pname,
                                  int32_t* params)
{
  return glGetActiveAtomicCounterBufferiv(static_cast<GLuint>(program),
                                          static_cast<GLuint>(bufferIndex),
                                          static_cast<GLenum>(pname),
                                          detail::constAway(reinterpret_cast<GLint*>(params)));
}
void getInternalformat(
  TextureTarget target, InternalFormat internalformat, InternalFormatPName pname, core::SizeType count, int32_t* params)
{
  return glGetInternalformativ(static_cast<GLenum>(target),
                               static_cast<GLenum>(internalformat),
                               static_cast<GLenum>(pname),
                               static_cast<GLsizei>(count),
                               detail::constAway(reinterpret_cast<GLint*>(params)));
}
void memoryBarrier(core::Bitfield<MemoryBarrierMask> barriers)
{
  return glMemoryBarrier(barriers.value());
}
void texStorage1D(TextureTarget target, core::SizeType levels, SizedInternalFormat internalformat, core::SizeType width)
{
  return glTexStorage1D(static_cast<GLenum>(target),
                        static_cast<GLsizei>(levels),
                        static_cast<GLenum>(internalformat),
                        static_cast<GLsizei>(width));
}
void texStorage2D(TextureTarget target,
                  core::SizeType levels,
                  SizedInternalFormat internalformat,
                  core::SizeType width,
                  core::SizeType height)
{
  return glTexStorage2D(static_cast<GLenum>(target),
                        static_cast<GLsizei>(levels),
                        static_cast<GLenum>(internalformat),
                        static_cast<GLsizei>(width),
                        static_cast<GLsizei>(height));
}
void texStorage3D(TextureTarget target,
                  core::SizeType levels,
                  SizedInternalFormat internalformat,
                  core::SizeType width,
                  core::SizeType height,
                  core::SizeType depth)
{
  return glTexStorage3D(static_cast<GLenum>(target),
                        static_cast<GLsizei>(levels),
                        static_cast<GLenum>(internalformat),
                        static_cast<GLsizei>(width),
                        static_cast<GLsizei>(height),
                        static_cast<GLsizei>(depth));
}
void bindVertexBuffer(uint32_t bindingindex, uint32_t buffer, std::intptr_t offset, core::SizeType stride)
{
  return glBindVertexBuffer(static_cast<GLuint>(bindingindex),
                            static_cast<GLuint>(buffer),
                            static_cast<GLintptr>(offset),
                            static_cast<GLsizei>(stride));
}
void clearBufferData(
  BufferStorageTarget target, SizedInternalFormat internalformat, PixelFormat format, PixelType type, const void* data)
{
  return glClearBufferData(static_cast<GLenum>(target),
                           static_cast<GLenum>(internalformat),
                           static_cast<GLenum>(format),
                           static_cast<GLenum>(type),
                           detail::constAway(data));
}
void clearBufferSubData(BufferTarget target,
                        SizedInternalFormat internalformat,
                        std::intptr_t offset,
                        std::size_t size,
                        PixelFormat format,
                        PixelType type,
                        const void* data)
{
  return glClearBufferSubData(static_cast<GLenum>(target),
                              static_cast<GLenum>(internalformat),
                              static_cast<GLintptr>(offset),
                              static_cast<GLsizeiptr>(size),
                              static_cast<GLenum>(format),
                              static_cast<GLenum>(type),
                              detail::constAway(data));
}
void copyImageSubData(uint32_t srcName,
                      CopyImageSubDataTarget srcTarget,
                      int32_t srcLevel,
                      int32_t srcX,
                      int32_t srcY,
                      int32_t srcZ,
                      uint32_t dstName,
                      CopyImageSubDataTarget dstTarget,
                      int32_t dstLevel,
                      int32_t dstX,
                      int32_t dstY,
                      int32_t dstZ,
                      core::SizeType srcWidth,
                      core::SizeType srcHeight,
                      core::SizeType srcDepth)
{
  return glCopyImageSubData(static_cast<GLuint>(srcName),
                            static_cast<GLenum>(srcTarget),
                            static_cast<GLint>(srcLevel),
                            static_cast<GLint>(srcX),
                            static_cast<GLint>(srcY),
                            static_cast<GLint>(srcZ),
                            static_cast<GLuint>(dstName),
                            static_cast<GLenum>(dstTarget),
                            static_cast<GLint>(dstLevel),
                            static_cast<GLint>(dstX),
                            static_cast<GLint>(dstY),
                            static_cast<GLint>(dstZ),
                            static_cast<GLsizei>(srcWidth),
                            static_cast<GLsizei>(srcHeight),
                            static_cast<GLsizei>(srcDepth));
}
void debugMessageCallback(core::DebugProc callback, const void* userParam)
{
  return glDebugMessageCallback(reinterpret_cast<GLDEBUGPROC>(callback), detail::constAway(userParam));
}
void debugMessageControl(
  DebugSource source, DebugType type, DebugSeverity severity, core::SizeType count, const uint32_t* ids, bool enabled)
{
  return glDebugMessageControl(static_cast<GLenum>(source),
                               static_cast<GLenum>(type),
                               static_cast<GLenum>(severity),
                               static_cast<GLsizei>(count),
                               detail::constAway(reinterpret_cast<const GLuint*>(ids)),
                               static_cast<GLboolean>(enabled));
}
void debugMessageInsert(
  DebugSource source, DebugType type, uint32_t id, DebugSeverity severity, core::SizeType length, const char* buf)
{
  return glDebugMessageInsert(static_cast<GLenum>(source),
                              static_cast<GLenum>(type),
                              static_cast<GLuint>(id),
                              static_cast<GLenum>(severity),
                              static_cast<GLsizei>(length),
                              detail::constAway(reinterpret_cast<const GLchar*>(buf)));
}
void dispatchCompute(uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z)
{
  return glDispatchCompute(
    static_cast<GLuint>(num_groups_x), static_cast<GLuint>(num_groups_y), static_cast<GLuint>(num_groups_z));
}
void dispatchComputeIndirect(std::intptr_t indirect)
{
  return glDispatchComputeIndirect(static_cast<GLintptr>(indirect));
}
void framebufferParameter(FramebufferTarget target, FramebufferParameterName pname, int32_t param)
{
  return glFramebufferParameteri(static_cast<GLenum>(target), static_cast<GLenum>(pname), static_cast<GLint>(param));
}
uint32_t getDebugMessageLog(uint32_t count,
                            core::SizeType bufSize,
                            DebugSource* sources,
                            DebugType* types,
                            uint32_t* ids,
                            DebugSeverity* severities,
                            core::SizeType* lengths,
                            char* messageLog)
{
  return static_cast<uint32_t>(glGetDebugMessageLog(static_cast<GLuint>(count),
                                                    static_cast<GLsizei>(bufSize),
                                                    detail::constAway(reinterpret_cast<GLenum*>(sources)),
                                                    detail::constAway(reinterpret_cast<GLenum*>(types)),
                                                    detail::constAway(reinterpret_cast<GLuint*>(ids)),
                                                    detail::constAway(reinterpret_cast<GLenum*>(severities)),
                                                    detail::constAway(reinterpret_cast<GLsizei*>(lengths)),
                                                    detail::constAway(reinterpret_cast<GLchar*>(messageLog))));
}
void getFramebufferParameter(FramebufferTarget target, FramebufferAttachmentParameterName pname, int32_t* params)
{
  return glGetFramebufferParameteriv(
    static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint*>(params)));
}
void getInternalformat(
  TextureTarget target, InternalFormat internalformat, InternalFormatPName pname, core::SizeType count, int64_t* params)
{
  return glGetInternalformati64v(static_cast<GLenum>(target),
                                 static_cast<GLenum>(internalformat),
                                 static_cast<GLenum>(pname),
                                 static_cast<GLsizei>(count),
                                 detail::constAway(reinterpret_cast<GLint64*>(params)));
}
void getObjectLabel(
  ObjectIdentifier identifier, uint32_t name, core::SizeType bufSize, core::SizeType* length, char* label)
{
  return glGetObjectLabel(static_cast<GLenum>(identifier),
                          static_cast<GLuint>(name),
                          static_cast<GLsizei>(bufSize),
                          detail::constAway(reinterpret_cast<GLsizei*>(length)),
                          detail::constAway(reinterpret_cast<GLchar*>(label)));
}
void getObjectPtrLabel(const void* ptr, core::SizeType bufSize, core::SizeType* length, char* label)
{
  return glGetObjectPtrLabel(detail::constAway(ptr),
                             static_cast<GLsizei>(bufSize),
                             detail::constAway(reinterpret_cast<GLsizei*>(length)),
                             detail::constAway(reinterpret_cast<GLchar*>(label)));
}
void getProgramInterface(uint32_t program,
                         ProgramInterface programInterface,
                         ProgramInterfacePName pname,
                         int32_t* params)
{
  return glGetProgramInterfaceiv(static_cast<GLuint>(program),
                                 static_cast<GLenum>(programInterface),
                                 static_cast<GLenum>(pname),
                                 detail::constAway(reinterpret_cast<GLint*>(params)));
}
uint32_t getProgramResourceIndex(uint32_t program, ProgramInterface programInterface, const char* name)
{
  return static_cast<uint32_t>(glGetProgramResourceIndex(static_cast<GLuint>(program),
                                                         static_cast<GLenum>(programInterface),
                                                         detail::constAway(reinterpret_cast<const GLchar*>(name))));
}
int32_t getProgramResourceLocation(uint32_t program, ProgramInterface programInterface, const char* name)
{
  return static_cast<int32_t>(glGetProgramResourceLocation(static_cast<GLuint>(program),
                                                           static_cast<GLenum>(programInterface),
                                                           detail::constAway(reinterpret_cast<const GLchar*>(name))));
}
int32_t getProgramResourceLocationIndex(uint32_t program, ProgramInterface programInterface, const char* name)
{
  return static_cast<int32_t>(
    glGetProgramResourceLocationIndex(static_cast<GLuint>(program),
                                      static_cast<GLenum>(programInterface),
                                      detail::constAway(reinterpret_cast<const GLchar*>(name))));
}
void getProgramResourceName(uint32_t program,
                            ProgramInterface programInterface,
                            uint32_t index,
                            core::SizeType bufSize,
                            core::SizeType* length,
                            char* name)
{
  return glGetProgramResourceName(static_cast<GLuint>(program),
                                  static_cast<GLenum>(programInterface),
                                  static_cast<GLuint>(index),
                                  static_cast<GLsizei>(bufSize),
                                  detail::constAway(reinterpret_cast<GLsizei*>(length)),
                                  detail::constAway(reinterpret_cast<GLchar*>(name)));
}
void getProgramResource(uint32_t program,
                        ProgramInterface programInterface,
                        uint32_t index,
                        core::SizeType propCount,
                        const ProgramResourceProperty* props,
                        core::SizeType count,
                        core::SizeType* length,
                        int32_t* params)
{
  return glGetProgramResourceiv(static_cast<GLuint>(program),
                                static_cast<GLenum>(programInterface),
                                static_cast<GLuint>(index),
                                static_cast<GLsizei>(propCount),
                                detail::constAway(reinterpret_cast<const GLenum*>(props)),
                                static_cast<GLsizei>(count),
                                detail::constAway(reinterpret_cast<GLsizei*>(length)),
                                detail::constAway(reinterpret_cast<GLint*>(params)));
}
void invalidateBufferData(uint32_t buffer)
{
  return glInvalidateBufferData(static_cast<GLuint>(buffer));
}
void invalidateBufferSubData(uint32_t buffer, std::intptr_t offset, std::size_t length)
{
  return glInvalidateBufferSubData(
    static_cast<GLuint>(buffer), static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(length));
}
void invalidateFramebuffer(FramebufferTarget target,
                           core::SizeType numAttachments,
                           const InvalidateFramebufferAttachment* attachments)
{
  return glInvalidateFramebuffer(static_cast<GLenum>(target),
                                 static_cast<GLsizei>(numAttachments),
                                 detail::constAway(reinterpret_cast<const GLenum*>(attachments)));
}
void invalidateSubFramebuffer(FramebufferTarget target,
                              core::SizeType numAttachments,
                              const InvalidateFramebufferAttachment* attachments,
                              int32_t x,
                              int32_t y,
                              core::SizeType width,
                              core::SizeType height)
{
  return glInvalidateSubFramebuffer(static_cast<GLenum>(target),
                                    static_cast<GLsizei>(numAttachments),
                                    detail::constAway(reinterpret_cast<const GLenum*>(attachments)),
                                    static_cast<GLint>(x),
                                    static_cast<GLint>(y),
                                    static_cast<GLsizei>(width),
                                    static_cast<GLsizei>(height));
}
void invalidateTexImage(uint32_t texture, int32_t level)
{
  return glInvalidateTexImage(static_cast<GLuint>(texture), static_cast<GLint>(level));
}
void invalidateTexSubImage(uint32_t texture,
                           int32_t level,
                           int32_t xoffset,
                           int32_t yoffset,
                           int32_t zoffset,
                           core::SizeType width,
                           core::SizeType height,
                           core::SizeType depth)
{
  return glInvalidateTexSubImage(static_cast<GLuint>(texture),
                                 static_cast<GLint>(level),
                                 static_cast<GLint>(xoffset),
                                 static_cast<GLint>(yoffset),
                                 static_cast<GLint>(zoffset),
                                 static_cast<GLsizei>(width),
                                 static_cast<GLsizei>(height),
                                 static_cast<GLsizei>(depth));
}
void multiDrawArraysIndirect(PrimitiveType mode, const void* indirect, core::SizeType drawcount, core::SizeType stride)
{
  return glMultiDrawArraysIndirect(static_cast<GLenum>(mode),
                                   detail::constAway(indirect),
                                   static_cast<GLsizei>(drawcount),
                                   static_cast<GLsizei>(stride));
}
void multiDrawElementsIndirect(
  PrimitiveType mode, DrawElementsType type, const void* indirect, core::SizeType drawcount, core::SizeType stride)
{
  return glMultiDrawElementsIndirect(static_cast<GLenum>(mode),
                                     static_cast<GLenum>(type),
                                     detail::constAway(indirect),
                                     static_cast<GLsizei>(drawcount),
                                     static_cast<GLsizei>(stride));
}
void objectLabel(ObjectIdentifier identifier, uint32_t name, core::SizeType length, const char* label)
{
  return glObjectLabel(static_cast<GLenum>(identifier),
                       static_cast<GLuint>(name),
                       static_cast<GLsizei>(length),
                       detail::constAway(reinterpret_cast<const GLchar*>(label)));
}
void objectPtrLabel(const void* ptr, core::SizeType length, const char* label)
{
  return glObjectPtrLabel(
    detail::constAway(ptr), static_cast<GLsizei>(length), detail::constAway(reinterpret_cast<const GLchar*>(label)));
}
void popDebugGroup()
{
  return glPopDebugGroup();
}
void pushDebugGroup(DebugSource source, uint32_t id, core::SizeType length, const char* message)
{
  return glPushDebugGroup(static_cast<GLenum>(source),
                          static_cast<GLuint>(id),
                          static_cast<GLsizei>(length),
                          detail::constAway(reinterpret_cast<const GLchar*>(message)));
}
void shaderStorageBlockBinding(uint32_t program, uint32_t storageBlockIndex, uint32_t storageBlockBinding)
{
  return glShaderStorageBlockBinding(
    static_cast<GLuint>(program), static_cast<GLuint>(storageBlockIndex), static_cast<GLuint>(storageBlockBinding));
}
void texBufferRange(
  TextureTarget target, SizedInternalFormat internalformat, uint32_t buffer, std::intptr_t offset, std::size_t size)
{
  return glTexBufferRange(static_cast<GLenum>(target),
                          static_cast<GLenum>(internalformat),
                          static_cast<GLuint>(buffer),
                          static_cast<GLintptr>(offset),
                          static_cast<GLsizeiptr>(size));
}
void texStorage2DMultisample(TextureTarget target,
                             core::SizeType samples,
                             SizedInternalFormat internalformat,
                             core::SizeType width,
                             core::SizeType height,
                             bool fixedsamplelocations)
{
  return glTexStorage2DMultisample(static_cast<GLenum>(target),
                                   static_cast<GLsizei>(samples),
                                   static_cast<GLenum>(internalformat),
                                   static_cast<GLsizei>(width),
                                   static_cast<GLsizei>(height),
                                   static_cast<GLboolean>(fixedsamplelocations));
}
void texStorage3DMultisample(TextureTarget target,
                             core::SizeType samples,
                             SizedInternalFormat internalformat,
                             core::SizeType width,
                             core::SizeType height,
                             core::SizeType depth,
                             bool fixedsamplelocations)
{
  return glTexStorage3DMultisample(static_cast<GLenum>(target),
                                   static_cast<GLsizei>(samples),
                                   static_cast<GLenum>(internalformat),
                                   static_cast<GLsizei>(width),
                                   static_cast<GLsizei>(height),
                                   static_cast<GLsizei>(depth),
                                   static_cast<GLboolean>(fixedsamplelocations));
}
void textureView(uint32_t texture,
                 TextureTarget target,
                 uint32_t origtexture,
                 SizedInternalFormat internalformat,
                 uint32_t minlevel,
                 uint32_t numlevels,
                 uint32_t minlayer,
                 uint32_t numlayers)
{
  return glTextureView(static_cast<GLuint>(texture),
                       static_cast<GLenum>(target),
                       static_cast<GLuint>(origtexture),
                       static_cast<GLenum>(internalformat),
                       static_cast<GLuint>(minlevel),
                       static_cast<GLuint>(numlevels),
                       static_cast<GLuint>(minlayer),
                       static_cast<GLuint>(numlayers));
}
void vertexAttribBinding(uint32_t attribindex, uint32_t bindingindex)
{
  return glVertexAttribBinding(static_cast<GLuint>(attribindex), static_cast<GLuint>(bindingindex));
}
void vertexAttribFormat(
  uint32_t attribindex, int32_t size, VertexAttribType type, bool normalized, uint32_t relativeoffset)
{
  return glVertexAttribFormat(static_cast<GLuint>(attribindex),
                              static_cast<GLint>(size),
                              static_cast<GLenum>(type),
                              static_cast<GLboolean>(normalized),
                              static_cast<GLuint>(relativeoffset));
}
void vertexAttribIFormat(uint32_t attribindex, int32_t size, VertexAttribIType type, uint32_t relativeoffset)
{
  return glVertexAttribIFormat(static_cast<GLuint>(attribindex),
                               static_cast<GLint>(size),
                               static_cast<GLenum>(type),
                               static_cast<GLuint>(relativeoffset));
}
void vertexAttribLFormat(uint32_t attribindex, int32_t size, VertexAttribLType type, uint32_t relativeoffset)
{
  return glVertexAttribLFormat(static_cast<GLuint>(attribindex),
                               static_cast<GLint>(size),
                               static_cast<GLenum>(type),
                               static_cast<GLuint>(relativeoffset));
}
void vertexBindingDivisor(uint32_t bindingindex, uint32_t divisor)
{
  return glVertexBindingDivisor(static_cast<GLuint>(bindingindex), static_cast<GLuint>(divisor));
}
void bindBuffersBase(BufferTarget target, uint32_t first, core::SizeType count, const uint32_t* buffers)
{
  return glBindBuffersBase(static_cast<GLenum>(target),
                           static_cast<GLuint>(first),
                           static_cast<GLsizei>(count),
                           detail::constAway(reinterpret_cast<const GLuint*>(buffers)));
}
void bindBuffersRange(BufferTarget target,
                      uint32_t first,
                      core::SizeType count,
                      const uint32_t* buffers,
                      const std::intptr_t* offsets,
                      const std::size_t* sizes)
{
  return glBindBuffersRange(static_cast<GLenum>(target),
                            static_cast<GLuint>(first),
                            static_cast<GLsizei>(count),
                            detail::constAway(reinterpret_cast<const GLuint*>(buffers)),
                            detail::constAway(reinterpret_cast<const GLintptr*>(offsets)),
                            detail::constAway(reinterpret_cast<const GLsizeiptr*>(sizes)));
}
void bindImageTextures(uint32_t first, core::SizeType count, const uint32_t* textures)
{
  return glBindImageTextures(static_cast<GLuint>(first),
                             static_cast<GLsizei>(count),
                             detail::constAway(reinterpret_cast<const GLuint*>(textures)));
}
void bindSampler(uint32_t first, core::SizeType count, const uint32_t* samplers)
{
  return glBindSamplers(static_cast<GLuint>(first),
                        static_cast<GLsizei>(count),
                        detail::constAway(reinterpret_cast<const GLuint*>(samplers)));
}
void bindTextures(uint32_t first, core::SizeType count, const uint32_t* textures)
{
  return glBindTextures(static_cast<GLuint>(first),
                        static_cast<GLsizei>(count),
                        detail::constAway(reinterpret_cast<const GLuint*>(textures)));
}
void bindVertexBuffers(uint32_t first,
                       core::SizeType count,
                       const uint32_t* buffers,
                       const std::intptr_t* offsets,
                       const core::SizeType* strides)
{
  return glBindVertexBuffers(static_cast<GLuint>(first),
                             static_cast<GLsizei>(count),
                             detail::constAway(reinterpret_cast<const GLuint*>(buffers)),
                             detail::constAway(reinterpret_cast<const GLintptr*>(offsets)),
                             detail::constAway(reinterpret_cast<const GLsizei*>(strides)));
}
void bufferStorage(BufferStorageTarget target,
                   std::size_t size,
                   const void* data,
                   core::Bitfield<BufferStorageMask> flags)
{
  return glBufferStorage(
    static_cast<GLenum>(target), static_cast<GLsizeiptr>(size), detail::constAway(data), flags.value());
}
void clearTexImage(uint32_t texture, int32_t level, PixelFormat format, PixelType type, const void* data)
{
  return glClearTexImage(static_cast<GLuint>(texture),
                         static_cast<GLint>(level),
                         static_cast<GLenum>(format),
                         static_cast<GLenum>(type),
                         detail::constAway(data));
}
void clearTexSubImage(uint32_t texture,
                      int32_t level,
                      int32_t xoffset,
                      int32_t yoffset,
                      int32_t zoffset,
                      core::SizeType width,
                      core::SizeType height,
                      core::SizeType depth,
                      PixelFormat format,
                      PixelType type,
                      const void* data)
{
  return glClearTexSubImage(static_cast<GLuint>(texture),
                            static_cast<GLint>(level),
                            static_cast<GLint>(xoffset),
                            static_cast<GLint>(yoffset),
                            static_cast<GLint>(zoffset),
                            static_cast<GLsizei>(width),
                            static_cast<GLsizei>(height),
                            static_cast<GLsizei>(depth),
                            static_cast<GLenum>(format),
                            static_cast<GLenum>(type),
                            detail::constAway(data));
}
void bindTextureUnit(uint32_t unit, uint32_t texture)
{
  return glBindTextureUnit(static_cast<GLuint>(unit), static_cast<GLuint>(texture));
}
void blitNamedFramebuffer(uint32_t readFramebuffer,
                          uint32_t drawFramebuffer,
                          int32_t srcX0,
                          int32_t srcY0,
                          int32_t srcX1,
                          int32_t srcY1,
                          int32_t dstX0,
                          int32_t dstY0,
                          int32_t dstX1,
                          int32_t dstY1,
                          core::Bitfield<ClearBufferMask> mask,
                          BlitFramebufferFilter filter)
{
  return glBlitNamedFramebuffer(static_cast<GLuint>(readFramebuffer),
                                static_cast<GLuint>(drawFramebuffer),
                                static_cast<GLint>(srcX0),
                                static_cast<GLint>(srcY0),
                                static_cast<GLint>(srcX1),
                                static_cast<GLint>(srcY1),
                                static_cast<GLint>(dstX0),
                                static_cast<GLint>(dstY0),
                                static_cast<GLint>(dstX1),
                                static_cast<GLint>(dstY1),
                                mask.value(),
                                static_cast<GLenum>(filter));
}
FramebufferStatus checkNamedFramebufferStatus(uint32_t framebuffer, FramebufferTarget target)
{
  return static_cast<FramebufferStatus>(
    glCheckNamedFramebufferStatus(static_cast<GLuint>(framebuffer), static_cast<GLenum>(target)));
}
void clearNamedBufferData(
  uint32_t buffer, SizedInternalFormat internalformat, PixelFormat format, PixelType type, const void* data)
{
  return glClearNamedBufferData(static_cast<GLuint>(buffer),
                                static_cast<GLenum>(internalformat),
                                static_cast<GLenum>(format),
                                static_cast<GLenum>(type),
                                detail::constAway(data));
}
void clearNamedBufferSubData(uint32_t buffer,
                             SizedInternalFormat internalformat,
                             std::intptr_t offset,
                             std::size_t size,
                             PixelFormat format,
                             PixelType type,
                             const void* data)
{
  return glClearNamedBufferSubData(static_cast<GLuint>(buffer),
                                   static_cast<GLenum>(internalformat),
                                   static_cast<GLintptr>(offset),
                                   static_cast<GLsizeiptr>(size),
                                   static_cast<GLenum>(format),
                                   static_cast<GLenum>(type),
                                   detail::constAway(data));
}
void clearNamedFramebufferf(uint32_t framebuffer, Buffer buffer, int32_t drawbuffer, float depth, int32_t stencil)
{
  return glClearNamedFramebufferfi(static_cast<GLuint>(framebuffer),
                                   static_cast<GLenum>(buffer),
                                   static_cast<GLint>(drawbuffer),
                                   static_cast<GLfloat>(depth),
                                   static_cast<GLint>(stencil));
}
void clearNamedFramebuffer(uint32_t framebuffer, Buffer buffer, int32_t drawbuffer, const float* value)
{
  return glClearNamedFramebufferfv(static_cast<GLuint>(framebuffer),
                                   static_cast<GLenum>(buffer),
                                   static_cast<GLint>(drawbuffer),
                                   detail::constAway(reinterpret_cast<const GLfloat*>(value)));
}
void clearNamedFramebuffer(uint32_t framebuffer, Buffer buffer, int32_t drawbuffer, const int32_t* value)
{
  return glClearNamedFramebufferiv(static_cast<GLuint>(framebuffer),
                                   static_cast<GLenum>(buffer),
                                   static_cast<GLint>(drawbuffer),
                                   detail::constAway(reinterpret_cast<const GLint*>(value)));
}
void clearNamedFramebuffer(uint32_t framebuffer, Buffer buffer, int32_t drawbuffer, const uint32_t* value)
{
  return glClearNamedFramebufferuiv(static_cast<GLuint>(framebuffer),
                                    static_cast<GLenum>(buffer),
                                    static_cast<GLint>(drawbuffer),
                                    detail::constAway(reinterpret_cast<const GLuint*>(value)));
}
void clipControl(ClipControlOrigin origin, ClipControlDepth depth)
{
  return glClipControl(static_cast<GLenum>(origin), static_cast<GLenum>(depth));
}
void compressedTextureSubImage1D(uint32_t texture,
                                 int32_t level,
                                 int32_t xoffset,
                                 core::SizeType width,
                                 InternalFormat format,
                                 core::SizeType imageSize,
                                 const void* data)
{
  return glCompressedTextureSubImage1D(static_cast<GLuint>(texture),
                                       static_cast<GLint>(level),
                                       static_cast<GLint>(xoffset),
                                       static_cast<GLsizei>(width),
                                       static_cast<GLenum>(format),
                                       static_cast<GLsizei>(imageSize),
                                       detail::constAway(data));
}
void compressedTextureSubImage2D(uint32_t texture,
                                 int32_t level,
                                 int32_t xoffset,
                                 int32_t yoffset,
                                 core::SizeType width,
                                 core::SizeType height,
                                 InternalFormat format,
                                 core::SizeType imageSize,
                                 const void* data)
{
  return glCompressedTextureSubImage2D(static_cast<GLuint>(texture),
                                       static_cast<GLint>(level),
                                       static_cast<GLint>(xoffset),
                                       static_cast<GLint>(yoffset),
                                       static_cast<GLsizei>(width),
                                       static_cast<GLsizei>(height),
                                       static_cast<GLenum>(format),
                                       static_cast<GLsizei>(imageSize),
                                       detail::constAway(data));
}
void compressedTextureSubImage3D(uint32_t texture,
                                 int32_t level,
                                 int32_t xoffset,
                                 int32_t yoffset,
                                 int32_t zoffset,
                                 core::SizeType width,
                                 core::SizeType height,
                                 core::SizeType depth,
                                 InternalFormat format,
                                 core::SizeType imageSize,
                                 const void* data)
{
  return glCompressedTextureSubImage3D(static_cast<GLuint>(texture),
                                       static_cast<GLint>(level),
                                       static_cast<GLint>(xoffset),
                                       static_cast<GLint>(yoffset),
                                       static_cast<GLint>(zoffset),
                                       static_cast<GLsizei>(width),
                                       static_cast<GLsizei>(height),
                                       static_cast<GLsizei>(depth),
                                       static_cast<GLenum>(format),
                                       static_cast<GLsizei>(imageSize),
                                       detail::constAway(data));
}
void copyNamedBufferSubData(
  uint32_t readBuffer, uint32_t writeBuffer, std::intptr_t readOffset, std::intptr_t writeOffset, std::size_t size)
{
  return glCopyNamedBufferSubData(static_cast<GLuint>(readBuffer),
                                  static_cast<GLuint>(writeBuffer),
                                  static_cast<GLintptr>(readOffset),
                                  static_cast<GLintptr>(writeOffset),
                                  static_cast<GLsizeiptr>(size));
}
void copyTextureSubImage1D(uint32_t texture, int32_t level, int32_t xoffset, int32_t x, int32_t y, core::SizeType width)
{
  return glCopyTextureSubImage1D(static_cast<GLuint>(texture),
                                 static_cast<GLint>(level),
                                 static_cast<GLint>(xoffset),
                                 static_cast<GLint>(x),
                                 static_cast<GLint>(y),
                                 static_cast<GLsizei>(width));
}
void copyTextureSubImage2D(uint32_t texture,
                           int32_t level,
                           int32_t xoffset,
                           int32_t yoffset,
                           int32_t x,
                           int32_t y,
                           core::SizeType width,
                           core::SizeType height)
{
  return glCopyTextureSubImage2D(static_cast<GLuint>(texture),
                                 static_cast<GLint>(level),
                                 static_cast<GLint>(xoffset),
                                 static_cast<GLint>(yoffset),
                                 static_cast<GLint>(x),
                                 static_cast<GLint>(y),
                                 static_cast<GLsizei>(width),
                                 static_cast<GLsizei>(height));
}
void copyTextureSubImage3D(uint32_t texture,
                           int32_t level,
                           int32_t xoffset,
                           int32_t yoffset,
                           int32_t zoffset,
                           int32_t x,
                           int32_t y,
                           core::SizeType width,
                           core::SizeType height)
{
  return glCopyTextureSubImage3D(static_cast<GLuint>(texture),
                                 static_cast<GLint>(level),
                                 static_cast<GLint>(xoffset),
                                 static_cast<GLint>(yoffset),
                                 static_cast<GLint>(zoffset),
                                 static_cast<GLint>(x),
                                 static_cast<GLint>(y),
                                 static_cast<GLsizei>(width),
                                 static_cast<GLsizei>(height));
}
void createBuffers(core::SizeType n, uint32_t* buffers)
{
  return glCreateBuffers(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<GLuint*>(buffers)));
}
void createFramebuffers(core::SizeType n, uint32_t* framebuffers)
{
  return glCreateFramebuffers(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<GLuint*>(framebuffers)));
}
void createProgramPipeline(core::SizeType n, uint32_t* pipelines)
{
  return glCreateProgramPipelines(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<GLuint*>(pipelines)));
}
void createQueries(QueryTarget target, core::SizeType n, uint32_t* ids)
{
  return glCreateQueries(
    static_cast<GLenum>(target), static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<GLuint*>(ids)));
}
void createRenderbuffers(core::SizeType n, uint32_t* renderbuffers)
{
  return glCreateRenderbuffers(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<GLuint*>(renderbuffers)));
}
void createSampler(core::SizeType n, uint32_t* samplers)
{
  return glCreateSamplers(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<GLuint*>(samplers)));
}
void createTextures(TextureTarget target, core::SizeType n, uint32_t* textures)
{
  return glCreateTextures(
    static_cast<GLenum>(target), static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<GLuint*>(textures)));
}
void createTransformFeedback(core::SizeType n, uint32_t* ids)
{
  return glCreateTransformFeedbacks(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<GLuint*>(ids)));
}
void createVertexArrays(core::SizeType n, uint32_t* arrays)
{
  return glCreateVertexArrays(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<GLuint*>(arrays)));
}
void disableVertexArrayAttrib(uint32_t vaobj, uint32_t index)
{
  return glDisableVertexArrayAttrib(static_cast<GLuint>(vaobj), static_cast<GLuint>(index));
}
void enableVertexArrayAttrib(uint32_t vaobj, uint32_t index)
{
  return glEnableVertexArrayAttrib(static_cast<GLuint>(vaobj), static_cast<GLuint>(index));
}
void flushMappedNamedBufferRange(uint32_t buffer, std::intptr_t offset, std::size_t length)
{
  return glFlushMappedNamedBufferRange(
    static_cast<GLuint>(buffer), static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(length));
}
void generateTextureMipmap(uint32_t texture)
{
  return glGenerateTextureMipmap(static_cast<GLuint>(texture));
}
void getCompressedTextureImage(uint32_t texture, int32_t level, core::SizeType bufSize, void* pixels)
{
  return glGetCompressedTextureImage(
    static_cast<GLuint>(texture), static_cast<GLint>(level), static_cast<GLsizei>(bufSize), detail::constAway(pixels));
}
void getCompressedTextureSubImage(uint32_t texture,
                                  int32_t level,
                                  int32_t xoffset,
                                  int32_t yoffset,
                                  int32_t zoffset,
                                  core::SizeType width,
                                  core::SizeType height,
                                  core::SizeType depth,
                                  core::SizeType bufSize,
                                  void* pixels)
{
  return glGetCompressedTextureSubImage(static_cast<GLuint>(texture),
                                        static_cast<GLint>(level),
                                        static_cast<GLint>(xoffset),
                                        static_cast<GLint>(yoffset),
                                        static_cast<GLint>(zoffset),
                                        static_cast<GLsizei>(width),
                                        static_cast<GLsizei>(height),
                                        static_cast<GLsizei>(depth),
                                        static_cast<GLsizei>(bufSize),
                                        detail::constAway(pixels));
}
GraphicsResetStatus getGraphicsResetStatus()
{
  return static_cast<GraphicsResetStatus>(glGetGraphicsResetStatus());
}
void getNamedBufferParameter(uint32_t buffer, BufferPName pname, int64_t* params)
{
  return glGetNamedBufferParameteri64v(
    static_cast<GLuint>(buffer), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint64*>(params)));
}
void getNamedBufferParameter(uint32_t buffer, BufferPName pname, int32_t* params)
{
  return glGetNamedBufferParameteriv(
    static_cast<GLuint>(buffer), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint*>(params)));
}
void getNamedBufferPointer(uint32_t buffer, BufferPointerName pname, void** params)
{
  return glGetNamedBufferPointerv(static_cast<GLuint>(buffer), static_cast<GLenum>(pname), detail::constAway(params));
}
void getNamedBufferSubData(uint32_t buffer, std::intptr_t offset, std::size_t size, void* data)
{
  return glGetNamedBufferSubData(
    static_cast<GLuint>(buffer), static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), detail::constAway(data));
}
void getNamedFramebufferAttachmentParameter(uint32_t framebuffer,
                                            FramebufferAttachment attachment,
                                            FramebufferAttachmentParameterName pname,
                                            int32_t* params)
{
  return glGetNamedFramebufferAttachmentParameteriv(static_cast<GLuint>(framebuffer),
                                                    static_cast<GLenum>(attachment),
                                                    static_cast<GLenum>(pname),
                                                    detail::constAway(reinterpret_cast<GLint*>(params)));
}
void getNamedFramebufferParameter(uint32_t framebuffer, GetFramebufferParameter pname, int32_t* param)
{
  return glGetNamedFramebufferParameteriv(
    static_cast<GLuint>(framebuffer), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint*>(param)));
}
void getNamedRenderbufferParameter(uint32_t renderbuffer, RenderbufferParameterName pname, int32_t* params)
{
  return glGetNamedRenderbufferParameteriv(
    static_cast<GLuint>(renderbuffer), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint*>(params)));
}
void getQueryBufferObjecti64v(uint32_t id, uint32_t buffer, QueryObjectParameterName pname, std::intptr_t offset)
{
  return glGetQueryBufferObjecti64v(
    static_cast<GLuint>(id), static_cast<GLuint>(buffer), static_cast<GLenum>(pname), static_cast<GLintptr>(offset));
}
void getQueryBufferObjectiv(uint32_t id, uint32_t buffer, QueryObjectParameterName pname, std::intptr_t offset)
{
  return glGetQueryBufferObjectiv(
    static_cast<GLuint>(id), static_cast<GLuint>(buffer), static_cast<GLenum>(pname), static_cast<GLintptr>(offset));
}
void getQueryBufferObjectui64v(uint32_t id, uint32_t buffer, QueryObjectParameterName pname, std::intptr_t offset)
{
  return glGetQueryBufferObjectui64v(
    static_cast<GLuint>(id), static_cast<GLuint>(buffer), static_cast<GLenum>(pname), static_cast<GLintptr>(offset));
}
void getQueryBufferObjectuiv(uint32_t id, uint32_t buffer, QueryObjectParameterName pname, std::intptr_t offset)
{
  return glGetQueryBufferObjectuiv(
    static_cast<GLuint>(id), static_cast<GLuint>(buffer), static_cast<GLenum>(pname), static_cast<GLintptr>(offset));
}
void getTextureImage(
  uint32_t texture, int32_t level, PixelFormat format, PixelType type, core::SizeType bufSize, void* pixels)
{
  return glGetTextureImage(static_cast<GLuint>(texture),
                           static_cast<GLint>(level),
                           static_cast<GLenum>(format),
                           static_cast<GLenum>(type),
                           static_cast<GLsizei>(bufSize),
                           detail::constAway(pixels));
}
void getTextureLevelParameter(uint32_t texture, int32_t level, GetTextureParameter pname, float* params)
{
  return glGetTextureLevelParameterfv(static_cast<GLuint>(texture),
                                      static_cast<GLint>(level),
                                      static_cast<GLenum>(pname),
                                      detail::constAway(reinterpret_cast<GLfloat*>(params)));
}
void getTextureLevelParameter(uint32_t texture, int32_t level, GetTextureParameter pname, int32_t* params)
{
  return glGetTextureLevelParameteriv(static_cast<GLuint>(texture),
                                      static_cast<GLint>(level),
                                      static_cast<GLenum>(pname),
                                      detail::constAway(reinterpret_cast<GLint*>(params)));
}
void getTextureParameterI(uint32_t texture, GetTextureParameter pname, int32_t* params)
{
  return glGetTextureParameterIiv(
    static_cast<GLuint>(texture), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint*>(params)));
}
void getTextureParameterI(uint32_t texture, GetTextureParameter pname, uint32_t* params)
{
  return glGetTextureParameterIuiv(
    static_cast<GLuint>(texture), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLuint*>(params)));
}
void getTextureParameter(uint32_t texture, GetTextureParameter pname, float* params)
{
  return glGetTextureParameterfv(
    static_cast<GLuint>(texture), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLfloat*>(params)));
}
void getTextureParameter(uint32_t texture, GetTextureParameter pname, int32_t* params)
{
  return glGetTextureParameteriv(
    static_cast<GLuint>(texture), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint*>(params)));
}
void getTextureSubImage(uint32_t texture,
                        int32_t level,
                        int32_t xoffset,
                        int32_t yoffset,
                        int32_t zoffset,
                        core::SizeType width,
                        core::SizeType height,
                        core::SizeType depth,
                        PixelFormat format,
                        PixelType type,
                        core::SizeType bufSize,
                        void* pixels)
{
  return glGetTextureSubImage(static_cast<GLuint>(texture),
                              static_cast<GLint>(level),
                              static_cast<GLint>(xoffset),
                              static_cast<GLint>(yoffset),
                              static_cast<GLint>(zoffset),
                              static_cast<GLsizei>(width),
                              static_cast<GLsizei>(height),
                              static_cast<GLsizei>(depth),
                              static_cast<GLenum>(format),
                              static_cast<GLenum>(type),
                              static_cast<GLsizei>(bufSize),
                              detail::constAway(pixels));
}
void getTransformFeedbackI(uint32_t xfb, TransformFeedbackPName pname, uint32_t index, int64_t* param)
{
  return glGetTransformFeedbacki64_v(static_cast<GLuint>(xfb),
                                     static_cast<GLenum>(pname),
                                     static_cast<GLuint>(index),
                                     detail::constAway(reinterpret_cast<GLint64*>(param)));
}
void getTransformFeedbackI(uint32_t xfb, TransformFeedbackPName pname, uint32_t index, int32_t* param)
{
  return glGetTransformFeedbacki_v(static_cast<GLuint>(xfb),
                                   static_cast<GLenum>(pname),
                                   static_cast<GLuint>(index),
                                   detail::constAway(reinterpret_cast<GLint*>(param)));
}
void getTransformFeedback(uint32_t xfb, TransformFeedbackPName pname, int32_t* param)
{
  return glGetTransformFeedbackiv(
    static_cast<GLuint>(xfb), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint*>(param)));
}
void getVertexArrayIndexed64(uint32_t vaobj, uint32_t index, VertexArrayPName pname, int64_t* param)
{
  return glGetVertexArrayIndexed64iv(static_cast<GLuint>(vaobj),
                                     static_cast<GLuint>(index),
                                     static_cast<GLenum>(pname),
                                     detail::constAway(reinterpret_cast<GLint64*>(param)));
}
void getVertexArrayIndexed(uint32_t vaobj, uint32_t index, VertexArrayPName pname, int32_t* param)
{
  return glGetVertexArrayIndexediv(static_cast<GLuint>(vaobj),
                                   static_cast<GLuint>(index),
                                   static_cast<GLenum>(pname),
                                   detail::constAway(reinterpret_cast<GLint*>(param)));
}
void getVertexArray(uint32_t vaobj, VertexArrayPName pname, int32_t* param)
{
  return glGetVertexArrayiv(
    static_cast<GLuint>(vaobj), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint*>(param)));
}
void getnCompressedTexImage(TextureTarget target, int32_t lod, core::SizeType bufSize, void* pixels)
{
  return glGetnCompressedTexImage(
    static_cast<GLenum>(target), static_cast<GLint>(lod), static_cast<GLsizei>(bufSize), detail::constAway(pixels));
}
void getnTexImage(
  TextureTarget target, int32_t level, PixelFormat format, PixelType type, core::SizeType bufSize, void* pixels)
{
  return glGetnTexImage(static_cast<GLenum>(target),
                        static_cast<GLint>(level),
                        static_cast<GLenum>(format),
                        static_cast<GLenum>(type),
                        static_cast<GLsizei>(bufSize),
                        detail::constAway(pixels));
}
void getnUniform(uint32_t program, int32_t location, core::SizeType bufSize, double* params)
{
  return glGetnUniformdv(static_cast<GLuint>(program),
                         static_cast<GLint>(location),
                         static_cast<GLsizei>(bufSize),
                         detail::constAway(reinterpret_cast<GLdouble*>(params)));
}
void getnUniform(uint32_t program, int32_t location, core::SizeType bufSize, float* params)
{
  return glGetnUniformfv(static_cast<GLuint>(program),
                         static_cast<GLint>(location),
                         static_cast<GLsizei>(bufSize),
                         detail::constAway(reinterpret_cast<GLfloat*>(params)));
}
void getnUniform(uint32_t program, int32_t location, core::SizeType bufSize, int32_t* params)
{
  return glGetnUniformiv(static_cast<GLuint>(program),
                         static_cast<GLint>(location),
                         static_cast<GLsizei>(bufSize),
                         detail::constAway(reinterpret_cast<GLint*>(params)));
}
void getnUniform(uint32_t program, int32_t location, core::SizeType bufSize, uint32_t* params)
{
  return glGetnUniformuiv(static_cast<GLuint>(program),
                          static_cast<GLint>(location),
                          static_cast<GLsizei>(bufSize),
                          detail::constAway(reinterpret_cast<GLuint*>(params)));
}
void invalidateNamedFramebufferData(uint32_t framebuffer,
                                    core::SizeType numAttachments,
                                    const FramebufferAttachment* attachments)
{
  return glInvalidateNamedFramebufferData(static_cast<GLuint>(framebuffer),
                                          static_cast<GLsizei>(numAttachments),
                                          detail::constAway(reinterpret_cast<const GLenum*>(attachments)));
}
void invalidateNamedFramebufferSubData(uint32_t framebuffer,
                                       core::SizeType numAttachments,
                                       const FramebufferAttachment* attachments,
                                       int32_t x,
                                       int32_t y,
                                       core::SizeType width,
                                       core::SizeType height)
{
  return glInvalidateNamedFramebufferSubData(static_cast<GLuint>(framebuffer),
                                             static_cast<GLsizei>(numAttachments),
                                             detail::constAway(reinterpret_cast<const GLenum*>(attachments)),
                                             static_cast<GLint>(x),
                                             static_cast<GLint>(y),
                                             static_cast<GLsizei>(width),
                                             static_cast<GLsizei>(height));
}
void* mapNamedBuffer(uint32_t buffer, BufferAccess access)
{
  return glMapNamedBuffer(static_cast<GLuint>(buffer), static_cast<GLenum>(access));
}
void* mapNamedBufferRange(uint32_t buffer,
                          std::intptr_t offset,
                          std::size_t length,
                          core::Bitfield<MapBufferAccessMask> access)
{
  return glMapNamedBufferRange(
    static_cast<GLuint>(buffer), static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(length), access.value());
}
void memoryBarrierByRegion(core::Bitfield<MemoryBarrierMask> barriers)
{
  return glMemoryBarrierByRegion(barriers.value());
}
void namedBufferData(uint32_t buffer, std::size_t size, const void* data, BufferUsage usage)
{
  return glNamedBufferData(
    static_cast<GLuint>(buffer), static_cast<GLsizeiptr>(size), detail::constAway(data), static_cast<GLenum>(usage));
}
void namedBufferStorage(uint32_t buffer, std::size_t size, const void* data, core::Bitfield<BufferStorageMask> flags)
{
  return glNamedBufferStorage(
    static_cast<GLuint>(buffer), static_cast<GLsizeiptr>(size), detail::constAway(data), flags.value());
}
void namedBufferSubData(uint32_t buffer, std::intptr_t offset, std::size_t size, const void* data)
{
  return glNamedBufferSubData(
    static_cast<GLuint>(buffer), static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), detail::constAway(data));
}
void namedFramebufferDrawBuffer(uint32_t framebuffer, ColorBuffer buf)
{
  return glNamedFramebufferDrawBuffer(static_cast<GLuint>(framebuffer), static_cast<GLenum>(buf));
}
void namedFramebufferDrawBuffers(uint32_t framebuffer, core::SizeType n, const ColorBuffer* bufs)
{
  return glNamedFramebufferDrawBuffers(static_cast<GLuint>(framebuffer),
                                       static_cast<GLsizei>(n),
                                       detail::constAway(reinterpret_cast<const GLenum*>(bufs)));
}
void namedFramebufferParameter(uint32_t framebuffer, FramebufferParameterName pname, int32_t param)
{
  return glNamedFramebufferParameteri(
    static_cast<GLuint>(framebuffer), static_cast<GLenum>(pname), static_cast<GLint>(param));
}
void namedFramebufferReadBuffer(uint32_t framebuffer, ColorBuffer src)
{
  return glNamedFramebufferReadBuffer(static_cast<GLuint>(framebuffer), static_cast<GLenum>(src));
}
void namedFramebufferRenderbuffer(uint32_t framebuffer,
                                  FramebufferAttachment attachment,
                                  RenderbufferTarget renderbuffertarget,
                                  uint32_t renderbuffer)
{
  return glNamedFramebufferRenderbuffer(static_cast<GLuint>(framebuffer),
                                        static_cast<GLenum>(attachment),
                                        static_cast<GLenum>(renderbuffertarget),
                                        static_cast<GLuint>(renderbuffer));
}
void namedFramebufferTexture(uint32_t framebuffer, FramebufferAttachment attachment, uint32_t texture, int32_t level)
{
  return glNamedFramebufferTexture(static_cast<GLuint>(framebuffer),
                                   static_cast<GLenum>(attachment),
                                   static_cast<GLuint>(texture),
                                   static_cast<GLint>(level));
}
void namedFramebufferTextureLayer(
  uint32_t framebuffer, FramebufferAttachment attachment, uint32_t texture, int32_t level, int32_t layer)
{
  return glNamedFramebufferTextureLayer(static_cast<GLuint>(framebuffer),
                                        static_cast<GLenum>(attachment),
                                        static_cast<GLuint>(texture),
                                        static_cast<GLint>(level),
                                        static_cast<GLint>(layer));
}
void namedRenderbufferStorage(uint32_t renderbuffer,
                              InternalFormat internalformat,
                              core::SizeType width,
                              core::SizeType height)
{
  return glNamedRenderbufferStorage(static_cast<GLuint>(renderbuffer),
                                    static_cast<GLenum>(internalformat),
                                    static_cast<GLsizei>(width),
                                    static_cast<GLsizei>(height));
}
void namedRenderbufferStorageMultisample(uint32_t renderbuffer,
                                         core::SizeType samples,
                                         InternalFormat internalformat,
                                         core::SizeType width,
                                         core::SizeType height)
{
  return glNamedRenderbufferStorageMultisample(static_cast<GLuint>(renderbuffer),
                                               static_cast<GLsizei>(samples),
                                               static_cast<GLenum>(internalformat),
                                               static_cast<GLsizei>(width),
                                               static_cast<GLsizei>(height));
}
void readnPixel(int32_t x,
                int32_t y,
                core::SizeType width,
                core::SizeType height,
                PixelFormat format,
                PixelType type,
                core::SizeType bufSize,
                void* data)
{
  return glReadnPixels(static_cast<GLint>(x),
                       static_cast<GLint>(y),
                       static_cast<GLsizei>(width),
                       static_cast<GLsizei>(height),
                       static_cast<GLenum>(format),
                       static_cast<GLenum>(type),
                       static_cast<GLsizei>(bufSize),
                       detail::constAway(data));
}
void textureBarrier()
{
  return glTextureBarrier();
}
void textureBuffer(uint32_t texture, SizedInternalFormat internalformat, uint32_t buffer)
{
  return glTextureBuffer(
    static_cast<GLuint>(texture), static_cast<GLenum>(internalformat), static_cast<GLuint>(buffer));
}
void textureBufferRange(
  uint32_t texture, SizedInternalFormat internalformat, uint32_t buffer, std::intptr_t offset, std::size_t size)
{
  return glTextureBufferRange(static_cast<GLuint>(texture),
                              static_cast<GLenum>(internalformat),
                              static_cast<GLuint>(buffer),
                              static_cast<GLintptr>(offset),
                              static_cast<GLsizeiptr>(size));
}
void textureParameterI(uint32_t texture, TextureParameterName pname, const int32_t* params)
{
  return glTextureParameterIiv(static_cast<GLuint>(texture),
                               static_cast<GLenum>(pname),
                               detail::constAway(reinterpret_cast<const GLint*>(params)));
}
void textureParameterI(uint32_t texture, TextureParameterName pname, const uint32_t* params)
{
  return glTextureParameterIuiv(static_cast<GLuint>(texture),
                                static_cast<GLenum>(pname),
                                detail::constAway(reinterpret_cast<const GLuint*>(params)));
}
void textureParameter(uint32_t texture, TextureParameterName pname, float param)
{
  return glTextureParameterf(static_cast<GLuint>(texture), static_cast<GLenum>(pname), static_cast<GLfloat>(param));
}
void textureParameter(uint32_t texture, TextureParameterName pname, const float* param)
{
  return glTextureParameterfv(static_cast<GLuint>(texture),
                              static_cast<GLenum>(pname),
                              detail::constAway(reinterpret_cast<const GLfloat*>(param)));
}
void textureParameter(uint32_t texture, TextureParameterName pname, int32_t param)
{
  return glTextureParameteri(static_cast<GLuint>(texture), static_cast<GLenum>(pname), static_cast<GLint>(param));
}
void textureParameter(uint32_t texture, TextureParameterName pname, const int32_t* param)
{
  return glTextureParameteriv(
    static_cast<GLuint>(texture), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<const GLint*>(param)));
}
void textureStorage1D(uint32_t texture, core::SizeType levels, SizedInternalFormat internalformat, core::SizeType width)
{
  return glTextureStorage1D(static_cast<GLuint>(texture),
                            static_cast<GLsizei>(levels),
                            static_cast<GLenum>(internalformat),
                            static_cast<GLsizei>(width));
}
void textureStorage2D(uint32_t texture,
                      core::SizeType levels,
                      SizedInternalFormat internalformat,
                      core::SizeType width,
                      core::SizeType height)
{
  return glTextureStorage2D(static_cast<GLuint>(texture),
                            static_cast<GLsizei>(levels),
                            static_cast<GLenum>(internalformat),
                            static_cast<GLsizei>(width),
                            static_cast<GLsizei>(height));
}
void textureStorage2DMultisample(uint32_t texture,
                                 core::SizeType samples,
                                 SizedInternalFormat internalformat,
                                 core::SizeType width,
                                 core::SizeType height,
                                 bool fixedsamplelocations)
{
  return glTextureStorage2DMultisample(static_cast<GLuint>(texture),
                                       static_cast<GLsizei>(samples),
                                       static_cast<GLenum>(internalformat),
                                       static_cast<GLsizei>(width),
                                       static_cast<GLsizei>(height),
                                       static_cast<GLboolean>(fixedsamplelocations));
}
void textureStorage3D(uint32_t texture,
                      core::SizeType levels,
                      SizedInternalFormat internalformat,
                      core::SizeType width,
                      core::SizeType height,
                      core::SizeType depth)
{
  return glTextureStorage3D(static_cast<GLuint>(texture),
                            static_cast<GLsizei>(levels),
                            static_cast<GLenum>(internalformat),
                            static_cast<GLsizei>(width),
                            static_cast<GLsizei>(height),
                            static_cast<GLsizei>(depth));
}
void textureStorage3DMultisample(uint32_t texture,
                                 core::SizeType samples,
                                 SizedInternalFormat internalformat,
                                 core::SizeType width,
                                 core::SizeType height,
                                 core::SizeType depth,
                                 bool fixedsamplelocations)
{
  return glTextureStorage3DMultisample(static_cast<GLuint>(texture),
                                       static_cast<GLsizei>(samples),
                                       static_cast<GLenum>(internalformat),
                                       static_cast<GLsizei>(width),
                                       static_cast<GLsizei>(height),
                                       static_cast<GLsizei>(depth),
                                       static_cast<GLboolean>(fixedsamplelocations));
}
void textureSubImage1D(uint32_t texture,
                       int32_t level,
                       int32_t xoffset,
                       core::SizeType width,
                       PixelFormat format,
                       PixelType type,
                       const void* pixels)
{
  return glTextureSubImage1D(static_cast<GLuint>(texture),
                             static_cast<GLint>(level),
                             static_cast<GLint>(xoffset),
                             static_cast<GLsizei>(width),
                             static_cast<GLenum>(format),
                             static_cast<GLenum>(type),
                             detail::constAway(pixels));
}
void textureSubImage2D(uint32_t texture,
                       int32_t level,
                       int32_t xoffset,
                       int32_t yoffset,
                       core::SizeType width,
                       core::SizeType height,
                       PixelFormat format,
                       PixelType type,
                       const void* pixels)
{
  return glTextureSubImage2D(static_cast<GLuint>(texture),
                             static_cast<GLint>(level),
                             static_cast<GLint>(xoffset),
                             static_cast<GLint>(yoffset),
                             static_cast<GLsizei>(width),
                             static_cast<GLsizei>(height),
                             static_cast<GLenum>(format),
                             static_cast<GLenum>(type),
                             detail::constAway(pixels));
}
void textureSubImage3D(uint32_t texture,
                       int32_t level,
                       int32_t xoffset,
                       int32_t yoffset,
                       int32_t zoffset,
                       core::SizeType width,
                       core::SizeType height,
                       core::SizeType depth,
                       PixelFormat format,
                       PixelType type,
                       const void* pixels)
{
  return glTextureSubImage3D(static_cast<GLuint>(texture),
                             static_cast<GLint>(level),
                             static_cast<GLint>(xoffset),
                             static_cast<GLint>(yoffset),
                             static_cast<GLint>(zoffset),
                             static_cast<GLsizei>(width),
                             static_cast<GLsizei>(height),
                             static_cast<GLsizei>(depth),
                             static_cast<GLenum>(format),
                             static_cast<GLenum>(type),
                             detail::constAway(pixels));
}
void transformFeedbackBufferBase(uint32_t xfb, uint32_t index, uint32_t buffer)
{
  return glTransformFeedbackBufferBase(
    static_cast<GLuint>(xfb), static_cast<GLuint>(index), static_cast<GLuint>(buffer));
}
void transformFeedbackBufferRange(uint32_t xfb, uint32_t index, uint32_t buffer, std::intptr_t offset, std::size_t size)
{
  return glTransformFeedbackBufferRange(static_cast<GLuint>(xfb),
                                        static_cast<GLuint>(index),
                                        static_cast<GLuint>(buffer),
                                        static_cast<GLintptr>(offset),
                                        static_cast<GLsizeiptr>(size));
}
bool unmapNamedBuffer(uint32_t buffer)
{
  return static_cast<bool>(glUnmapNamedBuffer(static_cast<GLuint>(buffer)));
}
void vertexArrayAttribBinding(uint32_t vaobj, uint32_t attribindex, uint32_t bindingindex)
{
  return glVertexArrayAttribBinding(
    static_cast<GLuint>(vaobj), static_cast<GLuint>(attribindex), static_cast<GLuint>(bindingindex));
}
void vertexArrayAttribFormat(
  uint32_t vaobj, uint32_t attribindex, int32_t size, VertexAttribType type, bool normalized, uint32_t relativeoffset)
{
  return glVertexArrayAttribFormat(static_cast<GLuint>(vaobj),
                                   static_cast<GLuint>(attribindex),
                                   static_cast<GLint>(size),
                                   static_cast<GLenum>(type),
                                   static_cast<GLboolean>(normalized),
                                   static_cast<GLuint>(relativeoffset));
}
void vertexArrayAttribIFormat(
  uint32_t vaobj, uint32_t attribindex, int32_t size, VertexAttribIType type, uint32_t relativeoffset)
{
  return glVertexArrayAttribIFormat(static_cast<GLuint>(vaobj),
                                    static_cast<GLuint>(attribindex),
                                    static_cast<GLint>(size),
                                    static_cast<GLenum>(type),
                                    static_cast<GLuint>(relativeoffset));
}
void vertexArrayAttribLFormat(
  uint32_t vaobj, uint32_t attribindex, int32_t size, VertexAttribLType type, uint32_t relativeoffset)
{
  return glVertexArrayAttribLFormat(static_cast<GLuint>(vaobj),
                                    static_cast<GLuint>(attribindex),
                                    static_cast<GLint>(size),
                                    static_cast<GLenum>(type),
                                    static_cast<GLuint>(relativeoffset));
}
void vertexArrayBindingDivisor(uint32_t vaobj, uint32_t bindingindex, uint32_t divisor)
{
  return glVertexArrayBindingDivisor(
    static_cast<GLuint>(vaobj), static_cast<GLuint>(bindingindex), static_cast<GLuint>(divisor));
}
void vertexArrayElementBuffer(uint32_t vaobj, uint32_t buffer)
{
  return glVertexArrayElementBuffer(static_cast<GLuint>(vaobj), static_cast<GLuint>(buffer));
}
void vertexArrayVertexBuffer(
  uint32_t vaobj, uint32_t bindingindex, uint32_t buffer, std::intptr_t offset, core::SizeType stride)
{
  return glVertexArrayVertexBuffer(static_cast<GLuint>(vaobj),
                                   static_cast<GLuint>(bindingindex),
                                   static_cast<GLuint>(buffer),
                                   static_cast<GLintptr>(offset),
                                   static_cast<GLsizei>(stride));
}
void vertexArrayVertexBuffers(uint32_t vaobj,
                              uint32_t first,
                              core::SizeType count,
                              const uint32_t* buffers,
                              const std::intptr_t* offsets,
                              const core::SizeType* strides)
{
  return glVertexArrayVertexBuffers(static_cast<GLuint>(vaobj),
                                    static_cast<GLuint>(first),
                                    static_cast<GLsizei>(count),
                                    detail::constAway(reinterpret_cast<const GLuint*>(buffers)),
                                    detail::constAway(reinterpret_cast<const GLintptr*>(offsets)),
                                    detail::constAway(reinterpret_cast<const GLsizei*>(strides)));
}
} // namespace gl::api
