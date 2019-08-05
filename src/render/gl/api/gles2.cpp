#include "gles2.hpp"
#include "gles2_api_provider.hpp"
namespace gles2
{
void activeTexture(TextureUnit texture)
{
    return glActiveTexture(static_cast<GLenum>(texture));
}
void attachShader(uint32_t program, uint32_t shader)
{
    return glAttachShader(static_cast<GLuint>(program), static_cast<GLuint>(shader));
}
void bindAttribLocation(uint32_t program, uint32_t index, const char *name)
{
    return glBindAttribLocation(static_cast<GLuint>(program), static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLchar *>(name)));
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
    return glBlendColor(static_cast<GLfloat>(red), static_cast<GLfloat>(green), static_cast<GLfloat>(blue), static_cast<GLfloat>(alpha));
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
void blendFuncSeparate(BlendingFactor sfactorRGB, BlendingFactor dfactorRGB, BlendingFactor sfactorAlpha, BlendingFactor dfactorAlpha)
{
    return glBlendFuncSeparate(static_cast<GLenum>(sfactorRGB), static_cast<GLenum>(dfactorRGB), static_cast<GLenum>(sfactorAlpha), static_cast<GLenum>(dfactorAlpha));
}
void bufferData(BufferTargetARB target, std::size_t size, const void *data, BufferUsageARB usage)
{
    return glBufferData(static_cast<GLenum>(target), static_cast<GLsizeiptr>(size), detail::constAway(data), static_cast<GLenum>(usage));
}
void bufferSubData(BufferTargetARB target, std::intptr_t offset, std::size_t size, const void *data)
{
    return glBufferSubData(static_cast<GLenum>(target), static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), detail::constAway(data));
}
FramebufferStatus checkFramebufferStatus(FramebufferTarget target)
{
    return static_cast<FramebufferStatus >(glCheckFramebufferStatus(static_cast<GLenum>(target)));
}
void clear(core::Bitfield<ClearBufferMask> mask)
{
    return glClear(mask.value());
}
void clearColor(float red, float green, float blue, float alpha)
{
    return glClearColor(static_cast<GLfloat>(red), static_cast<GLfloat>(green), static_cast<GLfloat>(blue), static_cast<GLfloat>(alpha));
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
    return glColorMask(static_cast<GLboolean>(red), static_cast<GLboolean>(green), static_cast<GLboolean>(blue), static_cast<GLboolean>(alpha));
}
void compileShader(uint32_t shader)
{
    return glCompileShader(static_cast<GLuint>(shader));
}
void compressedTexImage2D(TextureTarget target, int32_t level, InternalFormat internalformat, core::SizeType width, core::SizeType height, int32_t border, core::SizeType imageSize, const void *data)
{
    return glCompressedTexImage2D(static_cast<GLenum>(target), static_cast<GLint>(level), static_cast<GLenum>(internalformat), static_cast<GLsizei>(width), static_cast<GLsizei>(height), static_cast<GLint>(border), static_cast<GLsizei>(imageSize), detail::constAway(data));
}
void compressedTexSubImage2D(TextureTarget target, int32_t level, int32_t xoffset, int32_t yoffset, core::SizeType width, core::SizeType height, PixelFormat format, core::SizeType imageSize, const void *data)
{
    return glCompressedTexSubImage2D(static_cast<GLenum>(target), static_cast<GLint>(level), static_cast<GLint>(xoffset), static_cast<GLint>(yoffset), static_cast<GLsizei>(width), static_cast<GLsizei>(height), static_cast<GLenum>(format), static_cast<GLsizei>(imageSize), detail::constAway(data));
}
void copyTexImage2D(TextureTarget target, int32_t level, InternalFormat internalformat, int32_t x, int32_t y, core::SizeType width, core::SizeType height, int32_t border)
{
    return glCopyTexImage2D(static_cast<GLenum>(target), static_cast<GLint>(level), static_cast<GLenum>(internalformat), static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(width), static_cast<GLsizei>(height), static_cast<GLint>(border));
}
void copyTexSubImage2D(TextureTarget target, int32_t level, int32_t xoffset, int32_t yoffset, int32_t x, int32_t y, core::SizeType width, core::SizeType height)
{
    return glCopyTexSubImage2D(static_cast<GLenum>(target), static_cast<GLint>(level), static_cast<GLint>(xoffset), static_cast<GLint>(yoffset), static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(width), static_cast<GLsizei>(height));
}
uint32_t createProgram()
{
    return static_cast<uint32_t >(glCreateProgram());
}
uint32_t createShader(ShaderType type)
{
    return static_cast<uint32_t >(glCreateShader(static_cast<GLenum>(type)));
}
void cullFace(CullFaceMode mode)
{
    return glCullFace(static_cast<GLenum>(mode));
}
void deleteBuffers(core::SizeType n, const uint32_t *buffers)
{
    return glDeleteBuffers(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<const GLuint *>(buffers)));
}
void deleteFramebuffers(core::SizeType n, const uint32_t *framebuffers)
{
    return glDeleteFramebuffers(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<const GLuint *>(framebuffers)));
}
void deleteProgram(uint32_t program)
{
    return glDeleteProgram(static_cast<GLuint>(program));
}
void deleteRenderbuffers(core::SizeType n, const uint32_t *renderbuffers)
{
    return glDeleteRenderbuffers(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<const GLuint *>(renderbuffers)));
}
void deleteShader(uint32_t shader)
{
    return glDeleteShader(static_cast<GLuint>(shader));
}
void deleteTextures(core::SizeType n, const uint32_t *textures)
{
    return glDeleteTextures(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<const GLuint *>(textures)));
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
void detachShader(uint32_t program, uint32_t shader)
{
    return glDetachShader(static_cast<GLuint>(program), static_cast<GLuint>(shader));
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
void drawElements(PrimitiveType mode, core::SizeType count, DrawElementsType type, const void *indices)
{
    return glDrawElements(static_cast<GLenum>(mode), static_cast<GLsizei>(count), static_cast<GLenum>(type), detail::constAway(indices));
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
void framebufferRenderbuffer(FramebufferTarget target, FramebufferAttachment attachment, RenderbufferTarget renderbuffertarget, uint32_t renderbuffer)
{
    return glFramebufferRenderbuffer(static_cast<GLenum>(target), static_cast<GLenum>(attachment), static_cast<GLenum>(renderbuffertarget), static_cast<GLuint>(renderbuffer));
}
void framebufferTexture2D(FramebufferTarget target, FramebufferAttachment attachment, TextureTarget textarget, uint32_t texture, int32_t level)
{
    return glFramebufferTexture2D(static_cast<GLenum>(target), static_cast<GLenum>(attachment), static_cast<GLenum>(textarget), static_cast<GLuint>(texture), static_cast<GLint>(level));
}
void frontFace(FrontFaceDirection mode)
{
    return glFrontFace(static_cast<GLenum>(mode));
}
void genBuffers(core::SizeType n, uint32_t *buffers)
{
    return glGenBuffers(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<GLuint *>(buffers)));
}
void genFramebuffers(core::SizeType n, uint32_t *framebuffers)
{
    return glGenFramebuffers(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<GLuint *>(framebuffers)));
}
void genRenderbuffers(core::SizeType n, uint32_t *renderbuffers)
{
    return glGenRenderbuffers(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<GLuint *>(renderbuffers)));
}
void genTextures(core::SizeType n, uint32_t *textures)
{
    return glGenTextures(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<GLuint *>(textures)));
}
void generateMipmap(TextureTarget target)
{
    return glGenerateMipmap(static_cast<GLenum>(target));
}
void getActiveAttrib(uint32_t program, uint32_t index, core::SizeType bufSize, core::SizeType *length, int32_t *size, AttributeType *type, char *name)
{
    return glGetActiveAttrib(static_cast<GLuint>(program), static_cast<GLuint>(index), static_cast<GLsizei>(bufSize), detail::constAway(reinterpret_cast<GLsizei *>(length)), detail::constAway(reinterpret_cast<GLint *>(size)), detail::constAway(reinterpret_cast<GLenum *>(type)), detail::constAway(reinterpret_cast<GLchar *>(name)));
}
void getActiveUniform(uint32_t program, uint32_t index, core::SizeType bufSize, core::SizeType *length, int32_t *size, UniformType *type, char *name)
{
    return glGetActiveUniform(static_cast<GLuint>(program), static_cast<GLuint>(index), static_cast<GLsizei>(bufSize), detail::constAway(reinterpret_cast<GLsizei *>(length)), detail::constAway(reinterpret_cast<GLint *>(size)), detail::constAway(reinterpret_cast<GLenum *>(type)), detail::constAway(reinterpret_cast<GLchar *>(name)));
}
void getAttachedShaders(uint32_t program, core::SizeType maxCount, core::SizeType *count, uint32_t *shaders)
{
    return glGetAttachedShaders(static_cast<GLuint>(program), static_cast<GLsizei>(maxCount), detail::constAway(reinterpret_cast<GLsizei *>(count)), detail::constAway(reinterpret_cast<GLuint *>(shaders)));
}
int32_t getAttribLocation(uint32_t program, const char *name)
{
    return static_cast<int32_t >(glGetAttribLocation(static_cast<GLuint>(program), detail::constAway(reinterpret_cast<const GLchar *>(name))));
}
void getBooleanv(GetPName pname, bool *data)
{
    return glGetBooleanv(static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLboolean *>(data)));
}
void getBufferParameter(BufferTargetARB target, BufferPNameARB pname, int32_t *params)
{
    return glGetBufferParameteriv(static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint *>(params)));
}
ErrorCode getError()
{
    return static_cast<ErrorCode >(glGetError());
}
void getFloatv(GetPName pname, float *data)
{
    return glGetFloatv(static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLfloat *>(data)));
}
void getFramebufferAttachmentParameter(FramebufferTarget target, FramebufferAttachment attachment, FramebufferAttachmentParameterName pname, int32_t *params)
{
    return glGetFramebufferAttachmentParameteriv(static_cast<GLenum>(target), static_cast<GLenum>(attachment), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint *>(params)));
}
void getIntegerv(GetPName pname, int32_t *data)
{
    return glGetIntegerv(static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint *>(data)));
}
void getProgramInfoLog(uint32_t program, core::SizeType bufSize, core::SizeType *length, char *infoLog)
{
    return glGetProgramInfoLog(static_cast<GLuint>(program), static_cast<GLsizei>(bufSize), detail::constAway(reinterpret_cast<GLsizei *>(length)), detail::constAway(reinterpret_cast<GLchar *>(infoLog)));
}
void getProgram(uint32_t program, ProgramPropertyARB pname, int32_t *params)
{
    return glGetProgramiv(static_cast<GLuint>(program), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint *>(params)));
}
void getRenderbufferParameter(RenderbufferTarget target, RenderbufferParameterName pname, int32_t *params)
{
    return glGetRenderbufferParameteriv(static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint *>(params)));
}
void getShaderInfoLog(uint32_t shader, core::SizeType bufSize, core::SizeType *length, char *infoLog)
{
    return glGetShaderInfoLog(static_cast<GLuint>(shader), static_cast<GLsizei>(bufSize), detail::constAway(reinterpret_cast<GLsizei *>(length)), detail::constAway(reinterpret_cast<GLchar *>(infoLog)));
}
void getShaderPrecisionFormat(ShaderType shadertype, PrecisionType precisiontype, int32_t *range, int32_t *precision)
{
    return glGetShaderPrecisionFormat(static_cast<GLenum>(shadertype), static_cast<GLenum>(precisiontype), detail::constAway(reinterpret_cast<GLint *>(range)), detail::constAway(reinterpret_cast<GLint *>(precision)));
}
void getShaderSource(uint32_t shader, core::SizeType bufSize, core::SizeType *length, char *source)
{
    return glGetShaderSource(static_cast<GLuint>(shader), static_cast<GLsizei>(bufSize), detail::constAway(reinterpret_cast<GLsizei *>(length)), detail::constAway(reinterpret_cast<GLchar *>(source)));
}
void getShader(uint32_t shader, ShaderParameterName pname, int32_t *params)
{
    return glGetShaderiv(static_cast<GLuint>(shader), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint *>(params)));
}
const uint8_t *getString(StringName name)
{
    return static_cast<const uint8_t *>(glGetString(static_cast<GLenum>(name)));
}
void getTexParameter(TextureTarget target, GetTextureParameter pname, float *params)
{
    return glGetTexParameterfv(static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLfloat *>(params)));
}
void getTexParameter(TextureTarget target, GetTextureParameter pname, int32_t *params)
{
    return glGetTexParameteriv(static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint *>(params)));
}
int32_t getUniformLocation(uint32_t program, const char *name)
{
    return static_cast<int32_t >(glGetUniformLocation(static_cast<GLuint>(program), detail::constAway(reinterpret_cast<const GLchar *>(name))));
}
void getUniform(uint32_t program, int32_t location, float *params)
{
    return glGetUniformfv(static_cast<GLuint>(program), static_cast<GLint>(location), detail::constAway(reinterpret_cast<GLfloat *>(params)));
}
void getUniform(uint32_t program, int32_t location, int32_t *params)
{
    return glGetUniformiv(static_cast<GLuint>(program), static_cast<GLint>(location), detail::constAway(reinterpret_cast<GLint *>(params)));
}
void getVertexAttribPointerv(uint32_t index, VertexAttribPointerPropertyARB pname, void **pointer)
{
    return glGetVertexAttribPointerv(static_cast<GLuint>(index), static_cast<GLenum>(pname), detail::constAway(pointer));
}
void getVertexAttrib(uint32_t index, VertexAttribPropertyARB pname, float *params)
{
    return glGetVertexAttribfv(static_cast<GLuint>(index), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLfloat *>(params)));
}
void getVertexAttrib(uint32_t index, VertexAttribPropertyARB pname, int32_t *params)
{
    return glGetVertexAttribiv(static_cast<GLuint>(index), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint *>(params)));
}
void hint(HintTarget target, HintMode mode)
{
    return glHint(static_cast<GLenum>(target), static_cast<GLenum>(mode));
}
bool isBuffer(uint32_t buffer)
{
    return static_cast<bool >(glIsBuffer(static_cast<GLuint>(buffer)));
}
bool isEnable(EnableCap cap)
{
    return static_cast<bool >(glIsEnabled(static_cast<GLenum>(cap)));
}
bool isFramebuffer(uint32_t framebuffer)
{
    return static_cast<bool >(glIsFramebuffer(static_cast<GLuint>(framebuffer)));
}
bool isProgram(uint32_t program)
{
    return static_cast<bool >(glIsProgram(static_cast<GLuint>(program)));
}
bool isRenderbuffer(uint32_t renderbuffer)
{
    return static_cast<bool >(glIsRenderbuffer(static_cast<GLuint>(renderbuffer)));
}
bool isShader(uint32_t shader)
{
    return static_cast<bool >(glIsShader(static_cast<GLuint>(shader)));
}
bool isTexture(uint32_t texture)
{
    return static_cast<bool >(glIsTexture(static_cast<GLuint>(texture)));
}
void lineWidth(float width)
{
    return glLineWidth(static_cast<GLfloat>(width));
}
void linkProgram(uint32_t program)
{
    return glLinkProgram(static_cast<GLuint>(program));
}
void pixelStore(PixelStoreParameter pname, int32_t param)
{
    return glPixelStorei(static_cast<GLenum>(pname), static_cast<GLint>(param));
}
void polygonOffset(float factor, float units)
{
    return glPolygonOffset(static_cast<GLfloat>(factor), static_cast<GLfloat>(units));
}
void readPixel(int32_t x, int32_t y, core::SizeType width, core::SizeType height, PixelFormat format, PixelType type, void *pixels)
{
    return glReadPixels(static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(width), static_cast<GLsizei>(height), static_cast<GLenum>(format), static_cast<GLenum>(type), detail::constAway(pixels));
}
void releaseShaderCompiler()
{
    return glReleaseShaderCompiler();
}
void renderbufferStorage(RenderbufferTarget target, InternalFormat internalformat, core::SizeType width, core::SizeType height)
{
    return glRenderbufferStorage(static_cast<GLenum>(target), static_cast<GLenum>(internalformat), static_cast<GLsizei>(width), static_cast<GLsizei>(height));
}
void sampleCoverage(float value, bool invert)
{
    return glSampleCoverage(static_cast<GLfloat>(value), static_cast<GLboolean>(invert));
}
void scissor(int32_t x, int32_t y, core::SizeType width, core::SizeType height)
{
    return glScissor(static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(width), static_cast<GLsizei>(height));
}
void shaderBinary(core::SizeType count, const uint32_t *shaders, core::EnumType binaryformat, const void *binary, core::SizeType length)
{
    return glShaderBinary(static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLuint *>(shaders)), static_cast<GLenum>(binaryformat), detail::constAway(binary), static_cast<GLsizei>(length));
}
void shaderSource(uint32_t shader, core::SizeType count, const char *const*string, const int32_t *length)
{
    return glShaderSource(static_cast<GLuint>(shader), static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLchar *const*>(string)), detail::constAway(reinterpret_cast<const GLint *>(length)));
}
void stencilFunc(StencilFunction func, int32_t ref, uint32_t mask)
{
    return glStencilFunc(static_cast<GLenum>(func), static_cast<GLint>(ref), static_cast<GLuint>(mask));
}
void stencilFuncSeparate(StencilFaceDirection face, StencilFunction func, int32_t ref, uint32_t mask)
{
    return glStencilFuncSeparate(static_cast<GLenum>(face), static_cast<GLenum>(func), static_cast<GLint>(ref), static_cast<GLuint>(mask));
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
    return glStencilOpSeparate(static_cast<GLenum>(face), static_cast<GLenum>(sfail), static_cast<GLenum>(dpfail), static_cast<GLenum>(dppass));
}
void texImage2D(TextureTarget target, int32_t level, InternalFormat internalformat, core::SizeType width, core::SizeType height, int32_t border, PixelFormat format, PixelType type, const void *pixels)
{
    return glTexImage2D(static_cast<GLenum>(target), static_cast<GLint>(level), static_cast<GLenum>(internalformat), static_cast<GLsizei>(width), static_cast<GLsizei>(height), static_cast<GLint>(border), static_cast<GLenum>(format), static_cast<GLenum>(type), detail::constAway(pixels));
}
void texParameter(TextureTarget target, TextureParameterName pname, float param)
{
    return glTexParameterf(static_cast<GLenum>(target), static_cast<GLenum>(pname), static_cast<GLfloat>(param));
}
void texParameter(TextureTarget target, TextureParameterName pname, const float *params)
{
    return glTexParameterfv(static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<const GLfloat *>(params)));
}
void texParameter(TextureTarget target, TextureParameterName pname, int32_t param)
{
    return glTexParameteri(static_cast<GLenum>(target), static_cast<GLenum>(pname), static_cast<GLint>(param));
}
void texParameter(TextureTarget target, TextureParameterName pname, const int32_t *params)
{
    return glTexParameteriv(static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<const GLint *>(params)));
}
void texSubImage2D(TextureTarget target, int32_t level, int32_t xoffset, int32_t yoffset, core::SizeType width, core::SizeType height, PixelFormat format, PixelType type, const void *pixels)
{
    return glTexSubImage2D(static_cast<GLenum>(target), static_cast<GLint>(level), static_cast<GLint>(xoffset), static_cast<GLint>(yoffset), static_cast<GLsizei>(width), static_cast<GLsizei>(height), static_cast<GLenum>(format), static_cast<GLenum>(type), detail::constAway(pixels));
}
void uniform1(int32_t location, float v0)
{
    return glUniform1f(static_cast<GLint>(location), static_cast<GLfloat>(v0));
}
void uniform1(int32_t location, core::SizeType count, const float *value)
{
    return glUniform1fv(static_cast<GLint>(location), static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLfloat *>(value)));
}
void uniform1(int32_t location, int32_t v0)
{
    return glUniform1i(static_cast<GLint>(location), static_cast<GLint>(v0));
}
void uniform1(int32_t location, core::SizeType count, const int32_t *value)
{
    return glUniform1iv(static_cast<GLint>(location), static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLint *>(value)));
}
void uniform2(int32_t location, float v0, float v1)
{
    return glUniform2f(static_cast<GLint>(location), static_cast<GLfloat>(v0), static_cast<GLfloat>(v1));
}
void uniform2(int32_t location, core::SizeType count, const float *value)
{
    return glUniform2fv(static_cast<GLint>(location), static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLfloat *>(value)));
}
void uniform2(int32_t location, int32_t v0, int32_t v1)
{
    return glUniform2i(static_cast<GLint>(location), static_cast<GLint>(v0), static_cast<GLint>(v1));
}
void uniform2(int32_t location, core::SizeType count, const int32_t *value)
{
    return glUniform2iv(static_cast<GLint>(location), static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLint *>(value)));
}
void uniform3(int32_t location, float v0, float v1, float v2)
{
    return glUniform3f(static_cast<GLint>(location), static_cast<GLfloat>(v0), static_cast<GLfloat>(v1), static_cast<GLfloat>(v2));
}
void uniform3(int32_t location, core::SizeType count, const float *value)
{
    return glUniform3fv(static_cast<GLint>(location), static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLfloat *>(value)));
}
void uniform3(int32_t location, int32_t v0, int32_t v1, int32_t v2)
{
    return glUniform3i(static_cast<GLint>(location), static_cast<GLint>(v0), static_cast<GLint>(v1), static_cast<GLint>(v2));
}
void uniform3(int32_t location, core::SizeType count, const int32_t *value)
{
    return glUniform3iv(static_cast<GLint>(location), static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLint *>(value)));
}
void uniform4(int32_t location, float v0, float v1, float v2, float v3)
{
    return glUniform4f(static_cast<GLint>(location), static_cast<GLfloat>(v0), static_cast<GLfloat>(v1), static_cast<GLfloat>(v2), static_cast<GLfloat>(v3));
}
void uniform4(int32_t location, core::SizeType count, const float *value)
{
    return glUniform4fv(static_cast<GLint>(location), static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLfloat *>(value)));
}
void uniform4(int32_t location, int32_t v0, int32_t v1, int32_t v2, int32_t v3)
{
    return glUniform4i(static_cast<GLint>(location), static_cast<GLint>(v0), static_cast<GLint>(v1), static_cast<GLint>(v2), static_cast<GLint>(v3));
}
void uniform4(int32_t location, core::SizeType count, const int32_t *value)
{
    return glUniform4iv(static_cast<GLint>(location), static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLint *>(value)));
}
void uniformMatrix2(int32_t location, core::SizeType count, bool transpose, const float *value)
{
    return glUniformMatrix2fv(static_cast<GLint>(location), static_cast<GLsizei>(count), static_cast<GLboolean>(transpose), detail::constAway(reinterpret_cast<const GLfloat *>(value)));
}
void uniformMatrix3(int32_t location, core::SizeType count, bool transpose, const float *value)
{
    return glUniformMatrix3fv(static_cast<GLint>(location), static_cast<GLsizei>(count), static_cast<GLboolean>(transpose), detail::constAway(reinterpret_cast<const GLfloat *>(value)));
}
void uniformMatrix4(int32_t location, core::SizeType count, bool transpose, const float *value)
{
    return glUniformMatrix4fv(static_cast<GLint>(location), static_cast<GLsizei>(count), static_cast<GLboolean>(transpose), detail::constAway(reinterpret_cast<const GLfloat *>(value)));
}
void useProgram(uint32_t program)
{
    return glUseProgram(static_cast<GLuint>(program));
}
void validateProgram(uint32_t program)
{
    return glValidateProgram(static_cast<GLuint>(program));
}
void vertexAttrib1(uint32_t index, float x)
{
    return glVertexAttrib1f(static_cast<GLuint>(index), static_cast<GLfloat>(x));
}
void vertexAttrib1(uint32_t index, const float *v)
{
    return glVertexAttrib1fv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLfloat *>(v)));
}
void vertexAttrib2(uint32_t index, float x, float y)
{
    return glVertexAttrib2f(static_cast<GLuint>(index), static_cast<GLfloat>(x), static_cast<GLfloat>(y));
}
void vertexAttrib2(uint32_t index, const float *v)
{
    return glVertexAttrib2fv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLfloat *>(v)));
}
void vertexAttrib3(uint32_t index, float x, float y, float z)
{
    return glVertexAttrib3f(static_cast<GLuint>(index), static_cast<GLfloat>(x), static_cast<GLfloat>(y), static_cast<GLfloat>(z));
}
void vertexAttrib3(uint32_t index, const float *v)
{
    return glVertexAttrib3fv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLfloat *>(v)));
}
void vertexAttrib4(uint32_t index, float x, float y, float z, float w)
{
    return glVertexAttrib4f(static_cast<GLuint>(index), static_cast<GLfloat>(x), static_cast<GLfloat>(y), static_cast<GLfloat>(z), static_cast<GLfloat>(w));
}
void vertexAttrib4(uint32_t index, const float *v)
{
    return glVertexAttrib4fv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLfloat *>(v)));
}
void vertexAttribPointer(uint32_t index, int32_t size, VertexAttribPointerType type, bool normalized, core::SizeType stride, const void *pointer)
{
    return glVertexAttribPointer(static_cast<GLuint>(index), static_cast<GLint>(size), static_cast<GLenum>(type), static_cast<GLboolean>(normalized), static_cast<GLsizei>(stride), detail::constAway(pointer));
}
void viewport(int32_t x, int32_t y, core::SizeType width, core::SizeType height)
{
    return glViewport(static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(width), static_cast<GLsizei>(height));
}
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
void beginQuery(QueryTarget target, uint32_t id)
{
    return glBeginQuery(static_cast<GLenum>(target), static_cast<GLuint>(id));
}
void beginTransformFeedback(PrimitiveType primitiveMode)
{
    return glBeginTransformFeedback(static_cast<GLenum>(primitiveMode));
}
void bindBufferBase(BufferTargetARB target, uint32_t index, uint32_t buffer)
{
    return glBindBufferBase(static_cast<GLenum>(target), static_cast<GLuint>(index), static_cast<GLuint>(buffer));
}
void bindBufferRange(BufferTargetARB target, uint32_t index, uint32_t buffer, std::intptr_t offset, std::size_t size)
{
    return glBindBufferRange(static_cast<GLenum>(target), static_cast<GLuint>(index), static_cast<GLuint>(buffer), static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size));
}
void bindSampler(uint32_t unit, uint32_t sampler)
{
    return glBindSampler(static_cast<GLuint>(unit), static_cast<GLuint>(sampler));
}
void bindTransformFeedback(BindTransformFeedbackTarget target, uint32_t id)
{
    return glBindTransformFeedback(static_cast<GLenum>(target), static_cast<GLuint>(id));
}
void bindVertexArray(uint32_t array)
{
    return glBindVertexArray(static_cast<GLuint>(array));
}
void blitFramebuffer(int32_t srcX0, int32_t srcY0, int32_t srcX1, int32_t srcY1, int32_t dstX0, int32_t dstY0, int32_t dstX1, int32_t dstY1, core::Bitfield<ClearBufferMask> mask, BlitFramebufferFilter filter)
{
    return glBlitFramebuffer(static_cast<GLint>(srcX0), static_cast<GLint>(srcY0), static_cast<GLint>(srcX1), static_cast<GLint>(srcY1), static_cast<GLint>(dstX0), static_cast<GLint>(dstY0), static_cast<GLint>(dstX1), static_cast<GLint>(dstY1), mask.value(), static_cast<GLenum>(filter));
}
void clearBufferf(Buffer buffer, int32_t drawbuffer, float depth, int32_t stencil)
{
    return glClearBufferfi(static_cast<GLenum>(buffer), static_cast<GLint>(drawbuffer), static_cast<GLfloat>(depth), static_cast<GLint>(stencil));
}
void clearBuffer(Buffer buffer, int32_t drawbuffer, const float *value)
{
    return glClearBufferfv(static_cast<GLenum>(buffer), static_cast<GLint>(drawbuffer), detail::constAway(reinterpret_cast<const GLfloat *>(value)));
}
void clearBuffer(Buffer buffer, int32_t drawbuffer, const int32_t *value)
{
    return glClearBufferiv(static_cast<GLenum>(buffer), static_cast<GLint>(drawbuffer), detail::constAway(reinterpret_cast<const GLint *>(value)));
}
void clearBuffer(Buffer buffer, int32_t drawbuffer, const uint32_t *value)
{
    return glClearBufferuiv(static_cast<GLenum>(buffer), static_cast<GLint>(drawbuffer), detail::constAway(reinterpret_cast<const GLuint *>(value)));
}
SyncStatus clientWaitSync(core::Sync sync, core::Bitfield<SyncObjectMask> flags, uint64_t timeout)
{
    return static_cast<SyncStatus >(glClientWaitSync(static_cast<GLsync>(sync), flags.value(), static_cast<GLuint64>(timeout)));
}
void compressedTexImage3D(TextureTarget target, int32_t level, InternalFormat internalformat, core::SizeType width, core::SizeType height, core::SizeType depth, int32_t border, core::SizeType imageSize, const void *data)
{
    return glCompressedTexImage3D(static_cast<GLenum>(target), static_cast<GLint>(level), static_cast<GLenum>(internalformat), static_cast<GLsizei>(width), static_cast<GLsizei>(height), static_cast<GLsizei>(depth), static_cast<GLint>(border), static_cast<GLsizei>(imageSize), detail::constAway(data));
}
void compressedTexSubImage3D(TextureTarget target, int32_t level, int32_t xoffset, int32_t yoffset, int32_t zoffset, core::SizeType width, core::SizeType height, core::SizeType depth, PixelFormat format, core::SizeType imageSize, const void *data)
{
    return glCompressedTexSubImage3D(static_cast<GLenum>(target), static_cast<GLint>(level), static_cast<GLint>(xoffset), static_cast<GLint>(yoffset), static_cast<GLint>(zoffset), static_cast<GLsizei>(width), static_cast<GLsizei>(height), static_cast<GLsizei>(depth), static_cast<GLenum>(format), static_cast<GLsizei>(imageSize), detail::constAway(data));
}
void copyBufferSubData(CopyBufferSubDataTarget readTarget, CopyBufferSubDataTarget writeTarget, std::intptr_t readOffset, std::intptr_t writeOffset, std::size_t size)
{
    return glCopyBufferSubData(static_cast<GLenum>(readTarget), static_cast<GLenum>(writeTarget), static_cast<GLintptr>(readOffset), static_cast<GLintptr>(writeOffset), static_cast<GLsizeiptr>(size));
}
void copyTexSubImage3D(TextureTarget target, int32_t level, int32_t xoffset, int32_t yoffset, int32_t zoffset, int32_t x, int32_t y, core::SizeType width, core::SizeType height)
{
    return glCopyTexSubImage3D(static_cast<GLenum>(target), static_cast<GLint>(level), static_cast<GLint>(xoffset), static_cast<GLint>(yoffset), static_cast<GLint>(zoffset), static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(width), static_cast<GLsizei>(height));
}
void deleteQuerie(core::SizeType n, const uint32_t *ids)
{
    return glDeleteQueries(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<const GLuint *>(ids)));
}
void deleteSampler(core::SizeType count, const uint32_t *samplers)
{
    return glDeleteSamplers(static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLuint *>(samplers)));
}
void deleteSync(core::Sync sync)
{
    return glDeleteSync(static_cast<GLsync>(sync));
}
void deleteTransformFeedback(core::SizeType n, const uint32_t *ids)
{
    return glDeleteTransformFeedbacks(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<const GLuint *>(ids)));
}
void deleteVertexArrays(core::SizeType n, const uint32_t *arrays)
{
    return glDeleteVertexArrays(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<const GLuint *>(arrays)));
}
void drawArraysInstance(PrimitiveType mode, int32_t first, core::SizeType count, core::SizeType instancecount)
{
    return glDrawArraysInstanced(static_cast<GLenum>(mode), static_cast<GLint>(first), static_cast<GLsizei>(count), static_cast<GLsizei>(instancecount));
}
void drawBuffers(core::SizeType n, const DrawBufferMode *bufs)
{
    return glDrawBuffers(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<const GLenum *>(bufs)));
}
void drawElementsInstance(PrimitiveType mode, core::SizeType count, DrawElementsType type, const void *indices, core::SizeType instancecount)
{
    return glDrawElementsInstanced(static_cast<GLenum>(mode), static_cast<GLsizei>(count), static_cast<GLenum>(type), detail::constAway(indices), static_cast<GLsizei>(instancecount));
}
void drawRangeElements(PrimitiveType mode, uint32_t start, uint32_t end, core::SizeType count, DrawElementsType type, const void *indices)
{
    return glDrawRangeElements(static_cast<GLenum>(mode), static_cast<GLuint>(start), static_cast<GLuint>(end), static_cast<GLsizei>(count), static_cast<GLenum>(type), detail::constAway(indices));
}
void endQuery(QueryTarget target)
{
    return glEndQuery(static_cast<GLenum>(target));
}
void endTransformFeedback()
{
    return glEndTransformFeedback();
}
core::Sync fenceSync(SyncCondition condition, uint32_t flags)
{
    return static_cast<core::Sync >(glFenceSync(static_cast<GLenum>(condition), static_cast<GLbitfield>(flags)));
}
void flushMappedBufferRange(BufferTargetARB target, std::intptr_t offset, std::size_t length)
{
    return glFlushMappedBufferRange(static_cast<GLenum>(target), static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(length));
}
void framebufferTextureLayer(FramebufferTarget target, FramebufferAttachment attachment, uint32_t texture, int32_t level, int32_t layer)
{
    return glFramebufferTextureLayer(static_cast<GLenum>(target), static_cast<GLenum>(attachment), static_cast<GLuint>(texture), static_cast<GLint>(level), static_cast<GLint>(layer));
}
void genQuerie(core::SizeType n, uint32_t *ids)
{
    return glGenQueries(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<GLuint *>(ids)));
}
void genSampler(core::SizeType count, uint32_t *samplers)
{
    return glGenSamplers(static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<GLuint *>(samplers)));
}
void genTransformFeedback(core::SizeType n, uint32_t *ids)
{
    return glGenTransformFeedbacks(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<GLuint *>(ids)));
}
void genVertexArrays(core::SizeType n, uint32_t *arrays)
{
    return glGenVertexArrays(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<GLuint *>(arrays)));
}
void getActiveUniformBlockName(uint32_t program, uint32_t uniformBlockIndex, core::SizeType bufSize, core::SizeType *length, char *uniformBlockName)
{
    return glGetActiveUniformBlockName(static_cast<GLuint>(program), static_cast<GLuint>(uniformBlockIndex), static_cast<GLsizei>(bufSize), detail::constAway(reinterpret_cast<GLsizei *>(length)), detail::constAway(reinterpret_cast<GLchar *>(uniformBlockName)));
}
void getActiveUniformBlock(uint32_t program, uint32_t uniformBlockIndex, UniformBlockPName pname, int32_t *params)
{
    return glGetActiveUniformBlockiv(static_cast<GLuint>(program), static_cast<GLuint>(uniformBlockIndex), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint *>(params)));
}
void getActiveUniforms(uint32_t program, core::SizeType uniformCount, const uint32_t *uniformIndices, UniformPName pname, int32_t *params)
{
    return glGetActiveUniformsiv(static_cast<GLuint>(program), static_cast<GLsizei>(uniformCount), detail::constAway(reinterpret_cast<const GLuint *>(uniformIndices)), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint *>(params)));
}
void getBufferParameter(BufferTargetARB target, BufferPNameARB pname, int64_t *params)
{
    return glGetBufferParameteri64v(static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint64 *>(params)));
}
void getBufferPointerv(BufferTargetARB target, BufferPointerNameARB pname, void **params)
{
    return glGetBufferPointerv(static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(params));
}
int32_t getFragDataLocation(uint32_t program, const char *name)
{
    return static_cast<int32_t >(glGetFragDataLocation(static_cast<GLuint>(program), detail::constAway(reinterpret_cast<const GLchar *>(name))));
}
void getInteger64i_v(core::EnumType target, uint32_t index, int64_t *data)
{
    return glGetInteger64i_v(static_cast<GLenum>(target), static_cast<GLuint>(index), detail::constAway(reinterpret_cast<GLint64 *>(data)));
}
void getInteger64v(GetPName pname, int64_t *data)
{
    return glGetInteger64v(static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint64 *>(data)));
}
void getIntegeri_v(core::EnumType target, uint32_t index, int32_t *data)
{
    return glGetIntegeri_v(static_cast<GLenum>(target), static_cast<GLuint>(index), detail::constAway(reinterpret_cast<GLint *>(data)));
}
void getInternalformat(TextureTarget target, InternalFormat internalformat, InternalFormatPName pname, core::SizeType bufSize, int32_t *params)
{
    return glGetInternalformativ(static_cast<GLenum>(target), static_cast<GLenum>(internalformat), static_cast<GLenum>(pname), static_cast<GLsizei>(bufSize), detail::constAway(reinterpret_cast<GLint *>(params)));
}
void getProgramBinary(uint32_t program, core::SizeType bufSize, core::SizeType *length, core::EnumType *binaryFormat, void *binary)
{
    return glGetProgramBinary(static_cast<GLuint>(program), static_cast<GLsizei>(bufSize), detail::constAway(reinterpret_cast<GLsizei *>(length)), detail::constAway(reinterpret_cast<GLenum *>(binaryFormat)), detail::constAway(binary));
}
void getQueryObject(uint32_t id, QueryObjectParameterName pname, uint32_t *params)
{
    return glGetQueryObjectuiv(static_cast<GLuint>(id), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLuint *>(params)));
}
void getQuery(QueryTarget target, QueryParameterName pname, int32_t *params)
{
    return glGetQueryiv(static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint *>(params)));
}
void getSamplerParameter(uint32_t sampler, SamplerParameterF pname, float *params)
{
    return glGetSamplerParameterfv(static_cast<GLuint>(sampler), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLfloat *>(params)));
}
void getSamplerParameter(uint32_t sampler, SamplerParameterI pname, int32_t *params)
{
    return glGetSamplerParameteriv(static_cast<GLuint>(sampler), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint *>(params)));
}
const uint8_t *getString(StringName name, uint32_t index)
{
    return static_cast<const uint8_t *>(glGetStringi(static_cast<GLenum>(name), static_cast<GLuint>(index)));
}
void getSync(core::Sync sync, SyncParameterName pname, core::SizeType bufSize, core::SizeType *length, int32_t *values)
{
    return glGetSynciv(static_cast<GLsync>(sync), static_cast<GLenum>(pname), static_cast<GLsizei>(bufSize), detail::constAway(reinterpret_cast<GLsizei *>(length)), detail::constAway(reinterpret_cast<GLint *>(values)));
}
void getTransformFeedbackVarying(uint32_t program, uint32_t index, core::SizeType bufSize, core::SizeType *length, core::SizeType *size, GlslTypeToken *type, char *name)
{
    return glGetTransformFeedbackVarying(static_cast<GLuint>(program), static_cast<GLuint>(index), static_cast<GLsizei>(bufSize), detail::constAway(reinterpret_cast<GLsizei *>(length)), detail::constAway(reinterpret_cast<GLsizei *>(size)), detail::constAway(reinterpret_cast<GLenum *>(type)), detail::constAway(reinterpret_cast<GLchar *>(name)));
}
uint32_t getUniformBlockIndex(uint32_t program, const char *uniformBlockName)
{
    return static_cast<uint32_t >(glGetUniformBlockIndex(static_cast<GLuint>(program), detail::constAway(reinterpret_cast<const GLchar *>(uniformBlockName))));
}
void getUniformIndice(uint32_t program, core::SizeType uniformCount, const char *const*uniformNames, uint32_t *uniformIndices)
{
    return glGetUniformIndices(static_cast<GLuint>(program), static_cast<GLsizei>(uniformCount), detail::constAway(reinterpret_cast<const GLchar *const*>(uniformNames)), detail::constAway(reinterpret_cast<GLuint *>(uniformIndices)));
}
void getUniform(uint32_t program, int32_t location, uint32_t *params)
{
    return glGetUniformuiv(static_cast<GLuint>(program), static_cast<GLint>(location), detail::constAway(reinterpret_cast<GLuint *>(params)));
}
void getVertexAttribI(uint32_t index, VertexAttribEnum pname, int32_t *params)
{
    return glGetVertexAttribIiv(static_cast<GLuint>(index), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint *>(params)));
}
void getVertexAttribI(uint32_t index, VertexAttribEnum pname, uint32_t *params)
{
    return glGetVertexAttribIuiv(static_cast<GLuint>(index), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLuint *>(params)));
}
void invalidateFramebuffer(FramebufferTarget target, core::SizeType numAttachments, const FramebufferAttachment *attachments)
{
    return glInvalidateFramebuffer(static_cast<GLenum>(target), static_cast<GLsizei>(numAttachments), detail::constAway(reinterpret_cast<const GLenum *>(attachments)));
}
void invalidateSubFramebuffer(FramebufferTarget target, core::SizeType numAttachments, const FramebufferAttachment *attachments, int32_t x, int32_t y, core::SizeType width, core::SizeType height)
{
    return glInvalidateSubFramebuffer(static_cast<GLenum>(target), static_cast<GLsizei>(numAttachments), detail::constAway(reinterpret_cast<const GLenum *>(attachments)), static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(width), static_cast<GLsizei>(height));
}
bool isQuery(uint32_t id)
{
    return static_cast<bool >(glIsQuery(static_cast<GLuint>(id)));
}
bool isSampler(uint32_t sampler)
{
    return static_cast<bool >(glIsSampler(static_cast<GLuint>(sampler)));
}
bool isSync(core::Sync sync)
{
    return static_cast<bool >(glIsSync(static_cast<GLsync>(sync)));
}
bool isTransformFeedback(uint32_t id)
{
    return static_cast<bool >(glIsTransformFeedback(static_cast<GLuint>(id)));
}
bool isVertexArray(uint32_t array)
{
    return static_cast<bool >(glIsVertexArray(static_cast<GLuint>(array)));
}
void *mapBufferRange(BufferTargetARB target, std::intptr_t offset, std::size_t length, core::Bitfield<MapBufferAccessMask> access)
{
    return glMapBufferRange(static_cast<GLenum>(target), static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(length), access.value());
}
void pauseTransformFeedback()
{
    return glPauseTransformFeedback();
}
void programBinary(uint32_t program, core::EnumType binaryFormat, const void *binary, core::SizeType length)
{
    return glProgramBinary(static_cast<GLuint>(program), static_cast<GLenum>(binaryFormat), detail::constAway(binary), static_cast<GLsizei>(length));
}
void programParameter(uint32_t program, ProgramParameterPName pname, int32_t value)
{
    return glProgramParameteri(static_cast<GLuint>(program), static_cast<GLenum>(pname), static_cast<GLint>(value));
}
void readBuffer(ReadBufferMode src)
{
    return glReadBuffer(static_cast<GLenum>(src));
}
void renderbufferStorageMultisample(RenderbufferTarget target, core::SizeType samples, InternalFormat internalformat, core::SizeType width, core::SizeType height)
{
    return glRenderbufferStorageMultisample(static_cast<GLenum>(target), static_cast<GLsizei>(samples), static_cast<GLenum>(internalformat), static_cast<GLsizei>(width), static_cast<GLsizei>(height));
}
void resumeTransformFeedback()
{
    return glResumeTransformFeedback();
}
void samplerParameter(uint32_t sampler, SamplerParameterF pname, float param)
{
    return glSamplerParameterf(static_cast<GLuint>(sampler), static_cast<GLenum>(pname), static_cast<GLfloat>(param));
}
void samplerParameter(uint32_t sampler, SamplerParameterF pname, const float *param)
{
    return glSamplerParameterfv(static_cast<GLuint>(sampler), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<const GLfloat *>(param)));
}
void samplerParameter(uint32_t sampler, SamplerParameterI pname, int32_t param)
{
    return glSamplerParameteri(static_cast<GLuint>(sampler), static_cast<GLenum>(pname), static_cast<GLint>(param));
}
void samplerParameter(uint32_t sampler, SamplerParameterI pname, const int32_t *param)
{
    return glSamplerParameteriv(static_cast<GLuint>(sampler), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<const GLint *>(param)));
}
void texImage3D(TextureTarget target, int32_t level, int32_t internalformat, core::SizeType width, core::SizeType height, core::SizeType depth, int32_t border, PixelFormat format, PixelType type, const void *pixels)
{
    return glTexImage3D(static_cast<GLenum>(target), static_cast<GLint>(level), static_cast<GLint>(internalformat), static_cast<GLsizei>(width), static_cast<GLsizei>(height), static_cast<GLsizei>(depth), static_cast<GLint>(border), static_cast<GLenum>(format), static_cast<GLenum>(type), detail::constAway(pixels));
}
void texStorage2D(TextureTarget target, core::SizeType levels, InternalFormat internalformat, core::SizeType width, core::SizeType height)
{
    return glTexStorage2D(static_cast<GLenum>(target), static_cast<GLsizei>(levels), static_cast<GLenum>(internalformat), static_cast<GLsizei>(width), static_cast<GLsizei>(height));
}
void texStorage3D(TextureTarget target, core::SizeType levels, InternalFormat internalformat, core::SizeType width, core::SizeType height, core::SizeType depth)
{
    return glTexStorage3D(static_cast<GLenum>(target), static_cast<GLsizei>(levels), static_cast<GLenum>(internalformat), static_cast<GLsizei>(width), static_cast<GLsizei>(height), static_cast<GLsizei>(depth));
}
void texSubImage3D(TextureTarget target, int32_t level, int32_t xoffset, int32_t yoffset, int32_t zoffset, core::SizeType width, core::SizeType height, core::SizeType depth, PixelFormat format, PixelType type, const void *pixels)
{
    return glTexSubImage3D(static_cast<GLenum>(target), static_cast<GLint>(level), static_cast<GLint>(xoffset), static_cast<GLint>(yoffset), static_cast<GLint>(zoffset), static_cast<GLsizei>(width), static_cast<GLsizei>(height), static_cast<GLsizei>(depth), static_cast<GLenum>(format), static_cast<GLenum>(type), detail::constAway(pixels));
}
void transformFeedbackVarying(uint32_t program, core::SizeType count, const char *const*varyings, TransformFeedbackBufferMode bufferMode)
{
    return glTransformFeedbackVaryings(static_cast<GLuint>(program), static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLchar *const*>(varyings)), static_cast<GLenum>(bufferMode));
}
void uniform1(int32_t location, uint32_t v0)
{
    return glUniform1ui(static_cast<GLint>(location), static_cast<GLuint>(v0));
}
void uniform1(int32_t location, core::SizeType count, const uint32_t *value)
{
    return glUniform1uiv(static_cast<GLint>(location), static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLuint *>(value)));
}
void uniform2(int32_t location, uint32_t v0, uint32_t v1)
{
    return glUniform2ui(static_cast<GLint>(location), static_cast<GLuint>(v0), static_cast<GLuint>(v1));
}
void uniform2(int32_t location, core::SizeType count, const uint32_t *value)
{
    return glUniform2uiv(static_cast<GLint>(location), static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLuint *>(value)));
}
void uniform3(int32_t location, uint32_t v0, uint32_t v1, uint32_t v2)
{
    return glUniform3ui(static_cast<GLint>(location), static_cast<GLuint>(v0), static_cast<GLuint>(v1), static_cast<GLuint>(v2));
}
void uniform3(int32_t location, core::SizeType count, const uint32_t *value)
{
    return glUniform3uiv(static_cast<GLint>(location), static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLuint *>(value)));
}
void uniform4(int32_t location, uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3)
{
    return glUniform4ui(static_cast<GLint>(location), static_cast<GLuint>(v0), static_cast<GLuint>(v1), static_cast<GLuint>(v2), static_cast<GLuint>(v3));
}
void uniform4(int32_t location, core::SizeType count, const uint32_t *value)
{
    return glUniform4uiv(static_cast<GLint>(location), static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLuint *>(value)));
}
void uniformBlockBinding(uint32_t program, uint32_t uniformBlockIndex, uint32_t uniformBlockBinding)
{
    return glUniformBlockBinding(static_cast<GLuint>(program), static_cast<GLuint>(uniformBlockIndex), static_cast<GLuint>(uniformBlockBinding));
}
void uniformMatrix2x3(int32_t location, core::SizeType count, bool transpose, const float *value)
{
    return glUniformMatrix2x3fv(static_cast<GLint>(location), static_cast<GLsizei>(count), static_cast<GLboolean>(transpose), detail::constAway(reinterpret_cast<const GLfloat *>(value)));
}
void uniformMatrix2x4(int32_t location, core::SizeType count, bool transpose, const float *value)
{
    return glUniformMatrix2x4fv(static_cast<GLint>(location), static_cast<GLsizei>(count), static_cast<GLboolean>(transpose), detail::constAway(reinterpret_cast<const GLfloat *>(value)));
}
void uniformMatrix3x2(int32_t location, core::SizeType count, bool transpose, const float *value)
{
    return glUniformMatrix3x2fv(static_cast<GLint>(location), static_cast<GLsizei>(count), static_cast<GLboolean>(transpose), detail::constAway(reinterpret_cast<const GLfloat *>(value)));
}
void uniformMatrix3x4(int32_t location, core::SizeType count, bool transpose, const float *value)
{
    return glUniformMatrix3x4fv(static_cast<GLint>(location), static_cast<GLsizei>(count), static_cast<GLboolean>(transpose), detail::constAway(reinterpret_cast<const GLfloat *>(value)));
}
void uniformMatrix4x2(int32_t location, core::SizeType count, bool transpose, const float *value)
{
    return glUniformMatrix4x2fv(static_cast<GLint>(location), static_cast<GLsizei>(count), static_cast<GLboolean>(transpose), detail::constAway(reinterpret_cast<const GLfloat *>(value)));
}
void uniformMatrix4x3(int32_t location, core::SizeType count, bool transpose, const float *value)
{
    return glUniformMatrix4x3fv(static_cast<GLint>(location), static_cast<GLsizei>(count), static_cast<GLboolean>(transpose), detail::constAway(reinterpret_cast<const GLfloat *>(value)));
}
bool unmapBuffer(BufferTargetARB target)
{
    return static_cast<bool >(glUnmapBuffer(static_cast<GLenum>(target)));
}
void vertexAttribDivisor(uint32_t index, uint32_t divisor)
{
    return glVertexAttribDivisor(static_cast<GLuint>(index), static_cast<GLuint>(divisor));
}
void vertexAttribI4(uint32_t index, int32_t x, int32_t y, int32_t z, int32_t w)
{
    return glVertexAttribI4i(static_cast<GLuint>(index), static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLint>(z), static_cast<GLint>(w));
}
void vertexAttribI4(uint32_t index, const int32_t *v)
{
    return glVertexAttribI4iv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLint *>(v)));
}
void vertexAttribI4(uint32_t index, uint32_t x, uint32_t y, uint32_t z, uint32_t w)
{
    return glVertexAttribI4ui(static_cast<GLuint>(index), static_cast<GLuint>(x), static_cast<GLuint>(y), static_cast<GLuint>(z), static_cast<GLuint>(w));
}
void vertexAttribI4(uint32_t index, const uint32_t *v)
{
    return glVertexAttribI4uiv(static_cast<GLuint>(index), detail::constAway(reinterpret_cast<const GLuint *>(v)));
}
void vertexAttribIPointer(uint32_t index, int32_t size, VertexAttribPointerType type, core::SizeType stride, const void *pointer)
{
    return glVertexAttribIPointer(static_cast<GLuint>(index), static_cast<GLint>(size), static_cast<GLenum>(type), static_cast<GLsizei>(stride), detail::constAway(pointer));
}
void waitSync(core::Sync sync, uint32_t flags, uint64_t timeout)
{
    return glWaitSync(static_cast<GLsync>(sync), static_cast<GLbitfield>(flags), static_cast<GLuint64>(timeout));
}
#endif
#if defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
void activeShaderProgram(uint32_t pipeline, uint32_t program)
{
    return glActiveShaderProgram(static_cast<GLuint>(pipeline), static_cast<GLuint>(program));
}
void bindImageTexture(uint32_t unit, uint32_t texture, int32_t level, bool layered, int32_t layer, BufferAccessARB access, InternalFormat format)
{
    return glBindImageTexture(static_cast<GLuint>(unit), static_cast<GLuint>(texture), static_cast<GLint>(level), static_cast<GLboolean>(layered), static_cast<GLint>(layer), static_cast<GLenum>(access), static_cast<GLenum>(format));
}
void bindProgramPipeline(uint32_t pipeline)
{
    return glBindProgramPipeline(static_cast<GLuint>(pipeline));
}
void bindVertexBuffer(uint32_t bindingindex, uint32_t buffer, std::intptr_t offset, core::SizeType stride)
{
    return glBindVertexBuffer(static_cast<GLuint>(bindingindex), static_cast<GLuint>(buffer), static_cast<GLintptr>(offset), static_cast<GLsizei>(stride));
}
uint32_t createShaderProgramv(ShaderType type, core::SizeType count, const char *const*strings)
{
    return static_cast<uint32_t >(glCreateShaderProgramv(static_cast<GLenum>(type), static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLchar *const*>(strings))));
}
void deleteProgramPipeline(core::SizeType n, const uint32_t *pipelines)
{
    return glDeleteProgramPipelines(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<const GLuint *>(pipelines)));
}
void dispatchCompute(uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z)
{
    return glDispatchCompute(static_cast<GLuint>(num_groups_x), static_cast<GLuint>(num_groups_y), static_cast<GLuint>(num_groups_z));
}
void dispatchComputeIndirect(std::intptr_t indirect)
{
    return glDispatchComputeIndirect(static_cast<GLintptr>(indirect));
}
void drawArraysIndirect(PrimitiveType mode, const void *indirect)
{
    return glDrawArraysIndirect(static_cast<GLenum>(mode), detail::constAway(indirect));
}
void drawElementsIndirect(PrimitiveType mode, DrawElementsType type, const void *indirect)
{
    return glDrawElementsIndirect(static_cast<GLenum>(mode), static_cast<GLenum>(type), detail::constAway(indirect));
}
void framebufferParameter(FramebufferTarget target, FramebufferParameterName pname, int32_t param)
{
    return glFramebufferParameteri(static_cast<GLenum>(target), static_cast<GLenum>(pname), static_cast<GLint>(param));
}
void genProgramPipeline(core::SizeType n, uint32_t *pipelines)
{
    return glGenProgramPipelines(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<GLuint *>(pipelines)));
}
void getBooleani_v(BufferTargetARB target, uint32_t index, bool *data)
{
    return glGetBooleani_v(static_cast<GLenum>(target), static_cast<GLuint>(index), detail::constAway(reinterpret_cast<GLboolean *>(data)));
}
void getFramebufferParameter(FramebufferTarget target, FramebufferAttachmentParameterName pname, int32_t *params)
{
    return glGetFramebufferParameteriv(static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint *>(params)));
}
void getMultisample(GetMultisamplePNameNV pname, uint32_t index, float *val)
{
    return glGetMultisamplefv(static_cast<GLenum>(pname), static_cast<GLuint>(index), detail::constAway(reinterpret_cast<GLfloat *>(val)));
}
void getProgramInterface(uint32_t program, ProgramInterface programInterface, ProgramInterfacePName pname, int32_t *params)
{
    return glGetProgramInterfaceiv(static_cast<GLuint>(program), static_cast<GLenum>(programInterface), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint *>(params)));
}
void getProgramPipelineInfoLog(uint32_t pipeline, core::SizeType bufSize, core::SizeType *length, char *infoLog)
{
    return glGetProgramPipelineInfoLog(static_cast<GLuint>(pipeline), static_cast<GLsizei>(bufSize), detail::constAway(reinterpret_cast<GLsizei *>(length)), detail::constAway(reinterpret_cast<GLchar *>(infoLog)));
}
void getProgramPipeline(uint32_t pipeline, PipelineParameterName pname, int32_t *params)
{
    return glGetProgramPipelineiv(static_cast<GLuint>(pipeline), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint *>(params)));
}
uint32_t getProgramResourceIndex(uint32_t program, ProgramInterface programInterface, const char *name)
{
    return static_cast<uint32_t >(glGetProgramResourceIndex(static_cast<GLuint>(program), static_cast<GLenum>(programInterface), detail::constAway(reinterpret_cast<const GLchar *>(name))));
}
int32_t getProgramResourceLocation(uint32_t program, ProgramInterface programInterface, const char *name)
{
    return static_cast<int32_t >(glGetProgramResourceLocation(static_cast<GLuint>(program), static_cast<GLenum>(programInterface), detail::constAway(reinterpret_cast<const GLchar *>(name))));
}
void getProgramResourceName(uint32_t program, ProgramInterface programInterface, uint32_t index, core::SizeType bufSize, core::SizeType *length, char *name)
{
    return glGetProgramResourceName(static_cast<GLuint>(program), static_cast<GLenum>(programInterface), static_cast<GLuint>(index), static_cast<GLsizei>(bufSize), detail::constAway(reinterpret_cast<GLsizei *>(length)), detail::constAway(reinterpret_cast<GLchar *>(name)));
}
void getProgramResource(uint32_t program, ProgramInterface programInterface, uint32_t index, core::SizeType propCount, const ProgramResourceProperty *props, core::SizeType bufSize, core::SizeType *length, int32_t *params)
{
    return glGetProgramResourceiv(static_cast<GLuint>(program), static_cast<GLenum>(programInterface), static_cast<GLuint>(index), static_cast<GLsizei>(propCount), detail::constAway(reinterpret_cast<const GLenum *>(props)), static_cast<GLsizei>(bufSize), detail::constAway(reinterpret_cast<GLsizei *>(length)), detail::constAway(reinterpret_cast<GLint *>(params)));
}
void getTexLevelParameter(TextureTarget target, int32_t level, GetTextureParameter pname, float *params)
{
    return glGetTexLevelParameterfv(static_cast<GLenum>(target), static_cast<GLint>(level), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLfloat *>(params)));
}
void getTexLevelParameter(TextureTarget target, int32_t level, GetTextureParameter pname, int32_t *params)
{
    return glGetTexLevelParameteriv(static_cast<GLenum>(target), static_cast<GLint>(level), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint *>(params)));
}
bool isProgramPipeline(uint32_t pipeline)
{
    return static_cast<bool >(glIsProgramPipeline(static_cast<GLuint>(pipeline)));
}
void memoryBarrier(core::Bitfield<MemoryBarrierMask> barriers)
{
    return glMemoryBarrier(barriers.value());
}
void memoryBarrierByRegion(core::Bitfield<MemoryBarrierMask> barriers)
{
    return glMemoryBarrierByRegion(barriers.value());
}
void programUniform1(uint32_t program, int32_t location, float v0)
{
    return glProgramUniform1f(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLfloat>(v0));
}
void programUniform1(uint32_t program, int32_t location, core::SizeType count, const float *value)
{
    return glProgramUniform1fv(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLfloat *>(value)));
}
void programUniform1(uint32_t program, int32_t location, int32_t v0)
{
    return glProgramUniform1i(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLint>(v0));
}
void programUniform1(uint32_t program, int32_t location, core::SizeType count, const int32_t *value)
{
    return glProgramUniform1iv(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLint *>(value)));
}
void programUniform1(uint32_t program, int32_t location, uint32_t v0)
{
    return glProgramUniform1ui(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLuint>(v0));
}
void programUniform1(uint32_t program, int32_t location, core::SizeType count, const uint32_t *value)
{
    return glProgramUniform1uiv(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLuint *>(value)));
}
void programUniform2(uint32_t program, int32_t location, float v0, float v1)
{
    return glProgramUniform2f(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLfloat>(v0), static_cast<GLfloat>(v1));
}
void programUniform2(uint32_t program, int32_t location, core::SizeType count, const float *value)
{
    return glProgramUniform2fv(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLfloat *>(value)));
}
void programUniform2(uint32_t program, int32_t location, int32_t v0, int32_t v1)
{
    return glProgramUniform2i(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLint>(v0), static_cast<GLint>(v1));
}
void programUniform2(uint32_t program, int32_t location, core::SizeType count, const int32_t *value)
{
    return glProgramUniform2iv(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLint *>(value)));
}
void programUniform2(uint32_t program, int32_t location, uint32_t v0, uint32_t v1)
{
    return glProgramUniform2ui(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLuint>(v0), static_cast<GLuint>(v1));
}
void programUniform2(uint32_t program, int32_t location, core::SizeType count, const uint32_t *value)
{
    return glProgramUniform2uiv(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLuint *>(value)));
}
void programUniform3(uint32_t program, int32_t location, float v0, float v1, float v2)
{
    return glProgramUniform3f(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLfloat>(v0), static_cast<GLfloat>(v1), static_cast<GLfloat>(v2));
}
void programUniform3(uint32_t program, int32_t location, core::SizeType count, const float *value)
{
    return glProgramUniform3fv(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLfloat *>(value)));
}
void programUniform3(uint32_t program, int32_t location, int32_t v0, int32_t v1, int32_t v2)
{
    return glProgramUniform3i(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLint>(v0), static_cast<GLint>(v1), static_cast<GLint>(v2));
}
void programUniform3(uint32_t program, int32_t location, core::SizeType count, const int32_t *value)
{
    return glProgramUniform3iv(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLint *>(value)));
}
void programUniform3(uint32_t program, int32_t location, uint32_t v0, uint32_t v1, uint32_t v2)
{
    return glProgramUniform3ui(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLuint>(v0), static_cast<GLuint>(v1), static_cast<GLuint>(v2));
}
void programUniform3(uint32_t program, int32_t location, core::SizeType count, const uint32_t *value)
{
    return glProgramUniform3uiv(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLuint *>(value)));
}
void programUniform4(uint32_t program, int32_t location, float v0, float v1, float v2, float v3)
{
    return glProgramUniform4f(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLfloat>(v0), static_cast<GLfloat>(v1), static_cast<GLfloat>(v2), static_cast<GLfloat>(v3));
}
void programUniform4(uint32_t program, int32_t location, core::SizeType count, const float *value)
{
    return glProgramUniform4fv(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLfloat *>(value)));
}
void programUniform4(uint32_t program, int32_t location, int32_t v0, int32_t v1, int32_t v2, int32_t v3)
{
    return glProgramUniform4i(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLint>(v0), static_cast<GLint>(v1), static_cast<GLint>(v2), static_cast<GLint>(v3));
}
void programUniform4(uint32_t program, int32_t location, core::SizeType count, const int32_t *value)
{
    return glProgramUniform4iv(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLint *>(value)));
}
void programUniform4(uint32_t program, int32_t location, uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3)
{
    return glProgramUniform4ui(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLuint>(v0), static_cast<GLuint>(v1), static_cast<GLuint>(v2), static_cast<GLuint>(v3));
}
void programUniform4(uint32_t program, int32_t location, core::SizeType count, const uint32_t *value)
{
    return glProgramUniform4uiv(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLuint *>(value)));
}
void programUniformMatrix2(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float *value)
{
    return glProgramUniformMatrix2fv(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLsizei>(count), static_cast<GLboolean>(transpose), detail::constAway(reinterpret_cast<const GLfloat *>(value)));
}
void programUniformMatrix2x3(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float *value)
{
    return glProgramUniformMatrix2x3fv(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLsizei>(count), static_cast<GLboolean>(transpose), detail::constAway(reinterpret_cast<const GLfloat *>(value)));
}
void programUniformMatrix2x4(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float *value)
{
    return glProgramUniformMatrix2x4fv(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLsizei>(count), static_cast<GLboolean>(transpose), detail::constAway(reinterpret_cast<const GLfloat *>(value)));
}
void programUniformMatrix3(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float *value)
{
    return glProgramUniformMatrix3fv(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLsizei>(count), static_cast<GLboolean>(transpose), detail::constAway(reinterpret_cast<const GLfloat *>(value)));
}
void programUniformMatrix3x2(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float *value)
{
    return glProgramUniformMatrix3x2fv(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLsizei>(count), static_cast<GLboolean>(transpose), detail::constAway(reinterpret_cast<const GLfloat *>(value)));
}
void programUniformMatrix3x4(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float *value)
{
    return glProgramUniformMatrix3x4fv(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLsizei>(count), static_cast<GLboolean>(transpose), detail::constAway(reinterpret_cast<const GLfloat *>(value)));
}
void programUniformMatrix4(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float *value)
{
    return glProgramUniformMatrix4fv(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLsizei>(count), static_cast<GLboolean>(transpose), detail::constAway(reinterpret_cast<const GLfloat *>(value)));
}
void programUniformMatrix4x2(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float *value)
{
    return glProgramUniformMatrix4x2fv(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLsizei>(count), static_cast<GLboolean>(transpose), detail::constAway(reinterpret_cast<const GLfloat *>(value)));
}
void programUniformMatrix4x3(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float *value)
{
    return glProgramUniformMatrix4x3fv(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLsizei>(count), static_cast<GLboolean>(transpose), detail::constAway(reinterpret_cast<const GLfloat *>(value)));
}
void sampleMask(uint32_t maskNumber, uint32_t mask)
{
    return glSampleMaski(static_cast<GLuint>(maskNumber), static_cast<GLbitfield>(mask));
}
void texStorage2DMultisample(TextureTarget target, core::SizeType samples, InternalFormat internalformat, core::SizeType width, core::SizeType height, bool fixedsamplelocations)
{
    return glTexStorage2DMultisample(static_cast<GLenum>(target), static_cast<GLsizei>(samples), static_cast<GLenum>(internalformat), static_cast<GLsizei>(width), static_cast<GLsizei>(height), static_cast<GLboolean>(fixedsamplelocations));
}
void useProgramStage(uint32_t pipeline, core::Bitfield<UseProgramStageMask> stages, uint32_t program)
{
    return glUseProgramStages(static_cast<GLuint>(pipeline), stages.value(), static_cast<GLuint>(program));
}
void validateProgramPipeline(uint32_t pipeline)
{
    return glValidateProgramPipeline(static_cast<GLuint>(pipeline));
}
void vertexAttribBinding(uint32_t attribindex, uint32_t bindingindex)
{
    return glVertexAttribBinding(static_cast<GLuint>(attribindex), static_cast<GLuint>(bindingindex));
}
void vertexAttribFormat(uint32_t attribindex, int32_t size, VertexAttribType type, bool normalized, uint32_t relativeoffset)
{
    return glVertexAttribFormat(static_cast<GLuint>(attribindex), static_cast<GLint>(size), static_cast<GLenum>(type), static_cast<GLboolean>(normalized), static_cast<GLuint>(relativeoffset));
}
void vertexAttribIFormat(uint32_t attribindex, int32_t size, VertexAttribIType type, uint32_t relativeoffset)
{
    return glVertexAttribIFormat(static_cast<GLuint>(attribindex), static_cast<GLint>(size), static_cast<GLenum>(type), static_cast<GLuint>(relativeoffset));
}
void vertexBindingDivisor(uint32_t bindingindex, uint32_t divisor)
{
    return glVertexBindingDivisor(static_cast<GLuint>(bindingindex), static_cast<GLuint>(divisor));
}
#endif
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
void blendBarrier()
{
    return glBlendBarrier();
}
void blendEquationSeparate(uint32_t buf, BlendEquationModeEXT modeRGB, BlendEquationModeEXT modeAlpha)
{
    return glBlendEquationSeparatei(static_cast<GLuint>(buf), static_cast<GLenum>(modeRGB), static_cast<GLenum>(modeAlpha));
}
void blendEquation(uint32_t buf, BlendEquationModeEXT mode)
{
    return glBlendEquationi(static_cast<GLuint>(buf), static_cast<GLenum>(mode));
}
void blendFuncSeparate(uint32_t buf, BlendingFactor srcRGB, BlendingFactor dstRGB, BlendingFactor srcAlpha, BlendingFactor dstAlpha)
{
    return glBlendFuncSeparatei(static_cast<GLuint>(buf), static_cast<GLenum>(srcRGB), static_cast<GLenum>(dstRGB), static_cast<GLenum>(srcAlpha), static_cast<GLenum>(dstAlpha));
}
void blendFunc(uint32_t buf, BlendingFactor src, BlendingFactor dst)
{
    return glBlendFunci(static_cast<GLuint>(buf), static_cast<GLenum>(src), static_cast<GLenum>(dst));
}
void colorMask(uint32_t index, bool r, bool g, bool b, bool a)
{
    return glColorMaski(static_cast<GLuint>(index), static_cast<GLboolean>(r), static_cast<GLboolean>(g), static_cast<GLboolean>(b), static_cast<GLboolean>(a));
}
void copyImageSubData(uint32_t srcName, CopyImageSubDataTarget srcTarget, int32_t srcLevel, int32_t srcX, int32_t srcY, int32_t srcZ, uint32_t dstName, CopyImageSubDataTarget dstTarget, int32_t dstLevel, int32_t dstX, int32_t dstY, int32_t dstZ, core::SizeType srcWidth, core::SizeType srcHeight, core::SizeType srcDepth)
{
    return glCopyImageSubData(static_cast<GLuint>(srcName), static_cast<GLenum>(srcTarget), static_cast<GLint>(srcLevel), static_cast<GLint>(srcX), static_cast<GLint>(srcY), static_cast<GLint>(srcZ), static_cast<GLuint>(dstName), static_cast<GLenum>(dstTarget), static_cast<GLint>(dstLevel), static_cast<GLint>(dstX), static_cast<GLint>(dstY), static_cast<GLint>(dstZ), static_cast<GLsizei>(srcWidth), static_cast<GLsizei>(srcHeight), static_cast<GLsizei>(srcDepth));
}
void debugMessageCallback(core::DebugProc callback, const void *userParam)
{
    return glDebugMessageCallback(reinterpret_cast<GLDEBUGPROC>(callback), detail::constAway(userParam));
}
void debugMessageControl(DebugSource source, DebugType type, DebugSeverity severity, core::SizeType count, const uint32_t *ids, bool enabled)
{
    return glDebugMessageControl(static_cast<GLenum>(source), static_cast<GLenum>(type), static_cast<GLenum>(severity), static_cast<GLsizei>(count), detail::constAway(reinterpret_cast<const GLuint *>(ids)), static_cast<GLboolean>(enabled));
}
void debugMessageInsert(DebugSource source, DebugType type, uint32_t id, DebugSeverity severity, core::SizeType length, const char *buf)
{
    return glDebugMessageInsert(static_cast<GLenum>(source), static_cast<GLenum>(type), static_cast<GLuint>(id), static_cast<GLenum>(severity), static_cast<GLsizei>(length), detail::constAway(reinterpret_cast<const GLchar *>(buf)));
}
void disable(EnableCap target, uint32_t index)
{
    return glDisablei(static_cast<GLenum>(target), static_cast<GLuint>(index));
}
void drawElementsBaseVertex(PrimitiveType mode, core::SizeType count, DrawElementsType type, const void *indices, int32_t basevertex)
{
    return glDrawElementsBaseVertex(static_cast<GLenum>(mode), static_cast<GLsizei>(count), static_cast<GLenum>(type), detail::constAway(indices), static_cast<GLint>(basevertex));
}
void drawElementsInstancedBaseVertex(PrimitiveType mode, core::SizeType count, DrawElementsType type, const void *indices, core::SizeType instancecount, int32_t basevertex)
{
    return glDrawElementsInstancedBaseVertex(static_cast<GLenum>(mode), static_cast<GLsizei>(count), static_cast<GLenum>(type), detail::constAway(indices), static_cast<GLsizei>(instancecount), static_cast<GLint>(basevertex));
}
void drawRangeElementsBaseVertex(PrimitiveType mode, uint32_t start, uint32_t end, core::SizeType count, DrawElementsType type, const void *indices, int32_t basevertex)
{
    return glDrawRangeElementsBaseVertex(static_cast<GLenum>(mode), static_cast<GLuint>(start), static_cast<GLuint>(end), static_cast<GLsizei>(count), static_cast<GLenum>(type), detail::constAway(indices), static_cast<GLint>(basevertex));
}
void enable(EnableCap target, uint32_t index)
{
    return glEnablei(static_cast<GLenum>(target), static_cast<GLuint>(index));
}
void framebufferTexture(FramebufferTarget target, FramebufferAttachment attachment, uint32_t texture, int32_t level)
{
    return glFramebufferTexture(static_cast<GLenum>(target), static_cast<GLenum>(attachment), static_cast<GLuint>(texture), static_cast<GLint>(level));
}
uint32_t getDebugMessageLog(uint32_t count, core::SizeType bufSize, DebugSource *sources, DebugType *types, uint32_t *ids, DebugSeverity *severities, core::SizeType *lengths, char *messageLog)
{
    return static_cast<uint32_t >(glGetDebugMessageLog(static_cast<GLuint>(count), static_cast<GLsizei>(bufSize), detail::constAway(reinterpret_cast<GLenum *>(sources)), detail::constAway(reinterpret_cast<GLenum *>(types)), detail::constAway(reinterpret_cast<GLuint *>(ids)), detail::constAway(reinterpret_cast<GLenum *>(severities)), detail::constAway(reinterpret_cast<GLsizei *>(lengths)), detail::constAway(reinterpret_cast<GLchar *>(messageLog))));
}
GraphicsResetStatus getGraphicsResetStatus()
{
    return static_cast<GraphicsResetStatus >(glGetGraphicsResetStatus());
}
void getObjectLabel(ObjectIdentifier identifier, uint32_t name, core::SizeType bufSize, core::SizeType *length, char *label)
{
    return glGetObjectLabel(static_cast<GLenum>(identifier), static_cast<GLuint>(name), static_cast<GLsizei>(bufSize), detail::constAway(reinterpret_cast<GLsizei *>(length)), detail::constAway(reinterpret_cast<GLchar *>(label)));
}
void getObjectPtrLabel(const void *ptr, core::SizeType bufSize, core::SizeType *length, char *label)
{
    return glGetObjectPtrLabel(detail::constAway(ptr), static_cast<GLsizei>(bufSize), detail::constAway(reinterpret_cast<GLsizei *>(length)), detail::constAway(reinterpret_cast<GLchar *>(label)));
}
void getPointerv(GetPointervPName pname, void **params)
{
    return glGetPointerv(static_cast<GLenum>(pname), detail::constAway(params));
}
void getSamplerParameterI(uint32_t sampler, SamplerParameterI pname, int32_t *params)
{
    return glGetSamplerParameterIiv(static_cast<GLuint>(sampler), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint *>(params)));
}
void getSamplerParameterI(uint32_t sampler, SamplerParameterI pname, uint32_t *params)
{
    return glGetSamplerParameterIuiv(static_cast<GLuint>(sampler), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLuint *>(params)));
}
void getTexParameterI(TextureTarget target, GetTextureParameter pname, int32_t *params)
{
    return glGetTexParameterIiv(static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint *>(params)));
}
void getTexParameterI(TextureTarget target, GetTextureParameter pname, uint32_t *params)
{
    return glGetTexParameterIuiv(static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLuint *>(params)));
}
void getnUniform(uint32_t program, int32_t location, core::SizeType bufSize, float *params)
{
    return glGetnUniformfv(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLsizei>(bufSize), detail::constAway(reinterpret_cast<GLfloat *>(params)));
}
void getnUniform(uint32_t program, int32_t location, core::SizeType bufSize, int32_t *params)
{
    return glGetnUniformiv(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLsizei>(bufSize), detail::constAway(reinterpret_cast<GLint *>(params)));
}
void getnUniform(uint32_t program, int32_t location, core::SizeType bufSize, uint32_t *params)
{
    return glGetnUniformuiv(static_cast<GLuint>(program), static_cast<GLint>(location), static_cast<GLsizei>(bufSize), detail::constAway(reinterpret_cast<GLuint *>(params)));
}
bool isEnabled(EnableCap target, uint32_t index)
{
    return static_cast<bool >(glIsEnabledi(static_cast<GLenum>(target), static_cast<GLuint>(index)));
}
void minSampleShading(float value)
{
    return glMinSampleShading(static_cast<GLfloat>(value));
}
void objectLabel(ObjectIdentifier identifier, uint32_t name, core::SizeType length, const char *label)
{
    return glObjectLabel(static_cast<GLenum>(identifier), static_cast<GLuint>(name), static_cast<GLsizei>(length), detail::constAway(reinterpret_cast<const GLchar *>(label)));
}
void objectPtrLabel(const void *ptr, core::SizeType length, const char *label)
{
    return glObjectPtrLabel(detail::constAway(ptr), static_cast<GLsizei>(length), detail::constAway(reinterpret_cast<const GLchar *>(label)));
}
void patchParameter(PatchParameterName pname, int32_t value)
{
    return glPatchParameteri(static_cast<GLenum>(pname), static_cast<GLint>(value));
}
void popDebugGroup()
{
    return glPopDebugGroup();
}
void primitiveBoundingBox(float minX, float minY, float minZ, float minW, float maxX, float maxY, float maxZ, float maxW)
{
    return glPrimitiveBoundingBox(static_cast<GLfloat>(minX), static_cast<GLfloat>(minY), static_cast<GLfloat>(minZ), static_cast<GLfloat>(minW), static_cast<GLfloat>(maxX), static_cast<GLfloat>(maxY), static_cast<GLfloat>(maxZ), static_cast<GLfloat>(maxW));
}
void pushDebugGroup(DebugSource source, uint32_t id, core::SizeType length, const char *message)
{
    return glPushDebugGroup(static_cast<GLenum>(source), static_cast<GLuint>(id), static_cast<GLsizei>(length), detail::constAway(reinterpret_cast<const GLchar *>(message)));
}
void readnPixel(int32_t x, int32_t y, core::SizeType width, core::SizeType height, PixelFormat format, PixelType type, core::SizeType bufSize, void *data)
{
    return glReadnPixels(static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(width), static_cast<GLsizei>(height), static_cast<GLenum>(format), static_cast<GLenum>(type), static_cast<GLsizei>(bufSize), detail::constAway(data));
}
void samplerParameterI(uint32_t sampler, SamplerParameterI pname, const int32_t *param)
{
    return glSamplerParameterIiv(static_cast<GLuint>(sampler), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<const GLint *>(param)));
}
void samplerParameterI(uint32_t sampler, SamplerParameterI pname, const uint32_t *param)
{
    return glSamplerParameterIuiv(static_cast<GLuint>(sampler), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<const GLuint *>(param)));
}
void texBuffer(TextureTarget target, InternalFormat internalformat, uint32_t buffer)
{
    return glTexBuffer(static_cast<GLenum>(target), static_cast<GLenum>(internalformat), static_cast<GLuint>(buffer));
}
void texBufferRange(TextureTarget target, InternalFormat internalformat, uint32_t buffer, std::intptr_t offset, std::size_t size)
{
    return glTexBufferRange(static_cast<GLenum>(target), static_cast<GLenum>(internalformat), static_cast<GLuint>(buffer), static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size));
}
void texParameterI(TextureTarget target, TextureParameterName pname, const int32_t *params)
{
    return glTexParameterIiv(static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<const GLint *>(params)));
}
void texParameterI(TextureTarget target, TextureParameterName pname, const uint32_t *params)
{
    return glTexParameterIuiv(static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<const GLuint *>(params)));
}
void texStorage3DMultisample(TextureTarget target, core::SizeType samples, InternalFormat internalformat, core::SizeType width, core::SizeType height, core::SizeType depth, bool fixedsamplelocations)
{
    return glTexStorage3DMultisample(static_cast<GLenum>(target), static_cast<GLsizei>(samples), static_cast<GLenum>(internalformat), static_cast<GLsizei>(width), static_cast<GLsizei>(height), static_cast<GLsizei>(depth), static_cast<GLboolean>(fixedsamplelocations));
}
#endif
}
