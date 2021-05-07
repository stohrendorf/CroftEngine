#include "glsc2.hpp"
#include "glsc2_api_provider.hpp"
namespace glsc2::api
{
void activeTexture(TextureUnit texture)
{
  return glActiveTexture(static_cast<GLenum>(texture));
}
void bindBuffer(BufferTargetARB target, uint32_t buffer)
{
  return glBindBuffer(static_cast<GLenum>(target), static_cast<GLuint>(buffer));
}
void bindFramebuffer(FramebufferTarget target, uint32_t framebuffer)
{
  return glBindFramebuffer(static_cast<GLenum>(target), static_cast<GLuint>(framebuffer));
}
void bindRenderbuffer(RenderbufferTarget target, uint32_t renderbuffer)
{
  return glBindRenderbuffer(static_cast<GLenum>(target), static_cast<GLuint>(renderbuffer));
}
void bindTexture(TextureTarget target, uint32_t texture)
{
  return glBindTexture(static_cast<GLenum>(target), static_cast<GLuint>(texture));
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
void blendEquationSeparate(BlendEquationModeEXT modeRGB, BlendEquationModeEXT modeAlpha)
{
  return glBlendEquationSeparate(static_cast<GLenum>(modeRGB), static_cast<GLenum>(modeAlpha));
}
void blendFunc(BlendingFactor sfactor, BlendingFactor dfactor)
{
  return glBlendFunc(static_cast<GLenum>(sfactor), static_cast<GLenum>(dfactor));
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
void bufferData(BufferTargetARB target, std::size_t size, const void* data, BufferUsageARB usage)
{
  return glBufferData(
    static_cast<GLenum>(target), static_cast<GLsizeiptr>(size), detail::constAway(data), static_cast<GLenum>(usage));
}
void bufferSubData(BufferTargetARB target, std::intptr_t offset, std::size_t size, const void* data)
{
  return glBufferSubData(
    static_cast<GLenum>(target), static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), detail::constAway(data));
}
FramebufferStatus checkFramebufferStatus(FramebufferTarget target)
{
  return static_cast<FramebufferStatus>(glCheckFramebufferStatus(static_cast<GLenum>(target)));
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
void clearDepth(float d)
{
  return glClearDepthf(static_cast<GLfloat>(d));
}
void clearStencil(int32_t s)
{
  return glClearStencil(static_cast<GLint>(s));
}
void colorMask(bool red, bool green, bool blue, bool alpha)
{
  return glColorMask(static_cast<GLboolean>(red),
                     static_cast<GLboolean>(green),
                     static_cast<GLboolean>(blue),
                     static_cast<GLboolean>(alpha));
}
void compressedTexSubImage2D(TextureTarget target,
                             int32_t level,
                             int32_t xoffset,
                             int32_t yoffset,
                             core::SizeType width,
                             core::SizeType height,
                             PixelFormat format,
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
uint32_t createProgram()
{
  return static_cast<uint32_t>(glCreateProgram());
}
void cullFace(CullFaceMode mode)
{
  return glCullFace(static_cast<GLenum>(mode));
}
void depthFunc(DepthFunction func)
{
  return glDepthFunc(static_cast<GLenum>(func));
}
void depthMask(bool flag)
{
  return glDepthMask(static_cast<GLboolean>(flag));
}
void depthRange(float n, float f)
{
  return glDepthRangef(static_cast<GLfloat>(n), static_cast<GLfloat>(f));
}
void disable(EnableCap cap)
{
  return glDisable(static_cast<GLenum>(cap));
}
void disableVertexAttribArray(uint32_t index)
{
  return glDisableVertexAttribArray(static_cast<GLuint>(index));
}
void drawArrays(PrimitiveType mode, int32_t first, core::SizeType count)
{
  return glDrawArrays(static_cast<GLenum>(mode), static_cast<GLint>(first), static_cast<GLsizei>(count));
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
void enable(EnableCap cap)
{
  return glEnable(static_cast<GLenum>(cap));
}
void enableVertexAttribArray(uint32_t index)
{
  return glEnableVertexAttribArray(static_cast<GLuint>(index));
}
void finish()
{
  return glFinish();
}
void flush()
{
  return glFlush();
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
void framebufferTexture2D(
  FramebufferTarget target, FramebufferAttachment attachment, TextureTarget textarget, uint32_t texture, int32_t level)
{
  return glFramebufferTexture2D(static_cast<GLenum>(target),
                                static_cast<GLenum>(attachment),
                                static_cast<GLenum>(textarget),
                                static_cast<GLuint>(texture),
                                static_cast<GLint>(level));
}
void frontFace(FrontFaceDirection mode)
{
  return glFrontFace(static_cast<GLenum>(mode));
}
void genBuffers(core::SizeType n, uint32_t* buffers)
{
  return glGenBuffers(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<GLuint*>(buffers)));
}
void genFramebuffers(core::SizeType n, uint32_t* framebuffers)
{
  return glGenFramebuffers(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<GLuint*>(framebuffers)));
}
void genRenderbuffers(core::SizeType n, uint32_t* renderbuffers)
{
  return glGenRenderbuffers(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<GLuint*>(renderbuffers)));
}
void genTextures(core::SizeType n, uint32_t* textures)
{
  return glGenTextures(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<GLuint*>(textures)));
}
void generateMipmap(TextureTarget target)
{
  return glGenerateMipmap(static_cast<GLenum>(target));
}
int32_t getAttribLocation(uint32_t program, const char* name)
{
  return static_cast<int32_t>(
    glGetAttribLocation(static_cast<GLuint>(program), detail::constAway(reinterpret_cast<const GLchar*>(name))));
}
void getBooleanv(GetPName pname, bool* data)
{
  return glGetBooleanv(static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLboolean*>(data)));
}
void getBufferParameter(BufferTargetARB target, BufferPNameARB pname, int32_t* params)
{
  return glGetBufferParameteriv(
    static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint*>(params)));
}
ErrorCode getError()
{
  return static_cast<ErrorCode>(glGetError());
}
void getFloatv(GetPName pname, float* data)
{
  return glGetFloatv(static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLfloat*>(data)));
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
GraphicsResetStatus getGraphicsResetStatus()
{
  return static_cast<GraphicsResetStatus>(glGetGraphicsResetStatus());
}
void getIntegerv(GetPName pname, int32_t* data)
{
  return glGetIntegerv(static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint*>(data)));
}
void getProgram(uint32_t program, ProgramPropertyARB pname, int32_t* params)
{
  return glGetProgramiv(
    static_cast<GLuint>(program), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint*>(params)));
}
void getRenderbufferParameter(RenderbufferTarget target, RenderbufferParameterName pname, int32_t* params)
{
  return glGetRenderbufferParameteriv(
    static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint*>(params)));
}
const uint8_t* getString(StringName name)
{
  return static_cast<const uint8_t*>(glGetString(static_cast<GLenum>(name)));
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
int32_t getUniformLocation(uint32_t program, const char* name)
{
  return static_cast<int32_t>(
    glGetUniformLocation(static_cast<GLuint>(program), detail::constAway(reinterpret_cast<const GLchar*>(name))));
}
void getVertexAttribPointerv(uint32_t index, VertexAttribPointerPropertyARB pname, void** pointer)
{
  return glGetVertexAttribPointerv(static_cast<GLuint>(index), static_cast<GLenum>(pname), detail::constAway(pointer));
}
void getVertexAttrib(uint32_t index, VertexAttribPropertyARB pname, float* params)
{
  return glGetVertexAttribfv(
    static_cast<GLuint>(index), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLfloat*>(params)));
}
void getVertexAttrib(uint32_t index, VertexAttribPropertyARB pname, int32_t* params)
{
  return glGetVertexAttribiv(
    static_cast<GLuint>(index), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint*>(params)));
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
void hint(HintTarget target, HintMode mode)
{
  return glHint(static_cast<GLenum>(target), static_cast<GLenum>(mode));
}
bool isEnable(EnableCap cap)
{
  return static_cast<bool>(glIsEnabled(static_cast<GLenum>(cap)));
}
void lineWidth(float width)
{
  return glLineWidth(static_cast<GLfloat>(width));
}
void pixelStore(PixelStoreParameter pname, int32_t param)
{
  return glPixelStorei(static_cast<GLenum>(pname), static_cast<GLint>(param));
}
void polygonOffset(float factor, float units)
{
  return glPolygonOffset(static_cast<GLfloat>(factor), static_cast<GLfloat>(units));
}
void programBinary(uint32_t program, core::EnumType binaryFormat, const void* binary, core::SizeType length)
{
  return glProgramBinary(static_cast<GLuint>(program),
                         static_cast<GLenum>(binaryFormat),
                         detail::constAway(binary),
                         static_cast<GLsizei>(length));
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
void sampleCoverage(float value, bool invert)
{
  return glSampleCoverage(static_cast<GLfloat>(value), static_cast<GLboolean>(invert));
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
void stencilFuncSeparate(StencilFaceDirection face, StencilFunction func, int32_t ref, uint32_t mask)
{
  return glStencilFuncSeparate(
    static_cast<GLenum>(face), static_cast<GLenum>(func), static_cast<GLint>(ref), static_cast<GLuint>(mask));
}
void stencilMask(uint32_t mask)
{
  return glStencilMask(static_cast<GLuint>(mask));
}
void stencilMaskSeparate(StencilFaceDirection face, uint32_t mask)
{
  return glStencilMaskSeparate(static_cast<GLenum>(face), static_cast<GLuint>(mask));
}
void stencilOp(StencilOp fail, StencilOp zfail, StencilOp zpass)
{
  return glStencilOp(static_cast<GLenum>(fail), static_cast<GLenum>(zfail), static_cast<GLenum>(zpass));
}
void stencilOpSeparate(StencilFaceDirection face, StencilOp sfail, StencilOp dpfail, StencilOp dppass)
{
  return glStencilOpSeparate(
    static_cast<GLenum>(face), static_cast<GLenum>(sfail), static_cast<GLenum>(dpfail), static_cast<GLenum>(dppass));
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
void vertexAttrib1(uint32_t index, float x)
{
  return glVertexAttrib1f(static_cast<GLuint>(index), static_cast<GLfloat>(x));
}
void vertexAttrib1(uint32_t index, const float* v)
{
  return glVertexAttrib1fv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLfloat*>(v)));
}
void vertexAttrib2(uint32_t index, float x, float y)
{
  return glVertexAttrib2f(static_cast<GLuint>(index), static_cast<GLfloat>(x), static_cast<GLfloat>(y));
}
void vertexAttrib2(uint32_t index, const float* v)
{
  return glVertexAttrib2fv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLfloat*>(v)));
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
void viewport(int32_t x, int32_t y, core::SizeType width, core::SizeType height)
{
  return glViewport(
    static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(width), static_cast<GLsizei>(height));
}
} // namespace glsc2::api
