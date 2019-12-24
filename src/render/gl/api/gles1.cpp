#include "gles1.hpp"
#include "gles1_api_provider.hpp"
namespace gles1
{
void activeTexture(TextureUnit texture)
{
  return glActiveTexture(static_cast<GLenum>(texture));
}
void alphaFunc(AlphaFunction func, float ref)
{
  return glAlphaFunc(static_cast<GLenum>(func), static_cast<GLfloat>(ref));
}
void alphaFuncx(AlphaFunction func, core::Fixed ref)
{
  return glAlphaFuncx(static_cast<GLenum>(func), static_cast<GLfixed>(ref));
}
void bindBuffer(BufferTargetARB target, uint32_t buffer)
{
  return glBindBuffer(static_cast<GLenum>(target), static_cast<GLuint>(buffer));
}
void bindTexture(TextureTarget target, uint32_t texture)
{
  return glBindTexture(static_cast<GLenum>(target), static_cast<GLuint>(texture));
}
void blendFunc(BlendingFactor sfactor, BlendingFactor dfactor)
{
  return glBlendFunc(static_cast<GLenum>(sfactor), static_cast<GLenum>(dfactor));
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
void clear(core::Bitfield<ClearBufferMask> mask)
{
  return glClear(mask.value());
}
void clearColor(float red, float green, float blue, float alpha)
{
  return glClearColor(
    static_cast<GLfloat>(red), static_cast<GLfloat>(green), static_cast<GLfloat>(blue), static_cast<GLfloat>(alpha));
}
void clearColorx(core::Fixed red, core::Fixed green, core::Fixed blue, core::Fixed alpha)
{
  return glClearColorx(
    static_cast<GLfixed>(red), static_cast<GLfixed>(green), static_cast<GLfixed>(blue), static_cast<GLfixed>(alpha));
}
void clearDepth(float d)
{
  return glClearDepthf(static_cast<GLfloat>(d));
}
void clearDepthx(core::Fixed depth)
{
  return glClearDepthx(static_cast<GLfixed>(depth));
}
void clearStencil(int32_t s)
{
  return glClearStencil(static_cast<GLint>(s));
}
void clientActiveTexture(TextureUnit texture)
{
  return glClientActiveTexture(static_cast<GLenum>(texture));
}
void clipPlane(ClipPlaneName p, const float* eqn)
{
  return glClipPlanef(static_cast<GLenum>(p), detail::constAway(reinterpret_cast<const GLfloat*>(eqn)));
}
void clipPlanex(ClipPlaneName plane, const core::Fixed* equation)
{
  return glClipPlanex(static_cast<GLenum>(plane), detail::constAway(reinterpret_cast<const GLfixed*>(equation)));
}
void color4(float red, float green, float blue, float alpha)
{
  return glColor4f(
    static_cast<GLfloat>(red), static_cast<GLfloat>(green), static_cast<GLfloat>(blue), static_cast<GLfloat>(alpha));
}
void color4(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
  return glColor4ub(
    static_cast<GLubyte>(red), static_cast<GLubyte>(green), static_cast<GLubyte>(blue), static_cast<GLubyte>(alpha));
}
void color4x(core::Fixed red, core::Fixed green, core::Fixed blue, core::Fixed alpha)
{
  return glColor4x(
    static_cast<GLfixed>(red), static_cast<GLfixed>(green), static_cast<GLfixed>(blue), static_cast<GLfixed>(alpha));
}
void colorMask(bool red, bool green, bool blue, bool alpha)
{
  return glColorMask(static_cast<GLboolean>(red),
                     static_cast<GLboolean>(green),
                     static_cast<GLboolean>(blue),
                     static_cast<GLboolean>(alpha));
}
void colorPointer(int32_t size, ColorPointerType type, core::SizeType stride, const void* pointer)
{
  return glColorPointer(
    static_cast<GLint>(size), static_cast<GLenum>(type), static_cast<GLsizei>(stride), detail::constAway(pointer));
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
void cullFace(CullFaceMode mode)
{
  return glCullFace(static_cast<GLenum>(mode));
}
void deleteBuffers(core::SizeType n, const uint32_t* buffers)
{
  return glDeleteBuffers(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<const GLuint*>(buffers)));
}
void deleteTextures(core::SizeType n, const uint32_t* textures)
{
  return glDeleteTextures(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<const GLuint*>(textures)));
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
void depthRangex(core::Fixed n, core::Fixed f)
{
  return glDepthRangex(static_cast<GLfixed>(n), static_cast<GLfixed>(f));
}
void disable(EnableCap cap)
{
  return glDisable(static_cast<GLenum>(cap));
}
void disableClientState(EnableCap array)
{
  return glDisableClientState(static_cast<GLenum>(array));
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
void enable(EnableCap cap)
{
  return glEnable(static_cast<GLenum>(cap));
}
void enableClientState(EnableCap array)
{
  return glEnableClientState(static_cast<GLenum>(array));
}
void finish()
{
  return glFinish();
}
void flush()
{
  return glFlush();
}
void fog(FogParameter pname, float param)
{
  return glFogf(static_cast<GLenum>(pname), static_cast<GLfloat>(param));
}
void fog(FogParameter pname, const float* params)
{
  return glFogfv(static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<const GLfloat*>(params)));
}
void fogx(FogPName pname, core::Fixed param)
{
  return glFogx(static_cast<GLenum>(pname), static_cast<GLfixed>(param));
}
void fogxv(FogPName pname, const core::Fixed* param)
{
  return glFogxv(static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<const GLfixed*>(param)));
}
void frontFace(FrontFaceDirection mode)
{
  return glFrontFace(static_cast<GLenum>(mode));
}
void frustum(float l, float r, float b, float t, float n, float f)
{
  return glFrustumf(static_cast<GLfloat>(l),
                    static_cast<GLfloat>(r),
                    static_cast<GLfloat>(b),
                    static_cast<GLfloat>(t),
                    static_cast<GLfloat>(n),
                    static_cast<GLfloat>(f));
}
void frustumx(core::Fixed l, core::Fixed r, core::Fixed b, core::Fixed t, core::Fixed n, core::Fixed f)
{
  return glFrustumx(static_cast<GLfixed>(l),
                    static_cast<GLfixed>(r),
                    static_cast<GLfixed>(b),
                    static_cast<GLfixed>(t),
                    static_cast<GLfixed>(n),
                    static_cast<GLfixed>(f));
}
void genBuffers(core::SizeType n, uint32_t* buffers)
{
  return glGenBuffers(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<GLuint*>(buffers)));
}
void genTextures(core::SizeType n, uint32_t* textures)
{
  return glGenTextures(static_cast<GLsizei>(n), detail::constAway(reinterpret_cast<GLuint*>(textures)));
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
void getClipPlane(ClipPlaneName plane, float* equation)
{
  return glGetClipPlanef(static_cast<GLenum>(plane), detail::constAway(reinterpret_cast<GLfloat*>(equation)));
}
void getClipPlanex(ClipPlaneName plane, core::Fixed* equation)
{
  return glGetClipPlanex(static_cast<GLenum>(plane), detail::constAway(reinterpret_cast<GLfixed*>(equation)));
}
ErrorCode getError()
{
  return static_cast<ErrorCode>(glGetError());
}
void getFixe(GetPName pname, core::Fixed* params)
{
  return glGetFixedv(static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLfixed*>(params)));
}
void getFloatv(GetPName pname, float* data)
{
  return glGetFloatv(static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLfloat*>(data)));
}
void getIntegerv(GetPName pname, int32_t* data)
{
  return glGetIntegerv(static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint*>(data)));
}
void getLight(LightName light, LightParameter pname, float* params)
{
  return glGetLightfv(
    static_cast<GLenum>(light), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLfloat*>(params)));
}
void getLightxv(LightName light, LightParameter pname, core::Fixed* params)
{
  return glGetLightxv(
    static_cast<GLenum>(light), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLfixed*>(params)));
}
void getMaterial(MaterialFace face, MaterialParameter pname, float* params)
{
  return glGetMaterialfv(
    static_cast<GLenum>(face), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLfloat*>(params)));
}
void getMaterialxv(MaterialFace face, MaterialParameter pname, core::Fixed* params)
{
  return glGetMaterialxv(
    static_cast<GLenum>(face), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLfixed*>(params)));
}
void getPointerv(GetPointervPName pname, void** params)
{
  return glGetPointerv(static_cast<GLenum>(pname), detail::constAway(params));
}
const uint8_t* getString(StringName name)
{
  return static_cast<const uint8_t*>(glGetString(static_cast<GLenum>(name)));
}
void getTexEnv(TextureEnvTarget target, TextureEnvParameter pname, float* params)
{
  return glGetTexEnvfv(
    static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLfloat*>(params)));
}
void getTexEnv(TextureEnvTarget target, TextureEnvParameter pname, int32_t* params)
{
  return glGetTexEnviv(
    static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLint*>(params)));
}
void getTexEnvxv(TextureEnvTarget target, TextureEnvParameter pname, core::Fixed* params)
{
  return glGetTexEnvxv(
    static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLfixed*>(params)));
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
void getTexParameterxv(TextureTarget target, GetTextureParameter pname, core::Fixed* params)
{
  return glGetTexParameterxv(
    static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<GLfixed*>(params)));
}
void hint(HintTarget target, HintMode mode)
{
  return glHint(static_cast<GLenum>(target), static_cast<GLenum>(mode));
}
bool isBuffer(uint32_t buffer)
{
  return static_cast<bool>(glIsBuffer(static_cast<GLuint>(buffer)));
}
bool isEnable(EnableCap cap)
{
  return static_cast<bool>(glIsEnabled(static_cast<GLenum>(cap)));
}
bool isTexture(uint32_t texture)
{
  return static_cast<bool>(glIsTexture(static_cast<GLuint>(texture)));
}
void lightModel(LightModelParameter pname, float param)
{
  return glLightModelf(static_cast<GLenum>(pname), static_cast<GLfloat>(param));
}
void lightModel(LightModelParameter pname, const float* params)
{
  return glLightModelfv(static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<const GLfloat*>(params)));
}
void lightModelx(LightModelParameter pname, core::Fixed param)
{
  return glLightModelx(static_cast<GLenum>(pname), static_cast<GLfixed>(param));
}
void lightModelxv(LightModelParameter pname, const core::Fixed* param)
{
  return glLightModelxv(static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<const GLfixed*>(param)));
}
void light(LightName light, LightParameter pname, float param)
{
  return glLightf(static_cast<GLenum>(light), static_cast<GLenum>(pname), static_cast<GLfloat>(param));
}
void light(LightName light, LightParameter pname, const float* params)
{
  return glLightfv(static_cast<GLenum>(light),
                   static_cast<GLenum>(pname),
                   detail::constAway(reinterpret_cast<const GLfloat*>(params)));
}
void lightx(LightName light, LightParameter pname, core::Fixed param)
{
  return glLightx(static_cast<GLenum>(light), static_cast<GLenum>(pname), static_cast<GLfixed>(param));
}
void lightxv(LightName light, LightParameter pname, const core::Fixed* params)
{
  return glLightxv(static_cast<GLenum>(light),
                   static_cast<GLenum>(pname),
                   detail::constAway(reinterpret_cast<const GLfixed*>(params)));
}
void lineWidth(float width)
{
  return glLineWidth(static_cast<GLfloat>(width));
}
void lineWidthx(core::Fixed width)
{
  return glLineWidthx(static_cast<GLfixed>(width));
}
void loadIdentity()
{
  return glLoadIdentity();
}
void loadMatrix(const float* m)
{
  return glLoadMatrixf(detail::constAway(reinterpret_cast<const GLfloat*>(m)));
}
void loadMatrixx(const core::Fixed* m)
{
  return glLoadMatrixx(detail::constAway(reinterpret_cast<const GLfixed*>(m)));
}
void logicOp(LogicOp opcode)
{
  return glLogicOp(static_cast<GLenum>(opcode));
}
void material(MaterialFace face, MaterialParameter pname, float param)
{
  return glMaterialf(static_cast<GLenum>(face), static_cast<GLenum>(pname), static_cast<GLfloat>(param));
}
void material(MaterialFace face, MaterialParameter pname, const float* params)
{
  return glMaterialfv(
    static_cast<GLenum>(face), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<const GLfloat*>(params)));
}
void materialx(MaterialFace face, MaterialParameter pname, core::Fixed param)
{
  return glMaterialx(static_cast<GLenum>(face), static_cast<GLenum>(pname), static_cast<GLfixed>(param));
}
void materialxv(MaterialFace face, MaterialParameter pname, const core::Fixed* param)
{
  return glMaterialxv(
    static_cast<GLenum>(face), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<const GLfixed*>(param)));
}
void matrixMode(MatrixMode mode)
{
  return glMatrixMode(static_cast<GLenum>(mode));
}
void multMatrix(const float* m)
{
  return glMultMatrixf(detail::constAway(reinterpret_cast<const GLfloat*>(m)));
}
void multMatrixx(const core::Fixed* m)
{
  return glMultMatrixx(detail::constAway(reinterpret_cast<const GLfixed*>(m)));
}
void multiTexCoord4(TextureUnit target, float s, float t, float r, float q)
{
  return glMultiTexCoord4f(static_cast<GLenum>(target),
                           static_cast<GLfloat>(s),
                           static_cast<GLfloat>(t),
                           static_cast<GLfloat>(r),
                           static_cast<GLfloat>(q));
}
void multiTexCoord4x(TextureUnit texture, core::Fixed s, core::Fixed t, core::Fixed r, core::Fixed q)
{
  return glMultiTexCoord4x(static_cast<GLenum>(texture),
                           static_cast<GLfixed>(s),
                           static_cast<GLfixed>(t),
                           static_cast<GLfixed>(r),
                           static_cast<GLfixed>(q));
}
void normal3(float nx, float ny, float nz)
{
  return glNormal3f(static_cast<GLfloat>(nx), static_cast<GLfloat>(ny), static_cast<GLfloat>(nz));
}
void normal3x(core::Fixed nx, core::Fixed ny, core::Fixed nz)
{
  return glNormal3x(static_cast<GLfixed>(nx), static_cast<GLfixed>(ny), static_cast<GLfixed>(nz));
}
void normalPointer(NormalPointerType type, core::SizeType stride, const void* pointer)
{
  return glNormalPointer(static_cast<GLenum>(type), static_cast<GLsizei>(stride), detail::constAway(pointer));
}
void ortho(float l, float r, float b, float t, float n, float f)
{
  return glOrthof(static_cast<GLfloat>(l),
                  static_cast<GLfloat>(r),
                  static_cast<GLfloat>(b),
                  static_cast<GLfloat>(t),
                  static_cast<GLfloat>(n),
                  static_cast<GLfloat>(f));
}
void orthox(core::Fixed l, core::Fixed r, core::Fixed b, core::Fixed t, core::Fixed n, core::Fixed f)
{
  return glOrthox(static_cast<GLfixed>(l),
                  static_cast<GLfixed>(r),
                  static_cast<GLfixed>(b),
                  static_cast<GLfixed>(t),
                  static_cast<GLfixed>(n),
                  static_cast<GLfixed>(f));
}
void pixelStore(PixelStoreParameter pname, int32_t param)
{
  return glPixelStorei(static_cast<GLenum>(pname), static_cast<GLint>(param));
}
void pointParameter(PointParameterNameARB pname, float param)
{
  return glPointParameterf(static_cast<GLenum>(pname), static_cast<GLfloat>(param));
}
void pointParameter(PointParameterNameARB pname, const float* params)
{
  return glPointParameterfv(static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<const GLfloat*>(params)));
}
void pointParameterx(PointParameterNameARB pname, core::Fixed param)
{
  return glPointParameterx(static_cast<GLenum>(pname), static_cast<GLfixed>(param));
}
void pointParameterxv(PointParameterNameARB pname, const core::Fixed* params)
{
  return glPointParameterxv(static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<const GLfixed*>(params)));
}
void pointSize(float size)
{
  return glPointSize(static_cast<GLfloat>(size));
}
void pointSizex(core::Fixed size)
{
  return glPointSizex(static_cast<GLfixed>(size));
}
void polygonOffset(float factor, float units)
{
  return glPolygonOffset(static_cast<GLfloat>(factor), static_cast<GLfloat>(units));
}
void polygonOffsetx(core::Fixed factor, core::Fixed units)
{
  return glPolygonOffsetx(static_cast<GLfixed>(factor), static_cast<GLfixed>(units));
}
void popMatrix()
{
  return glPopMatrix();
}
void pushMatrix()
{
  return glPushMatrix();
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
void rotate(float angle, float x, float y, float z)
{
  return glRotatef(
    static_cast<GLfloat>(angle), static_cast<GLfloat>(x), static_cast<GLfloat>(y), static_cast<GLfloat>(z));
}
void rotatex(core::Fixed angle, core::Fixed x, core::Fixed y, core::Fixed z)
{
  return glRotatex(
    static_cast<GLfixed>(angle), static_cast<GLfixed>(x), static_cast<GLfixed>(y), static_cast<GLfixed>(z));
}
void sampleCoverage(float value, bool invert)
{
  return glSampleCoverage(static_cast<GLfloat>(value), static_cast<GLboolean>(invert));
}
void sampleCoveragex(GLclampx value, bool invert)
{
  return glSampleCoveragex(value, static_cast<GLboolean>(invert));
}
void scale(float x, float y, float z)
{
  return glScalef(static_cast<GLfloat>(x), static_cast<GLfloat>(y), static_cast<GLfloat>(z));
}
void scalex(core::Fixed x, core::Fixed y, core::Fixed z)
{
  return glScalex(static_cast<GLfixed>(x), static_cast<GLfixed>(y), static_cast<GLfixed>(z));
}
void scissor(int32_t x, int32_t y, core::SizeType width, core::SizeType height)
{
  return glScissor(
    static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(width), static_cast<GLsizei>(height));
}
void shadeModel(ShadingModel mode)
{
  return glShadeModel(static_cast<GLenum>(mode));
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
void texCoordPointer(int32_t size, TexCoordPointerType type, core::SizeType stride, const void* pointer)
{
  return glTexCoordPointer(
    static_cast<GLint>(size), static_cast<GLenum>(type), static_cast<GLsizei>(stride), detail::constAway(pointer));
}
void texEnv(TextureEnvTarget target, TextureEnvParameter pname, float param)
{
  return glTexEnvf(static_cast<GLenum>(target), static_cast<GLenum>(pname), static_cast<GLfloat>(param));
}
void texEnv(TextureEnvTarget target, TextureEnvParameter pname, const float* params)
{
  return glTexEnvfv(static_cast<GLenum>(target),
                    static_cast<GLenum>(pname),
                    detail::constAway(reinterpret_cast<const GLfloat*>(params)));
}
void texEnv(TextureEnvTarget target, TextureEnvParameter pname, int32_t param)
{
  return glTexEnvi(static_cast<GLenum>(target), static_cast<GLenum>(pname), static_cast<GLint>(param));
}
void texEnv(TextureEnvTarget target, TextureEnvParameter pname, const int32_t* params)
{
  return glTexEnviv(
    static_cast<GLenum>(target), static_cast<GLenum>(pname), detail::constAway(reinterpret_cast<const GLint*>(params)));
}
void texEnvx(TextureEnvTarget target, TextureEnvParameter pname, core::Fixed param)
{
  return glTexEnvx(static_cast<GLenum>(target), static_cast<GLenum>(pname), static_cast<GLfixed>(param));
}
void texEnvxv(TextureEnvTarget target, TextureEnvParameter pname, const core::Fixed* params)
{
  return glTexEnvxv(static_cast<GLenum>(target),
                    static_cast<GLenum>(pname),
                    detail::constAway(reinterpret_cast<const GLfixed*>(params)));
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
void texParameterx(TextureTarget target, GetTextureParameter pname, core::Fixed param)
{
  return glTexParameterx(static_cast<GLenum>(target), static_cast<GLenum>(pname), static_cast<GLfixed>(param));
}
void texParameterxv(TextureTarget target, GetTextureParameter pname, const core::Fixed* params)
{
  return glTexParameterxv(static_cast<GLenum>(target),
                          static_cast<GLenum>(pname),
                          detail::constAway(reinterpret_cast<const GLfixed*>(params)));
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
void translate(float x, float y, float z)
{
  return glTranslatef(static_cast<GLfloat>(x), static_cast<GLfloat>(y), static_cast<GLfloat>(z));
}
void translatex(core::Fixed x, core::Fixed y, core::Fixed z)
{
  return glTranslatex(static_cast<GLfixed>(x), static_cast<GLfixed>(y), static_cast<GLfixed>(z));
}
void vertexPointer(int32_t size, VertexPointerType type, core::SizeType stride, const void* pointer)
{
  return glVertexPointer(
    static_cast<GLint>(size), static_cast<GLenum>(type), static_cast<GLsizei>(stride), detail::constAway(pointer));
}
void viewport(int32_t x, int32_t y, core::SizeType width, core::SizeType height)
{
  return glViewport(
    static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(width), static_cast<GLsizei>(height));
}
} // namespace gles1
