These are the manually applied patches to allow running in [RenderDoc]().

## `load_GL_ARB_bindless_texture`

```c
static int load_GL_ARB_bindless_texture(GLADloadproc load)
{
  glad_glGetTextureHandleARB = (PFNGLGETTEXTUREHANDLEARBPROC)load("glGetTextureHandleARB");
  if(glad_glGetTextureHandleARB == NULL)
    goto fail;
  glad_glGetTextureSamplerHandleARB = (PFNGLGETTEXTURESAMPLERHANDLEARBPROC)load("glGetTextureSamplerHandleARB");
  if(glad_glGetTextureSamplerHandleARB == NULL)
    goto fail;
  glad_glMakeTextureHandleResidentARB = (PFNGLMAKETEXTUREHANDLERESIDENTARBPROC)load("glMakeTextureHandleResidentARB");
  if(glad_glMakeTextureHandleResidentARB == NULL)
    goto fail;
  glad_glMakeTextureHandleNonResidentARB
    = (PFNGLMAKETEXTUREHANDLENONRESIDENTARBPROC)load("glMakeTextureHandleNonResidentARB");
  if(glad_glMakeTextureHandleNonResidentARB == NULL)
    goto fail;
  glad_glGetImageHandleARB = (PFNGLGETIMAGEHANDLEARBPROC)load("glGetImageHandleARB");
  if(glad_glGetImageHandleARB == NULL)
    goto fail;
  glad_glMakeImageHandleResidentARB = (PFNGLMAKEIMAGEHANDLERESIDENTARBPROC)load("glMakeImageHandleResidentARB");
  if(glad_glMakeImageHandleResidentARB == NULL)
    goto fail;
  glad_glMakeImageHandleNonResidentARB
    = (PFNGLMAKEIMAGEHANDLENONRESIDENTARBPROC)load("glMakeImageHandleNonResidentARB");
  if(glad_glMakeImageHandleNonResidentARB == NULL)
    goto fail;
  glad_glUniformHandleui64ARB = (PFNGLUNIFORMHANDLEUI64ARBPROC)load("glUniformHandleui64ARB");
  if(glad_glUniformHandleui64ARB == NULL)
    goto fail;
  glad_glUniformHandleui64vARB = (PFNGLUNIFORMHANDLEUI64VARBPROC)load("glUniformHandleui64vARB");
  if(glad_glUniformHandleui64vARB == NULL)
    goto fail;
  glad_glProgramUniformHandleui64ARB = (PFNGLPROGRAMUNIFORMHANDLEUI64ARBPROC)load("glProgramUniformHandleui64ARB");
  if(glad_glProgramUniformHandleui64ARB == NULL)
    goto fail;
  glad_glProgramUniformHandleui64vARB = (PFNGLPROGRAMUNIFORMHANDLEUI64VARBPROC)load("glProgramUniformHandleui64vARB");
  if(glad_glProgramUniformHandleui64vARB == NULL)
    goto fail;
  glad_glIsTextureHandleResidentARB = (PFNGLISTEXTUREHANDLERESIDENTARBPROC)load("glIsTextureHandleResidentARB");
  if(glad_glIsTextureHandleResidentARB == NULL)
    goto fail;
  glad_glIsImageHandleResidentARB = (PFNGLISIMAGEHANDLERESIDENTARBPROC)load("glIsImageHandleResidentARB");
  if(glad_glIsImageHandleResidentARB == NULL)
    goto fail;
  glad_glVertexAttribL1ui64ARB = (PFNGLVERTEXATTRIBL1UI64ARBPROC)load("glVertexAttribL1ui64ARB");
  if(glad_glVertexAttribL1ui64ARB == NULL)
    goto fail;
  glad_glVertexAttribL1ui64vARB = (PFNGLVERTEXATTRIBL1UI64VARBPROC)load("glVertexAttribL1ui64vARB");
  if(glad_glVertexAttribL1ui64vARB == NULL)
    goto fail;
  glad_glGetVertexAttribLui64vARB = (PFNGLGETVERTEXATTRIBLUI64VARBPROC)load("glGetVertexAttribLui64vARB");
  if(glad_glGetVertexAttribLui64vARB == NULL)
    goto fail;
  return 1;

fail:
  fprintf(stderr, "GL_ARB_bindless_texture could not be loaded");
  return 0;
}
```

## `find_extensionsGL`

```c
// ...
GLAD_GL_ARB_bindless_texture = 1; // has_ext("GL_ARB_bindless_texture");
// ...
```

## `gladLoadGLLoader`

```c
// ...
if(!load_GL_ARB_bindless_texture(load))
  return 0;
```
