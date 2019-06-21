#pragma once
#include "soglb_core.hpp"
#include "gles2_enabled_features.hpp"
namespace gles2
{
// API feature levels
// #define API_LEVEL_GL_ES_VERSION_2_0
// #define API_LEVEL_GL_ES_VERSION_3_0
// #define API_LEVEL_GL_ES_VERSION_3_1
// #define API_LEVEL_GL_ES_VERSION_3_2

// API extensions
// #define WITH_API_EXTENSION_GL_AMD_compressed_3DC_texture
// #define WITH_API_EXTENSION_GL_AMD_compressed_ATC_texture
// #define WITH_API_EXTENSION_GL_AMD_framebuffer_multisample_advanced
// #define WITH_API_EXTENSION_GL_AMD_performance_monitor
// #define WITH_API_EXTENSION_GL_AMD_program_binary_Z400
// #define WITH_API_EXTENSION_GL_ANGLE_depth_texture
// #define WITH_API_EXTENSION_GL_ANGLE_framebuffer_blit
// #define WITH_API_EXTENSION_GL_ANGLE_framebuffer_multisample
// #define WITH_API_EXTENSION_GL_ANGLE_instanced_arrays
// #define WITH_API_EXTENSION_GL_ANGLE_pack_reverse_row_order
// #define WITH_API_EXTENSION_GL_ANGLE_program_binary
// #define WITH_API_EXTENSION_GL_ANGLE_texture_compression_dxt3
// #define WITH_API_EXTENSION_GL_ANGLE_texture_compression_dxt5
// #define WITH_API_EXTENSION_GL_ANGLE_texture_usage
// #define WITH_API_EXTENSION_GL_ANGLE_translated_shader_source
// #define WITH_API_EXTENSION_GL_APPLE_clip_distance
// #define WITH_API_EXTENSION_GL_APPLE_copy_texture_levels
// #define WITH_API_EXTENSION_GL_APPLE_framebuffer_multisample
// #define WITH_API_EXTENSION_GL_APPLE_rgb_422
// #define WITH_API_EXTENSION_GL_APPLE_sync
// #define WITH_API_EXTENSION_GL_APPLE_texture_format_BGRA8888
// #define WITH_API_EXTENSION_GL_APPLE_texture_max_level
// #define WITH_API_EXTENSION_GL_APPLE_texture_packed_float
// #define WITH_API_EXTENSION_GL_ARM_mali_program_binary
// #define WITH_API_EXTENSION_GL_ARM_mali_shader_binary
// #define WITH_API_EXTENSION_GL_ARM_shader_framebuffer_fetch
// #define WITH_API_EXTENSION_GL_DMP_program_binary
// #define WITH_API_EXTENSION_GL_DMP_shader_binary
// #define WITH_API_EXTENSION_GL_EXT_EGL_image_storage
// #define WITH_API_EXTENSION_GL_EXT_YUV_target
// #define WITH_API_EXTENSION_GL_EXT_base_instance
// #define WITH_API_EXTENSION_GL_EXT_blend_func_extended
// #define WITH_API_EXTENSION_GL_EXT_blend_minmax
// #define WITH_API_EXTENSION_GL_EXT_buffer_storage
// #define WITH_API_EXTENSION_GL_EXT_clear_texture
// #define WITH_API_EXTENSION_GL_EXT_clip_control
// #define WITH_API_EXTENSION_GL_EXT_clip_cull_distance
// #define WITH_API_EXTENSION_GL_EXT_color_buffer_half_float
// #define WITH_API_EXTENSION_GL_EXT_copy_image
// #define WITH_API_EXTENSION_GL_EXT_debug_label
// #define WITH_API_EXTENSION_GL_EXT_debug_marker
// #define WITH_API_EXTENSION_GL_EXT_depth_clamp
// #define WITH_API_EXTENSION_GL_EXT_discard_framebuffer
// #define WITH_API_EXTENSION_GL_EXT_disjoint_timer_query
// #define WITH_API_EXTENSION_GL_EXT_draw_buffers
// #define WITH_API_EXTENSION_GL_EXT_draw_buffers_indexed
// #define WITH_API_EXTENSION_GL_EXT_draw_elements_base_vertex
// #define WITH_API_EXTENSION_GL_EXT_draw_instanced
// #define WITH_API_EXTENSION_GL_EXT_draw_transform_feedback
// #define WITH_API_EXTENSION_GL_EXT_external_buffer
// #define WITH_API_EXTENSION_GL_EXT_geometry_shader
// #define WITH_API_EXTENSION_GL_EXT_instanced_arrays
// #define WITH_API_EXTENSION_GL_EXT_map_buffer_range
// #define WITH_API_EXTENSION_GL_EXT_memory_object
// #define WITH_API_EXTENSION_GL_EXT_memory_object_fd
// #define WITH_API_EXTENSION_GL_EXT_memory_object_win32
// #define WITH_API_EXTENSION_GL_EXT_multi_draw_arrays
// #define WITH_API_EXTENSION_GL_EXT_multi_draw_indirect
// #define WITH_API_EXTENSION_GL_EXT_multisampled_compatibility
// #define WITH_API_EXTENSION_GL_EXT_multisampled_render_to_texture
// #define WITH_API_EXTENSION_GL_EXT_multiview_draw_buffers
// #define WITH_API_EXTENSION_GL_EXT_occlusion_query_boolean
// #define WITH_API_EXTENSION_GL_EXT_polygon_offset_clamp
// #define WITH_API_EXTENSION_GL_EXT_primitive_bounding_box
// #define WITH_API_EXTENSION_GL_EXT_protected_textures
// #define WITH_API_EXTENSION_GL_EXT_pvrtc_sRGB
// #define WITH_API_EXTENSION_GL_EXT_raster_multisample
// #define WITH_API_EXTENSION_GL_EXT_read_format_bgra
// #define WITH_API_EXTENSION_GL_EXT_render_snorm
// #define WITH_API_EXTENSION_GL_EXT_robustness
// #define WITH_API_EXTENSION_GL_EXT_sRGB
// #define WITH_API_EXTENSION_GL_EXT_sRGB_write_control
// #define WITH_API_EXTENSION_GL_EXT_semaphore
// #define WITH_API_EXTENSION_GL_EXT_semaphore_fd
// #define WITH_API_EXTENSION_GL_EXT_semaphore_win32
// #define WITH_API_EXTENSION_GL_EXT_separate_shader_objects
// #define WITH_API_EXTENSION_GL_EXT_shader_framebuffer_fetch
// #define WITH_API_EXTENSION_GL_EXT_shader_framebuffer_fetch_non_coherent
// #define WITH_API_EXTENSION_GL_EXT_shader_pixel_local_storage
// #define WITH_API_EXTENSION_GL_EXT_shader_pixel_local_storage2
// #define WITH_API_EXTENSION_GL_EXT_shadow_samplers
// #define WITH_API_EXTENSION_GL_EXT_sparse_texture
// #define WITH_API_EXTENSION_GL_EXT_tessellation_shader
// #define WITH_API_EXTENSION_GL_EXT_texture_border_clamp
// #define WITH_API_EXTENSION_GL_EXT_texture_buffer
// #define WITH_API_EXTENSION_GL_EXT_texture_compression_astc_decode_mode
// #define WITH_API_EXTENSION_GL_EXT_texture_compression_bptc
// #define WITH_API_EXTENSION_GL_EXT_texture_compression_dxt1
// #define WITH_API_EXTENSION_GL_EXT_texture_compression_rgtc
// #define WITH_API_EXTENSION_GL_EXT_texture_compression_s3tc
// #define WITH_API_EXTENSION_GL_EXT_texture_compression_s3tc_srgb
// #define WITH_API_EXTENSION_GL_EXT_texture_cube_map_array
// #define WITH_API_EXTENSION_GL_EXT_texture_filter_anisotropic
// #define WITH_API_EXTENSION_GL_EXT_texture_filter_minmax
// #define WITH_API_EXTENSION_GL_EXT_texture_format_BGRA8888
// #define WITH_API_EXTENSION_GL_EXT_texture_format_sRGB_override
// #define WITH_API_EXTENSION_GL_EXT_texture_mirror_clamp_to_edge
// #define WITH_API_EXTENSION_GL_EXT_texture_norm16
// #define WITH_API_EXTENSION_GL_EXT_texture_rg
// #define WITH_API_EXTENSION_GL_EXT_texture_sRGB_R8
// #define WITH_API_EXTENSION_GL_EXT_texture_sRGB_RG8
// #define WITH_API_EXTENSION_GL_EXT_texture_sRGB_decode
// #define WITH_API_EXTENSION_GL_EXT_texture_storage
// #define WITH_API_EXTENSION_GL_EXT_texture_type_2_10_10_10_REV
// #define WITH_API_EXTENSION_GL_EXT_texture_view
// #define WITH_API_EXTENSION_GL_EXT_unpack_subimage
// #define WITH_API_EXTENSION_GL_EXT_win32_keyed_mutex
// #define WITH_API_EXTENSION_GL_EXT_window_rectangles
// #define WITH_API_EXTENSION_GL_FJ_shader_binary_GCCSO
// #define WITH_API_EXTENSION_GL_IMG_bindless_texture
// #define WITH_API_EXTENSION_GL_IMG_framebuffer_downsample
// #define WITH_API_EXTENSION_GL_IMG_multisampled_render_to_texture
// #define WITH_API_EXTENSION_GL_IMG_program_binary
// #define WITH_API_EXTENSION_GL_IMG_read_format
// #define WITH_API_EXTENSION_GL_IMG_shader_binary
// #define WITH_API_EXTENSION_GL_IMG_texture_compression_pvrtc
// #define WITH_API_EXTENSION_GL_IMG_texture_compression_pvrtc2
// #define WITH_API_EXTENSION_GL_IMG_texture_filter_cubic
// #define WITH_API_EXTENSION_GL_INTEL_blackhole_render
// #define WITH_API_EXTENSION_GL_INTEL_conservative_rasterization
// #define WITH_API_EXTENSION_GL_INTEL_framebuffer_CMAA
// #define WITH_API_EXTENSION_GL_INTEL_performance_query
// #define WITH_API_EXTENSION_GL_KHR_blend_equation_advanced
// #define WITH_API_EXTENSION_GL_KHR_blend_equation_advanced_coherent
// #define WITH_API_EXTENSION_GL_KHR_context_flush_control
// #define WITH_API_EXTENSION_GL_KHR_debug
// #define WITH_API_EXTENSION_GL_KHR_no_error
// #define WITH_API_EXTENSION_GL_KHR_parallel_shader_compile
// #define WITH_API_EXTENSION_GL_KHR_robustness
// #define WITH_API_EXTENSION_GL_KHR_texture_compression_astc_hdr
// #define WITH_API_EXTENSION_GL_KHR_texture_compression_astc_ldr
// #define WITH_API_EXTENSION_GL_MESA_framebuffer_flip_y
// #define WITH_API_EXTENSION_GL_MESA_program_binary_formats
// #define WITH_API_EXTENSION_GL_NV_bindless_texture
// #define WITH_API_EXTENSION_GL_NV_blend_equation_advanced
// #define WITH_API_EXTENSION_GL_NV_blend_equation_advanced_coherent
// #define WITH_API_EXTENSION_GL_NV_blend_minmax_factor
// #define WITH_API_EXTENSION_GL_NV_clip_space_w_scaling
// #define WITH_API_EXTENSION_GL_NV_conditional_render
// #define WITH_API_EXTENSION_GL_NV_conservative_raster
// #define WITH_API_EXTENSION_GL_NV_conservative_raster_pre_snap
// #define WITH_API_EXTENSION_GL_NV_conservative_raster_pre_snap_triangles
// #define WITH_API_EXTENSION_GL_NV_copy_buffer
// #define WITH_API_EXTENSION_GL_NV_coverage_sample
// #define WITH_API_EXTENSION_GL_NV_depth_nonlinear
// #define WITH_API_EXTENSION_GL_NV_draw_buffers
// #define WITH_API_EXTENSION_GL_NV_draw_instanced
// #define WITH_API_EXTENSION_GL_NV_draw_vulkan_image
// #define WITH_API_EXTENSION_GL_NV_fbo_color_attachments
// #define WITH_API_EXTENSION_GL_NV_fence
// #define WITH_API_EXTENSION_GL_NV_fill_rectangle
// #define WITH_API_EXTENSION_GL_NV_fragment_coverage_to_color
// #define WITH_API_EXTENSION_GL_NV_framebuffer_blit
// #define WITH_API_EXTENSION_GL_NV_framebuffer_mixed_samples
// #define WITH_API_EXTENSION_GL_NV_framebuffer_multisample
// #define WITH_API_EXTENSION_GL_NV_gpu_shader5
// #define WITH_API_EXTENSION_GL_NV_instanced_arrays
// #define WITH_API_EXTENSION_GL_NV_internalformat_sample_query
// #define WITH_API_EXTENSION_GL_NV_memory_attachment
// #define WITH_API_EXTENSION_GL_NV_mesh_shader
// #define WITH_API_EXTENSION_GL_NV_non_square_matrices
// #define WITH_API_EXTENSION_GL_NV_path_rendering
// #define WITH_API_EXTENSION_GL_NV_path_rendering_shared_edge
// #define WITH_API_EXTENSION_GL_NV_pixel_buffer_object
// #define WITH_API_EXTENSION_GL_NV_polygon_mode
// #define WITH_API_EXTENSION_GL_NV_read_buffer
// #define WITH_API_EXTENSION_GL_NV_representative_fragment_test
// #define WITH_API_EXTENSION_GL_NV_sRGB_formats
// #define WITH_API_EXTENSION_GL_NV_sample_locations
// #define WITH_API_EXTENSION_GL_NV_scissor_exclusive
// #define WITH_API_EXTENSION_GL_NV_shading_rate_image
// #define WITH_API_EXTENSION_GL_NV_shadow_samplers_array
// #define WITH_API_EXTENSION_GL_NV_shadow_samplers_cube
// #define WITH_API_EXTENSION_GL_NV_texture_border_clamp
// #define WITH_API_EXTENSION_GL_NV_viewport_array
// #define WITH_API_EXTENSION_GL_NV_viewport_swizzle
// #define WITH_API_EXTENSION_GL_OES_EGL_image
// #define WITH_API_EXTENSION_GL_OES_EGL_image_external
// #define WITH_API_EXTENSION_GL_OES_compressed_ETC1_RGB8_texture
// #define WITH_API_EXTENSION_GL_OES_compressed_paletted_texture
// #define WITH_API_EXTENSION_GL_OES_copy_image
// #define WITH_API_EXTENSION_GL_OES_depth24
// #define WITH_API_EXTENSION_GL_OES_depth32
// #define WITH_API_EXTENSION_GL_OES_depth_texture
// #define WITH_API_EXTENSION_GL_OES_draw_buffers_indexed
// #define WITH_API_EXTENSION_GL_OES_draw_elements_base_vertex
// #define WITH_API_EXTENSION_GL_OES_element_index_uint
// #define WITH_API_EXTENSION_GL_OES_geometry_shader
// #define WITH_API_EXTENSION_GL_OES_get_program_binary
// #define WITH_API_EXTENSION_GL_OES_mapbuffer
// #define WITH_API_EXTENSION_GL_OES_packed_depth_stencil
// #define WITH_API_EXTENSION_GL_OES_primitive_bounding_box
// #define WITH_API_EXTENSION_GL_OES_required_internalformat
// #define WITH_API_EXTENSION_GL_OES_rgb8_rgba8
// #define WITH_API_EXTENSION_GL_OES_sample_shading
// #define WITH_API_EXTENSION_GL_OES_shader_multisample_interpolation
// #define WITH_API_EXTENSION_GL_OES_standard_derivatives
// #define WITH_API_EXTENSION_GL_OES_stencil1
// #define WITH_API_EXTENSION_GL_OES_stencil4
// #define WITH_API_EXTENSION_GL_OES_surfaceless_context
// #define WITH_API_EXTENSION_GL_OES_tessellation_shader
// #define WITH_API_EXTENSION_GL_OES_texture_3D
// #define WITH_API_EXTENSION_GL_OES_texture_border_clamp
// #define WITH_API_EXTENSION_GL_OES_texture_buffer
// #define WITH_API_EXTENSION_GL_OES_texture_compression_astc
// #define WITH_API_EXTENSION_GL_OES_texture_cube_map_array
// #define WITH_API_EXTENSION_GL_OES_texture_float
// #define WITH_API_EXTENSION_GL_OES_texture_half_float
// #define WITH_API_EXTENSION_GL_OES_texture_stencil8
// #define WITH_API_EXTENSION_GL_OES_texture_storage_multisample_2d_array
// #define WITH_API_EXTENSION_GL_OES_texture_view
// #define WITH_API_EXTENSION_GL_OES_vertex_array_object
// #define WITH_API_EXTENSION_GL_OES_vertex_half_float
// #define WITH_API_EXTENSION_GL_OES_vertex_type_10_10_10_2
// #define WITH_API_EXTENSION_GL_OES_viewport_array
// #define WITH_API_EXTENSION_GL_OVR_multiview
// #define WITH_API_EXTENSION_GL_OVR_multiview_multisampled_render_to_texture
// #define WITH_API_EXTENSION_GL_QCOM_alpha_test
// #define WITH_API_EXTENSION_GL_QCOM_binning_control
// #define WITH_API_EXTENSION_GL_QCOM_driver_control
// #define WITH_API_EXTENSION_GL_QCOM_extended_get
// #define WITH_API_EXTENSION_GL_QCOM_extended_get2
// #define WITH_API_EXTENSION_GL_QCOM_framebuffer_foveated
// #define WITH_API_EXTENSION_GL_QCOM_perfmon_global_mode
// #define WITH_API_EXTENSION_GL_QCOM_shader_framebuffer_fetch_noncoherent
// #define WITH_API_EXTENSION_GL_QCOM_texture_foveated
// #define WITH_API_EXTENSION_GL_QCOM_texture_foveated_subsampled_layout
// #define WITH_API_EXTENSION_GL_QCOM_tiled_rendering
// #define WITH_API_EXTENSION_GL_QCOM_writeonly_rendering
// #define WITH_API_EXTENSION_GL_VIV_shader_binary

// enums
enum class AlphaFunction : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Always = 0x0207,
    Equal = 0x0202,
    Gequal = 0x0206,
    Greater = 0x0204,
    Lequal = 0x0203,
    Less = 0x0201,
    Never = 0x0200,
    Notequal = 0x0205,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum AlphaFunction

enum class AtomicCounterBufferPName : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    AtomicCounterBufferBinding = 0x92C1,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum AtomicCounterBufferPName

enum class AttribMask : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    ColorBufferBit = 0x00004000,
    DepthBufferBit = 0x00000100,
    StencilBufferBit = 0x00000400,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum AttribMask
constexpr core::Bitfield<AttribMask> operator|(AttribMask left, AttribMask right) { return core::Bitfield<AttribMask>(left) | right;}

enum class AttributeType : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Bool = 0x8B56,
    BoolVec2 = 0x8B57,
    BoolVec3 = 0x8B58,
    BoolVec4 = 0x8B59,
    FloatMat2 = 0x8B5A,
    FloatMat3 = 0x8B5B,
    FloatMat4 = 0x8B5C,
    FloatVec2 = 0x8B50,
    FloatVec3 = 0x8B51,
    FloatVec4 = 0x8B52,
    IntVec2 = 0x8B53,
    IntVec3 = 0x8B54,
    IntVec4 = 0x8B55,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    FloatMat2x3 = 0x8B65,
    FloatMat2x4 = 0x8B66,
    FloatMat3x2 = 0x8B67,
    FloatMat3x4 = 0x8B68,
    FloatMat4x2 = 0x8B69,
    FloatMat4x3 = 0x8B6A,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum AttributeType

enum class BindTransformFeedbackTarget : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    TransformFeedback = 0x8E22,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum BindTransformFeedbackTarget

enum class BlendEquationModeEXT : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    FuncAdd = 0x8006,
    FuncReverseSubtract = 0x800B,
    FuncSubtract = 0x800A,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Max = 0x8008,
    Min = 0x8007,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum BlendEquationModeEXT

enum class BlendingFactor : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    ConstantAlpha = 0x8003,
    ConstantColor = 0x8001,
    DstAlpha = 0x0304,
    DstColor = 0x0306,
    One = 1,
    OneMinusConstantAlpha = 0x8004,
    OneMinusConstantColor = 0x8002,
    OneMinusDstAlpha = 0x0305,
    OneMinusDstColor = 0x0307,
    OneMinusSrcAlpha = 0x0303,
    OneMinusSrcColor = 0x0301,
    SrcAlpha = 0x0302,
    SrcAlphaSaturate = 0x0308,
    SrcColor = 0x0300,
    Zero = 0,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum BlendingFactor

enum class BlitFramebufferFilter : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Linear = 0x2601,
    Nearest = 0x2600,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum BlitFramebufferFilter

enum class Boolean : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    False = 0,
    True = 1,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum Boolean

enum class Buffer : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Color = 0x1800,
    Depth = 0x1801,
    Stencil = 0x1802,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum Buffer

enum class BufferAccessARB : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    ReadOnly = 0x88B8,
    ReadWrite = 0x88BA,
    WriteOnly = 0x88B9,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum BufferAccessARB

enum class BufferPNameARB : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    BufferSize = 0x8764,
    BufferUsage = 0x8765,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    BufferAccessFlags = 0x911F,
    BufferMapped = 0x88BC,
    BufferMapLength = 0x9120,
    BufferMapOffset = 0x9121,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum BufferPNameARB

enum class BufferPointerNameARB : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    BufferMapPointer = 0x88BD,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum BufferPointerNameARB

enum class BufferStorageMask : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    MapReadBit = 0x0001,
    MapWriteBit = 0x0002,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum BufferStorageMask
constexpr core::Bitfield<BufferStorageMask> operator|(BufferStorageMask left, BufferStorageMask right) { return core::Bitfield<BufferStorageMask>(left) | right;}

enum class BufferStorageTarget : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    ArrayBuffer = 0x8892,
    ElementArrayBuffer = 0x8893,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    CopyReadBuffer = 0x8F36,
    CopyWriteBuffer = 0x8F37,
    PixelPackBuffer = 0x88EB,
    PixelUnpackBuffer = 0x88EC,
    TransformFeedbackBuffer = 0x8C8E,
    UniformBuffer = 0x8A11,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    AtomicCounterBuffer = 0x92C0,
    DispatchIndirectBuffer = 0x90EE,
    DrawIndirectBuffer = 0x8F3F,
    ShaderStorageBuffer = 0x90D2,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    TextureBuffer = 0x8C2A,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum BufferStorageTarget

enum class BufferTargetARB : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    ArrayBuffer = 0x8892,
    ElementArrayBuffer = 0x8893,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    CopyReadBuffer = 0x8F36,
    CopyWriteBuffer = 0x8F37,
    PixelPackBuffer = 0x88EB,
    PixelUnpackBuffer = 0x88EC,
    TransformFeedbackBuffer = 0x8C8E,
    UniformBuffer = 0x8A11,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    AtomicCounterBuffer = 0x92C0,
    DispatchIndirectBuffer = 0x90EE,
    DrawIndirectBuffer = 0x8F3F,
    ShaderStorageBuffer = 0x90D2,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    TextureBuffer = 0x8C2A,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum BufferTargetARB

enum class BufferUsageARB : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    DynamicDraw = 0x88E8,
    StaticDraw = 0x88E4,
    StreamDraw = 0x88E0,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    DynamicCopy = 0x88EA,
    DynamicRead = 0x88E9,
    StaticCopy = 0x88E6,
    StaticRead = 0x88E5,
    StreamCopy = 0x88E2,
    StreamRead = 0x88E1,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum BufferUsageARB

enum class CheckFramebufferStatusTarget : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Framebuffer = 0x8D40,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    DrawFramebuffer = 0x8CA9,
    ReadFramebuffer = 0x8CA8,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum CheckFramebufferStatusTarget

enum class ClampColorModeARB : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    False = 0,
    True = 1,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum ClampColorModeARB

enum class ClearBufferMask : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    ColorBufferBit = 0x00004000,
    DepthBufferBit = 0x00000100,
    StencilBufferBit = 0x00000400,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum ClearBufferMask
constexpr core::Bitfield<ClearBufferMask> operator|(ClearBufferMask left, ClearBufferMask right) { return core::Bitfield<ClearBufferMask>(left) | right;}

enum class ColorBuffer : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Back = 0x0405,
    ColorAttachment0 = 0x8CE0,
    Front = 0x0404,
    FrontAndBack = 0x0408,
    None = 0,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    ColorAttachment1 = 0x8CE1,
    ColorAttachment10 = 0x8CEA,
    ColorAttachment11 = 0x8CEB,
    ColorAttachment12 = 0x8CEC,
    ColorAttachment13 = 0x8CED,
    ColorAttachment14 = 0x8CEE,
    ColorAttachment15 = 0x8CEF,
    ColorAttachment16 = 0x8CF0,
    ColorAttachment17 = 0x8CF1,
    ColorAttachment18 = 0x8CF2,
    ColorAttachment19 = 0x8CF3,
    ColorAttachment2 = 0x8CE2,
    ColorAttachment20 = 0x8CF4,
    ColorAttachment21 = 0x8CF5,
    ColorAttachment22 = 0x8CF6,
    ColorAttachment23 = 0x8CF7,
    ColorAttachment24 = 0x8CF8,
    ColorAttachment25 = 0x8CF9,
    ColorAttachment26 = 0x8CFA,
    ColorAttachment27 = 0x8CFB,
    ColorAttachment28 = 0x8CFC,
    ColorAttachment29 = 0x8CFD,
    ColorAttachment3 = 0x8CE3,
    ColorAttachment30 = 0x8CFE,
    ColorAttachment31 = 0x8CFF,
    ColorAttachment4 = 0x8CE4,
    ColorAttachment5 = 0x8CE5,
    ColorAttachment6 = 0x8CE6,
    ColorAttachment7 = 0x8CE7,
    ColorAttachment8 = 0x8CE8,
    ColorAttachment9 = 0x8CE9,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum ColorBuffer

enum class ColorMaterialFace : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Back = 0x0405,
    Front = 0x0404,
    FrontAndBack = 0x0408,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum ColorMaterialFace

enum class ColorPointerType : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Byte = 0x1400,
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
    UnsignedByte = 0x1401,
    UnsignedInt = 0x1405,
    UnsignedShort = 0x1403,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum ColorPointerType

enum class CombinerBiasNV : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    None = 0,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum CombinerBiasNV

enum class CombinerScaleNV : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    None = 0,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum CombinerScaleNV

enum class ContextFlagMask : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    ContextFlagDebugBit = 0x00000002,
    ContextFlagRobustAccessBit = 0x00000004,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum ContextFlagMask
constexpr core::Bitfield<ContextFlagMask> operator|(ContextFlagMask left, ContextFlagMask right) { return core::Bitfield<ContextFlagMask>(left) | right;}

enum class CopyBufferSubDataTarget : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    ArrayBuffer = 0x8892,
    ElementArrayBuffer = 0x8893,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    CopyReadBuffer = 0x8F36,
    CopyWriteBuffer = 0x8F37,
    PixelPackBuffer = 0x88EB,
    PixelUnpackBuffer = 0x88EC,
    TransformFeedbackBuffer = 0x8C8E,
    UniformBuffer = 0x8A11,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    AtomicCounterBuffer = 0x92C0,
    DispatchIndirectBuffer = 0x90EE,
    DrawIndirectBuffer = 0x8F3F,
    ShaderStorageBuffer = 0x90D2,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    TextureBuffer = 0x8C2A,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum CopyBufferSubDataTarget

enum class CullFaceMode : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Back = 0x0405,
    Front = 0x0404,
    FrontAndBack = 0x0408,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum CullFaceMode

enum class DebugSeverity : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    DontCare = 0x1100,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    DebugSeverityHigh = 0x9146,
    DebugSeverityLow = 0x9148,
    DebugSeverityMedium = 0x9147,
    DebugSeverityNotification = 0x826B,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum DebugSeverity

enum class DebugSource : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    DontCare = 0x1100,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    DebugSourceApi = 0x8246,
    DebugSourceApplication = 0x824A,
    DebugSourceOther = 0x824B,
    DebugSourceShaderCompiler = 0x8248,
    DebugSourceThirdParty = 0x8249,
    DebugSourceWindowSystem = 0x8247,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum DebugSource

enum class DebugType : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    DontCare = 0x1100,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    DebugTypeDeprecatedBehavior = 0x824D,
    DebugTypeError = 0x824C,
    DebugTypeMarker = 0x8268,
    DebugTypeOther = 0x8251,
    DebugTypePerformance = 0x8250,
    DebugTypePopGroup = 0x826A,
    DebugTypePortability = 0x824F,
    DebugTypePushGroup = 0x8269,
    DebugTypeUndefinedBehavior = 0x824E,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum DebugType

enum class DepthFunction : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Always = 0x0207,
    Equal = 0x0202,
    Gequal = 0x0206,
    Greater = 0x0204,
    Lequal = 0x0203,
    Less = 0x0201,
    Never = 0x0200,
    Notequal = 0x0205,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum DepthFunction

enum class DrawBufferMode : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Back = 0x0405,
    ColorAttachment0 = 0x8CE0,
    Front = 0x0404,
    FrontAndBack = 0x0408,
    None = 0,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    ColorAttachment1 = 0x8CE1,
    ColorAttachment10 = 0x8CEA,
    ColorAttachment11 = 0x8CEB,
    ColorAttachment12 = 0x8CEC,
    ColorAttachment13 = 0x8CED,
    ColorAttachment14 = 0x8CEE,
    ColorAttachment15 = 0x8CEF,
    ColorAttachment16 = 0x8CF0,
    ColorAttachment17 = 0x8CF1,
    ColorAttachment18 = 0x8CF2,
    ColorAttachment19 = 0x8CF3,
    ColorAttachment2 = 0x8CE2,
    ColorAttachment20 = 0x8CF4,
    ColorAttachment21 = 0x8CF5,
    ColorAttachment22 = 0x8CF6,
    ColorAttachment23 = 0x8CF7,
    ColorAttachment24 = 0x8CF8,
    ColorAttachment25 = 0x8CF9,
    ColorAttachment26 = 0x8CFA,
    ColorAttachment27 = 0x8CFB,
    ColorAttachment28 = 0x8CFC,
    ColorAttachment29 = 0x8CFD,
    ColorAttachment3 = 0x8CE3,
    ColorAttachment30 = 0x8CFE,
    ColorAttachment31 = 0x8CFF,
    ColorAttachment4 = 0x8CE4,
    ColorAttachment5 = 0x8CE5,
    ColorAttachment6 = 0x8CE6,
    ColorAttachment7 = 0x8CE7,
    ColorAttachment8 = 0x8CE8,
    ColorAttachment9 = 0x8CE9,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum DrawBufferMode

enum class DrawElementsType : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    UnsignedByte = 0x1401,
    UnsignedInt = 0x1405,
    UnsignedShort = 0x1403,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum DrawElementsType

enum class EnableCap : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Blend = 0x0BE2,
    CullFace = 0x0B44,
    DepthTest = 0x0B71,
    Dither = 0x0BD0,
    PolygonOffsetFill = 0x8037,
    SampleAlphaToCoverage = 0x809E,
    SampleCoverage = 0x80A0,
    ScissorTest = 0x0C11,
    StencilTest = 0x0B90,
    Texture2d = 0x0DE1,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    PrimitiveRestartFixedIndex = 0x8D69,
    RasterizerDiscard = 0x8C89,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    SampleMask = 0x8E51,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    DebugOutput = 0x92E0,
    DebugOutputSynchronous = 0x8242,
    SampleShading = 0x8C36,
    VertexArray = 0x8074,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum EnableCap

enum class ErrorCode : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    InvalidEnum = 0x0500,
    InvalidFramebufferOperation = 0x0506,
    InvalidOperation = 0x0502,
    InvalidValue = 0x0501,
    NoError = 0,
    OutOfMemory = 0x0505,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    StackOverflow = 0x0503,
    StackUnderflow = 0x0504,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum ErrorCode

enum class FogCoordinatePointerType : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Float = 0x1406,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum FogCoordinatePointerType

enum class FogMode : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Linear = 0x2601,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum FogMode

enum class FogPointerTypeEXT : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Float = 0x1406,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum FogPointerTypeEXT

enum class FogPointerTypeIBM : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Float = 0x1406,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum FogPointerTypeIBM

enum class FramebufferAttachment : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    ColorAttachment0 = 0x8CE0,
    DepthAttachment = 0x8D00,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    ColorAttachment1 = 0x8CE1,
    ColorAttachment10 = 0x8CEA,
    ColorAttachment11 = 0x8CEB,
    ColorAttachment12 = 0x8CEC,
    ColorAttachment13 = 0x8CED,
    ColorAttachment14 = 0x8CEE,
    ColorAttachment15 = 0x8CEF,
    ColorAttachment16 = 0x8CF0,
    ColorAttachment17 = 0x8CF1,
    ColorAttachment18 = 0x8CF2,
    ColorAttachment19 = 0x8CF3,
    ColorAttachment2 = 0x8CE2,
    ColorAttachment20 = 0x8CF4,
    ColorAttachment21 = 0x8CF5,
    ColorAttachment22 = 0x8CF6,
    ColorAttachment23 = 0x8CF7,
    ColorAttachment24 = 0x8CF8,
    ColorAttachment25 = 0x8CF9,
    ColorAttachment26 = 0x8CFA,
    ColorAttachment27 = 0x8CFB,
    ColorAttachment28 = 0x8CFC,
    ColorAttachment29 = 0x8CFD,
    ColorAttachment3 = 0x8CE3,
    ColorAttachment30 = 0x8CFE,
    ColorAttachment31 = 0x8CFF,
    ColorAttachment4 = 0x8CE4,
    ColorAttachment5 = 0x8CE5,
    ColorAttachment6 = 0x8CE6,
    ColorAttachment7 = 0x8CE7,
    ColorAttachment8 = 0x8CE8,
    ColorAttachment9 = 0x8CE9,
    DepthStencilAttachment = 0x821A,
    MaxColorAttachments = 0x8CDF,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum FramebufferAttachment

enum class FramebufferAttachmentParameterName : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    FramebufferAttachmentObjectName = 0x8CD1,
    FramebufferAttachmentTextureCubeMapFace = 0x8CD3,
    FramebufferAttachmentTextureLevel = 0x8CD2,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    FramebufferAttachmentAlphaSize = 0x8215,
    FramebufferAttachmentBlueSize = 0x8214,
    FramebufferAttachmentColorEncoding = 0x8210,
    FramebufferAttachmentComponentType = 0x8211,
    FramebufferAttachmentDepthSize = 0x8216,
    FramebufferAttachmentGreenSize = 0x8213,
    FramebufferAttachmentRedSize = 0x8212,
    FramebufferAttachmentStencilSize = 0x8217,
    FramebufferAttachmentTextureLayer = 0x8CD4,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    FramebufferAttachmentLayered = 0x8DA7,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum FramebufferAttachmentParameterName

enum class FramebufferParameterName : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    FramebufferDefaultFixedSampleLocations = 0x9314,
    FramebufferDefaultHeight = 0x9311,
    FramebufferDefaultSamples = 0x9313,
    FramebufferDefaultWidth = 0x9310,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    FramebufferDefaultLayers = 0x9312,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum FramebufferParameterName

enum class FramebufferStatus : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    FramebufferComplete = 0x8CD5,
    FramebufferIncompleteAttachment = 0x8CD6,
    FramebufferIncompleteMissingAttachment = 0x8CD7,
    FramebufferUnsupported = 0x8CDD,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    FramebufferIncompleteMultisample = 0x8D56,
    FramebufferUndefined = 0x8219,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    FramebufferIncompleteLayerTargets = 0x8DA8,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum FramebufferStatus

enum class FramebufferTarget : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Framebuffer = 0x8D40,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    DrawFramebuffer = 0x8CA9,
    ReadFramebuffer = 0x8CA8,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum FramebufferTarget

enum class FrontFaceDirection : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Ccw = 0x0901,
    Cw = 0x0900,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum FrontFaceDirection

enum class GetFramebufferParameter : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    ImplementationColorReadFormat = 0x8B9B,
    ImplementationColorReadType = 0x8B9A,
    Samples = 0x80A9,
    SampleBuffers = 0x80A8,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    FramebufferDefaultFixedSampleLocations = 0x9314,
    FramebufferDefaultHeight = 0x9311,
    FramebufferDefaultSamples = 0x9313,
    FramebufferDefaultWidth = 0x9310,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    FramebufferDefaultLayers = 0x9312,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum GetFramebufferParameter

enum class GetMultisamplePNameNV : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    SamplePosition = 0x8E50,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum GetMultisamplePNameNV

enum class GetPName : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    ActiveTexture = 0x84E0,
    AliasedLineWidthRange = 0x846E,
    AliasedPointSizeRange = 0x846D,
    AlphaBits = 0x0D55,
    ArrayBufferBinding = 0x8894,
    Blend = 0x0BE2,
    BlendColor = 0x8005,
    BlendDstAlpha = 0x80CA,
    BlendDstRgb = 0x80C8,
    BlendEquationAlpha = 0x883D,
    BlendEquationRgb = 0x8009,
    BlendSrcAlpha = 0x80CB,
    BlendSrcRgb = 0x80C9,
    BlueBits = 0x0D54,
    ColorClearValue = 0x0C22,
    ColorWritemask = 0x0C23,
    CompressedTextureFormats = 0x86A3,
    CullFace = 0x0B44,
    CullFaceMode = 0x0B45,
    CurrentProgram = 0x8B8D,
    DepthBits = 0x0D56,
    DepthClearValue = 0x0B73,
    DepthFunc = 0x0B74,
    DepthRange = 0x0B70,
    DepthTest = 0x0B71,
    DepthWritemask = 0x0B72,
    Dither = 0x0BD0,
    ElementArrayBufferBinding = 0x8895,
    FrontFace = 0x0B46,
    GreenBits = 0x0D53,
    ImplementationColorReadFormat = 0x8B9B,
    ImplementationColorReadType = 0x8B9A,
    LineWidth = 0x0B21,
    MaxCombinedTextureImageUnits = 0x8B4D,
    MaxCubeMapTextureSize = 0x851C,
    MaxFragmentUniformVectors = 0x8DFD,
    MaxRenderbufferSize = 0x84E8,
    MaxTextureImageUnits = 0x8872,
    MaxTextureSize = 0x0D33,
    MaxVaryingVectors = 0x8DFC,
    MaxVertexAttribs = 0x8869,
    MaxVertexTextureImageUnits = 0x8B4C,
    MaxVertexUniformVectors = 0x8DFB,
    MaxViewportDims = 0x0D3A,
    NumCompressedTextureFormats = 0x86A2,
    NumShaderBinaryFormats = 0x8DF9,
    PackAlignment = 0x0D05,
    PolygonOffsetFactor = 0x8038,
    PolygonOffsetFill = 0x8037,
    PolygonOffsetUnits = 0x2A00,
    RedBits = 0x0D52,
    RenderbufferBinding = 0x8CA7,
    Samples = 0x80A9,
    SampleBuffers = 0x80A8,
    SampleCoverageInvert = 0x80AB,
    SampleCoverageValue = 0x80AA,
    ScissorBox = 0x0C10,
    ScissorTest = 0x0C11,
    ShaderCompiler = 0x8DFA,
    StencilBackFail = 0x8801,
    StencilBackFunc = 0x8800,
    StencilBackPassDepthFail = 0x8802,
    StencilBackPassDepthPass = 0x8803,
    StencilBackRef = 0x8CA3,
    StencilBackValueMask = 0x8CA4,
    StencilBackWritemask = 0x8CA5,
    StencilBits = 0x0D57,
    StencilClearValue = 0x0B91,
    StencilFail = 0x0B94,
    StencilFunc = 0x0B92,
    StencilPassDepthFail = 0x0B95,
    StencilPassDepthPass = 0x0B96,
    StencilRef = 0x0B97,
    StencilTest = 0x0B90,
    StencilValueMask = 0x0B93,
    StencilWritemask = 0x0B98,
    SubpixelBits = 0x0D50,
    Texture2d = 0x0DE1,
    TextureBinding2d = 0x8069,
    TextureBindingCubeMap = 0x8514,
    UnpackAlignment = 0x0CF5,
    Viewport = 0x0BA2,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    DrawFramebufferBinding = 0x8CA6,
    FragmentShaderDerivativeHint = 0x8B8B,
    MajorVersion = 0x821B,
    Max3dTextureSize = 0x8073,
    MaxArrayTextureLayers = 0x88FF,
    MaxCombinedFragmentUniformComponents = 0x8A33,
    MaxCombinedUniformBlocks = 0x8A2E,
    MaxCombinedVertexUniformComponents = 0x8A31,
    MaxDrawBuffers = 0x8824,
    MaxElementsIndices = 0x80E9,
    MaxElementsVertices = 0x80E8,
    MaxElementIndex = 0x8D6B,
    MaxFragmentInputComponents = 0x9125,
    MaxFragmentUniformBlocks = 0x8A2D,
    MaxFragmentUniformComponents = 0x8B49,
    MaxProgramTexelOffset = 0x8905,
    MaxServerWaitTimeout = 0x9111,
    MaxTextureLodBias = 0x84FD,
    MaxUniformBlockSize = 0x8A30,
    MaxUniformBufferBindings = 0x8A2F,
    MaxVaryingComponents = 0x8B4B,
    MaxVertexOutputComponents = 0x9122,
    MaxVertexUniformBlocks = 0x8A2B,
    MaxVertexUniformComponents = 0x8B4A,
    MinorVersion = 0x821C,
    MinProgramTexelOffset = 0x8904,
    NumExtensions = 0x821D,
    NumProgramBinaryFormats = 0x87FE,
    PackRowLength = 0x0D02,
    PackSkipPixels = 0x0D04,
    PackSkipRows = 0x0D03,
    PixelPackBufferBinding = 0x88ED,
    PixelUnpackBufferBinding = 0x88EF,
    ProgramBinaryFormats = 0x87FF,
    ReadBuffer = 0x0C02,
    ReadFramebufferBinding = 0x8CAA,
    SamplerBinding = 0x8919,
    TextureBinding2dArray = 0x8C1D,
    TextureBinding3d = 0x806A,
    TransformFeedbackBufferBinding = 0x8C8F,
    TransformFeedbackBufferSize = 0x8C85,
    TransformFeedbackBufferStart = 0x8C84,
    UniformBufferBinding = 0x8A28,
    UniformBufferOffsetAlignment = 0x8A34,
    UniformBufferSize = 0x8A2A,
    UniformBufferStart = 0x8A29,
    UnpackImageHeight = 0x806E,
    UnpackRowLength = 0x0CF2,
    UnpackSkipImages = 0x806D,
    UnpackSkipPixels = 0x0CF4,
    UnpackSkipRows = 0x0CF3,
    VertexArrayBinding = 0x85B5,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    DispatchIndirectBufferBinding = 0x90EF,
    MaxColorTextureSamples = 0x910E,
    MaxCombinedAtomicCounters = 0x92D7,
    MaxCombinedComputeUniformComponents = 0x8266,
    MaxCombinedShaderStorageBlocks = 0x90DC,
    MaxComputeAtomicCounters = 0x8265,
    MaxComputeAtomicCounterBuffers = 0x8264,
    MaxComputeShaderStorageBlocks = 0x90DB,
    MaxComputeTextureImageUnits = 0x91BC,
    MaxComputeUniformBlocks = 0x91BB,
    MaxComputeUniformComponents = 0x8263,
    MaxComputeWorkGroupCount = 0x91BE,
    MaxComputeWorkGroupInvocations = 0x90EB,
    MaxComputeWorkGroupSize = 0x91BF,
    MaxDepthTextureSamples = 0x910F,
    MaxFragmentAtomicCounters = 0x92D6,
    MaxFragmentShaderStorageBlocks = 0x90DA,
    MaxFramebufferHeight = 0x9316,
    MaxFramebufferSamples = 0x9318,
    MaxFramebufferWidth = 0x9315,
    MaxIntegerSamples = 0x9110,
    MaxSampleMaskWords = 0x8E59,
    MaxShaderStorageBufferBindings = 0x90DD,
    MaxUniformLocations = 0x826E,
    MaxVertexAtomicCounters = 0x92D2,
    MaxVertexAttribBindings = 0x82DA,
    MaxVertexAttribRelativeOffset = 0x82D9,
    MaxVertexShaderStorageBlocks = 0x90D6,
    ProgramPipelineBinding = 0x825A,
    ShaderStorageBufferBinding = 0x90D3,
    ShaderStorageBufferOffsetAlignment = 0x90DF,
    ShaderStorageBufferSize = 0x90D5,
    ShaderStorageBufferStart = 0x90D4,
    TextureBinding2dMultisample = 0x9104,
    VertexBindingDivisor = 0x82D6,
    VertexBindingOffset = 0x82D7,
    VertexBindingStride = 0x82D8,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    ContextFlags = 0x821E,
    DebugGroupStackDepth = 0x826D,
    LayerProvokingVertex = 0x825E,
    MaxCombinedGeometryUniformComponents = 0x8A32,
    MaxDebugGroupStackDepth = 0x826C,
    MaxFramebufferLayers = 0x9317,
    MaxGeometryAtomicCounters = 0x92D5,
    MaxGeometryInputComponents = 0x9123,
    MaxGeometryOutputComponents = 0x9124,
    MaxGeometryShaderStorageBlocks = 0x90D7,
    MaxGeometryTextureImageUnits = 0x8C29,
    MaxGeometryUniformBlocks = 0x8A2C,
    MaxGeometryUniformComponents = 0x8DDF,
    MaxLabelLength = 0x82E8,
    MaxTessControlAtomicCounters = 0x92D3,
    MaxTessControlShaderStorageBlocks = 0x90D8,
    MaxTessEvaluationAtomicCounters = 0x92D4,
    MaxTessEvaluationShaderStorageBlocks = 0x90D9,
    MaxTextureBufferSize = 0x8C2B,
    TextureBinding2dMultisampleArray = 0x9105,
    TextureBindingBuffer = 0x8C2C,
    TextureBufferOffsetAlignment = 0x919F,
    VertexArray = 0x8074,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum GetPName

enum class GetPointervPName : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    DebugCallbackFunction = 0x8244,
    DebugCallbackUserParam = 0x8245,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum GetPointervPName

enum class GetTextureParameter : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    TextureMagFilter = 0x2800,
    TextureMinFilter = 0x2801,
    TextureWrapS = 0x2802,
    TextureWrapT = 0x2803,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    TextureAlphaSize = 0x805F,
    TextureBlueSize = 0x805E,
    TextureGreenSize = 0x805D,
    TextureHeight = 0x1001,
    TextureInternalFormat = 0x1003,
    TextureRedSize = 0x805C,
    TextureWidth = 0x1000,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    TextureBorderColor = 0x1004,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum GetTextureParameter

enum class GlslTypeToken : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Bool = 0x8B56,
    BoolVec2 = 0x8B57,
    BoolVec3 = 0x8B58,
    BoolVec4 = 0x8B59,
    Float = 0x1406,
    FloatMat2 = 0x8B5A,
    FloatMat3 = 0x8B5B,
    FloatMat4 = 0x8B5C,
    FloatVec2 = 0x8B50,
    FloatVec3 = 0x8B51,
    FloatVec4 = 0x8B52,
    Int = 0x1404,
    IntVec2 = 0x8B53,
    IntVec3 = 0x8B54,
    IntVec4 = 0x8B55,
    Sampler2d = 0x8B5E,
    SamplerCube = 0x8B60,
    UnsignedInt = 0x1405,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    FloatMat2x3 = 0x8B65,
    FloatMat2x4 = 0x8B66,
    FloatMat3x2 = 0x8B67,
    FloatMat3x4 = 0x8B68,
    FloatMat4x2 = 0x8B69,
    FloatMat4x3 = 0x8B6A,
    IntSampler2d = 0x8DCA,
    IntSampler2dArray = 0x8DCF,
    IntSampler3d = 0x8DCB,
    IntSamplerCube = 0x8DCC,
    Sampler2dArray = 0x8DC1,
    Sampler2dArrayShadow = 0x8DC4,
    Sampler2dShadow = 0x8B62,
    Sampler3d = 0x8B5F,
    SamplerCubeShadow = 0x8DC5,
    UnsignedIntSampler2d = 0x8DD2,
    UnsignedIntSampler2dArray = 0x8DD7,
    UnsignedIntSampler3d = 0x8DD3,
    UnsignedIntSamplerCube = 0x8DD4,
    UnsignedIntVec2 = 0x8DC6,
    UnsignedIntVec3 = 0x8DC7,
    UnsignedIntVec4 = 0x8DC8,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Image2d = 0x904D,
    Image2dArray = 0x9053,
    Image3d = 0x904E,
    ImageCube = 0x9050,
    IntImage2d = 0x9058,
    IntImage2dArray = 0x905E,
    IntImage3d = 0x9059,
    IntImageCube = 0x905B,
    IntSampler2dMultisample = 0x9109,
    Sampler2dMultisample = 0x9108,
    UnsignedIntAtomicCounter = 0x92DB,
    UnsignedIntImage2d = 0x9063,
    UnsignedIntImage2dArray = 0x9069,
    UnsignedIntImage3d = 0x9064,
    UnsignedIntImageCube = 0x9066,
    UnsignedIntSampler2dMultisample = 0x910A,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    ImageBuffer = 0x9051,
    ImageCubeMapArray = 0x9054,
    IntImageBuffer = 0x905C,
    IntImageCubeMapArray = 0x905F,
    IntSampler2dMultisampleArray = 0x910C,
    IntSamplerBuffer = 0x8DD0,
    IntSamplerCubeMapArray = 0x900E,
    Sampler2dMultisampleArray = 0x910B,
    SamplerBuffer = 0x8DC2,
    SamplerCubeMapArray = 0x900C,
    SamplerCubeMapArrayShadow = 0x900D,
    UnsignedIntImageBuffer = 0x9067,
    UnsignedIntImageCubeMapArray = 0x906A,
    UnsignedIntSampler2dMultisampleArray = 0x910D,
    UnsignedIntSamplerBuffer = 0x8DD8,
    UnsignedIntSamplerCubeMapArray = 0x900F,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum GlslTypeToken

enum class GraphicsResetStatus : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    NoError = 0,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    GuiltyContextReset = 0x8253,
    InnocentContextReset = 0x8254,
    UnknownContextReset = 0x8255,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum GraphicsResetStatus

enum class HintMode : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    DontCare = 0x1100,
    Fastest = 0x1101,
    Nicest = 0x1102,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum HintMode

enum class HintTarget : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    GenerateMipmapHint = 0x8192,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    FragmentShaderDerivativeHint = 0x8B8B,
    ProgramBinaryRetrievableHint = 0x8257,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum HintTarget

enum class IndexPointerType : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum IndexPointerType

enum class InternalFormat : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    DepthComponent = 0x1902,
    DepthComponent16 = 0x81A5,
    Rgb = 0x1907,
    Rgb5A1 = 0x8057,
    Rgba = 0x1908,
    Rgba4 = 0x8056,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    CompressedR11Eac = 0x9270,
    CompressedRg11Eac = 0x9272,
    CompressedRgb8Etc2 = 0x9274,
    CompressedRgb8PunchthroughAlpha1Etc2 = 0x9276,
    CompressedRgba8Etc2Eac = 0x9278,
    CompressedSignedR11Eac = 0x9271,
    CompressedSignedRg11Eac = 0x9273,
    CompressedSrgb8Alpha8Etc2Eac = 0x9279,
    CompressedSrgb8Etc2 = 0x9275,
    CompressedSrgb8PunchthroughAlpha1Etc2 = 0x9277,
    Depth24Stencil8 = 0x88F0,
    Depth32fStencil8 = 0x8CAD,
    DepthComponent32f = 0x8CAC,
    DepthStencil = 0x84F9,
    R11fG11fB10f = 0x8C3A,
    R16f = 0x822D,
    R16i = 0x8233,
    R16ui = 0x8234,
    R32f = 0x822E,
    R32i = 0x8235,
    R32ui = 0x8236,
    R8 = 0x8229,
    R8i = 0x8231,
    R8ui = 0x8232,
    R8Snorm = 0x8F94,
    Red = 0x1903,
    Rg = 0x8227,
    Rg16f = 0x822F,
    Rg16i = 0x8239,
    Rg16ui = 0x823A,
    Rg32f = 0x8230,
    Rg32i = 0x823B,
    Rg32ui = 0x823C,
    Rg8 = 0x822B,
    Rg8i = 0x8237,
    Rg8ui = 0x8238,
    Rg8Snorm = 0x8F95,
    Rgb10A2 = 0x8059,
    Rgb10A2ui = 0x906F,
    Rgb16f = 0x881B,
    Rgb16i = 0x8D89,
    Rgb16ui = 0x8D77,
    Rgb32f = 0x8815,
    Rgb32i = 0x8D83,
    Rgb32ui = 0x8D71,
    Rgb8 = 0x8051,
    Rgb8i = 0x8D8F,
    Rgb8ui = 0x8D7D,
    Rgb8Snorm = 0x8F96,
    Rgb9E5 = 0x8C3D,
    Rgba16f = 0x881A,
    Rgba16i = 0x8D88,
    Rgba16ui = 0x8D76,
    Rgba32f = 0x8814,
    Rgba32i = 0x8D82,
    Rgba32ui = 0x8D70,
    Rgba8 = 0x8058,
    Rgba8i = 0x8D8E,
    Rgba8ui = 0x8D7C,
    Rgba8Snorm = 0x8F97,
    Srgb = 0x8C40,
    Srgb8 = 0x8C41,
    Srgb8Alpha8 = 0x8C43,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum InternalFormat

enum class InternalFormatPName : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Samples = 0x80A9,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    NumSampleCounts = 0x9380,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    ImageFormatCompatibilityType = 0x90C7,
    TextureCompressed = 0x86A1,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum InternalFormatPName

enum class LightEnvModeSGIX : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Replace = 0x1E01,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum LightEnvModeSGIX

enum class ListNameType : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Byte = 0x1400,
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
    UnsignedByte = 0x1401,
    UnsignedInt = 0x1405,
    UnsignedShort = 0x1403,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum ListNameType

enum class LogicOp : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Invert = 0x150A,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum LogicOp

enum class MapBufferAccessMask : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    MapFlushExplicitBit = 0x0010,
    MapInvalidateBufferBit = 0x0008,
    MapInvalidateRangeBit = 0x0004,
    MapReadBit = 0x0001,
    MapUnsynchronizedBit = 0x0020,
    MapWriteBit = 0x0002,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum MapBufferAccessMask
constexpr core::Bitfield<MapBufferAccessMask> operator|(MapBufferAccessMask left, MapBufferAccessMask right) { return core::Bitfield<MapBufferAccessMask>(left) | right;}

enum class MaterialFace : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Back = 0x0405,
    Front = 0x0404,
    FrontAndBack = 0x0408,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum MaterialFace

enum class MatrixMode : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Texture = 0x1702,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum MatrixMode

enum class MemoryBarrierMask : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    AllBarrierBits = 0xFFFFFFFF,
    AtomicCounterBarrierBit = 0x00001000,
    BufferUpdateBarrierBit = 0x00000200,
    CommandBarrierBit = 0x00000040,
    ElementArrayBarrierBit = 0x00000002,
    FramebufferBarrierBit = 0x00000400,
    PixelBufferBarrierBit = 0x00000080,
    ShaderImageAccessBarrierBit = 0x00000020,
    ShaderStorageBarrierBit = 0x00002000,
    TextureFetchBarrierBit = 0x00000008,
    TextureUpdateBarrierBit = 0x00000100,
    TransformFeedbackBarrierBit = 0x00000800,
    UniformBarrierBit = 0x00000004,
    VertexAttribArrayBarrierBit = 0x00000001,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum MemoryBarrierMask
constexpr core::Bitfield<MemoryBarrierMask> operator|(MemoryBarrierMask left, MemoryBarrierMask right) { return core::Bitfield<MemoryBarrierMask>(left) | right;}

enum class NormalPointerType : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Byte = 0x1400,
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum NormalPointerType

enum class ObjectIdentifier : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Framebuffer = 0x8D40,
    Renderbuffer = 0x8D41,
    Texture = 0x1702,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    TransformFeedback = 0x8E22,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    Buffer = 0x82E0,
    Program = 0x82E2,
    ProgramPipeline = 0x82E4,
    Query = 0x82E3,
    Sampler = 0x82E6,
    Shader = 0x82E1,
    VertexArray = 0x8074,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum ObjectIdentifier

enum class PatchParameterName : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    PatchVertices = 0x8E72,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum PatchParameterName

enum class PathFillMode : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Invert = 0x150A,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum PathFillMode

enum class PathFontStyle : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    None = 0,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum PathFontStyle

enum class PathGenMode : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    None = 0,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum PathGenMode

enum class PathTransformType : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    None = 0,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum PathTransformType

enum class PipelineParameterName : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    FragmentShader = 0x8B30,
    InfoLogLength = 0x8B84,
    VertexShader = 0x8B31,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    ActiveProgram = 0x8259,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    GeometryShader = 0x8DD9,
    TessControlShader = 0x8E88,
    TessEvaluationShader = 0x8E87,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum PipelineParameterName

enum class PixelCopyType : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Color = 0x1800,
    Depth = 0x1801,
    Stencil = 0x1802,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum PixelCopyType

enum class PixelFormat : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Alpha = 0x1906,
    DepthComponent = 0x1902,
    Luminance = 0x1909,
    LuminanceAlpha = 0x190A,
    Rgb = 0x1907,
    Rgba = 0x1908,
    UnsignedInt = 0x1405,
    UnsignedShort = 0x1403,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Blue = 0x1905,
    DepthStencil = 0x84F9,
    Green = 0x1904,
    Red = 0x1903,
    RedInteger = 0x8D94,
    Rg = 0x8227,
    RgbaInteger = 0x8D99,
    RgbInteger = 0x8D98,
    RgInteger = 0x8228,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    StencilIndex = 0x1901,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum PixelFormat

enum class PixelStoreParameter : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    PackAlignment = 0x0D05,
    UnpackAlignment = 0x0CF5,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    PackRowLength = 0x0D02,
    PackSkipPixels = 0x0D04,
    PackSkipRows = 0x0D03,
    UnpackImageHeight = 0x806E,
    UnpackRowLength = 0x0CF2,
    UnpackSkipImages = 0x806D,
    UnpackSkipPixels = 0x0CF4,
    UnpackSkipRows = 0x0CF3,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum PixelStoreParameter

enum class PixelTexGenMode : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Luminance = 0x1909,
    LuminanceAlpha = 0x190A,
    None = 0,
    Rgb = 0x1907,
    Rgba = 0x1908,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum PixelTexGenMode

enum class PixelType : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Byte = 0x1400,
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
    UnsignedByte = 0x1401,
    UnsignedInt = 0x1405,
    UnsignedShort = 0x1403,
    UnsignedShort4444 = 0x8033,
    UnsignedShort5551 = 0x8034,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum PixelType

enum class PrecisionType : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    HighFloat = 0x8DF2,
    HighInt = 0x8DF5,
    LowFloat = 0x8DF0,
    LowInt = 0x8DF3,
    MediumFloat = 0x8DF1,
    MediumInt = 0x8DF4,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum PrecisionType

enum class PrimitiveType : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Lines = 0x0001,
    LineLoop = 0x0002,
    LineStrip = 0x0003,
    Points = 0x0000,
    Triangles = 0x0004,
    TriangleFan = 0x0006,
    TriangleStrip = 0x0005,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    LinesAdjacency = 0x000A,
    LineStripAdjacency = 0x000B,
    Patches = 0x000E,
    Quads = 0x0007,
    TrianglesAdjacency = 0x000C,
    TriangleStripAdjacency = 0x000D,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum PrimitiveType

enum class ProgramInterface : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    TransformFeedbackBuffer = 0x8C8E,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    BufferVariable = 0x92E5,
    ProgramInput = 0x92E3,
    ProgramOutput = 0x92E4,
    ShaderStorageBlock = 0x92E6,
    TransformFeedbackVarying = 0x92F4,
    Uniform = 0x92E1,
    UniformBlock = 0x92E2,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum ProgramInterface

enum class ProgramInterfacePName : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    ActiveResources = 0x92F5,
    MaxNameLength = 0x92F6,
    MaxNumActiveVariables = 0x92F7,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum ProgramInterfacePName

enum class ProgramParameterPName : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    ProgramBinaryRetrievableHint = 0x8257,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    ProgramSeparable = 0x8258,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum ProgramParameterPName

enum class ProgramPropertyARB : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    ActiveAttributes = 0x8B89,
    ActiveAttributeMaxLength = 0x8B8A,
    ActiveUniforms = 0x8B86,
    ActiveUniformMaxLength = 0x8B87,
    AttachedShaders = 0x8B85,
    DeleteStatus = 0x8B80,
    InfoLogLength = 0x8B84,
    LinkStatus = 0x8B82,
    ValidateStatus = 0x8B83,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    ActiveUniformBlocks = 0x8A36,
    ActiveUniformBlockMaxNameLength = 0x8A35,
    ProgramBinaryLength = 0x8741,
    TransformFeedbackBufferMode = 0x8C7F,
    TransformFeedbackVaryings = 0x8C83,
    TransformFeedbackVaryingMaxLength = 0x8C76,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    ActiveAtomicCounterBuffers = 0x92D9,
    ComputeWorkGroupSize = 0x8267,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    GeometryInputType = 0x8917,
    GeometryOutputType = 0x8918,
    GeometryVerticesOut = 0x8916,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum ProgramPropertyARB

enum class ProgramResourceProperty : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    ActiveVariables = 0x9305,
    ArraySize = 0x92FB,
    ArrayStride = 0x92FE,
    AtomicCounterBufferIndex = 0x9301,
    BlockIndex = 0x92FD,
    BufferBinding = 0x9302,
    BufferDataSize = 0x9303,
    IsRowMajor = 0x9300,
    Location = 0x930E,
    MatrixStride = 0x92FF,
    NameLength = 0x92F9,
    NumActiveVariables = 0x9304,
    Offset = 0x92FC,
    ReferencedByComputeShader = 0x930B,
    ReferencedByFragmentShader = 0x930A,
    ReferencedByVertexShader = 0x9306,
    TopLevelArraySize = 0x930C,
    TopLevelArrayStride = 0x930D,
    Type = 0x92FA,
    Uniform = 0x92E1,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    IsPerPatch = 0x92E7,
    ReferencedByGeometryShader = 0x9309,
    ReferencedByTessControlShader = 0x9307,
    ReferencedByTessEvaluationShader = 0x9308,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum ProgramResourceProperty

enum class QueryObjectParameterName : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    QueryResult = 0x8866,
    QueryResultAvailable = 0x8867,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum QueryObjectParameterName

enum class QueryParameterName : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    CurrentQuery = 0x8865,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum QueryParameterName

enum class QueryTarget : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    AnySamplesPassed = 0x8C2F,
    AnySamplesPassedConservative = 0x8D6A,
    TransformFeedbackPrimitivesWritten = 0x8C88,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    PrimitivesGenerated = 0x8C87,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum QueryTarget

enum class ReadBufferMode : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Back = 0x0405,
    ColorAttachment0 = 0x8CE0,
    Front = 0x0404,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    ColorAttachment1 = 0x8CE1,
    ColorAttachment10 = 0x8CEA,
    ColorAttachment11 = 0x8CEB,
    ColorAttachment12 = 0x8CEC,
    ColorAttachment13 = 0x8CED,
    ColorAttachment14 = 0x8CEE,
    ColorAttachment15 = 0x8CEF,
    ColorAttachment2 = 0x8CE2,
    ColorAttachment3 = 0x8CE3,
    ColorAttachment4 = 0x8CE4,
    ColorAttachment5 = 0x8CE5,
    ColorAttachment6 = 0x8CE6,
    ColorAttachment7 = 0x8CE7,
    ColorAttachment8 = 0x8CE8,
    ColorAttachment9 = 0x8CE9,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum ReadBufferMode

enum class RenderbufferParameterName : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    RenderbufferAlphaSize = 0x8D53,
    RenderbufferBlueSize = 0x8D52,
    RenderbufferDepthSize = 0x8D54,
    RenderbufferGreenSize = 0x8D51,
    RenderbufferHeight = 0x8D43,
    RenderbufferInternalFormat = 0x8D44,
    RenderbufferRedSize = 0x8D50,
    RenderbufferStencilSize = 0x8D55,
    RenderbufferWidth = 0x8D42,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    RenderbufferSamples = 0x8CAB,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum RenderbufferParameterName

enum class RenderbufferTarget : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Renderbuffer = 0x8D41,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum RenderbufferTarget

enum class SamplerParameterF : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    TextureMaxLod = 0x813B,
    TextureMinLod = 0x813A,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    TextureBorderColor = 0x1004,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum SamplerParameterF

enum class SamplerParameterI : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    TextureMagFilter = 0x2800,
    TextureMinFilter = 0x2801,
    TextureWrapS = 0x2802,
    TextureWrapT = 0x2803,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    TextureCompareFunc = 0x884D,
    TextureCompareMode = 0x884C,
    TextureWrapR = 0x8072,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum SamplerParameterI

enum class ShaderParameterName : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    CompileStatus = 0x8B81,
    DeleteStatus = 0x8B80,
    InfoLogLength = 0x8B84,
    ShaderSourceLength = 0x8B88,
    ShaderType = 0x8B4F,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum ShaderParameterName

enum class ShaderType : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    FragmentShader = 0x8B30,
    VertexShader = 0x8B31,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    ComputeShader = 0x91B9,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    GeometryShader = 0x8DD9,
    TessControlShader = 0x8E88,
    TessEvaluationShader = 0x8E87,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum ShaderType

enum class StencilFaceDirection : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Back = 0x0405,
    Front = 0x0404,
    FrontAndBack = 0x0408,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum StencilFaceDirection

enum class StencilFunction : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Always = 0x0207,
    Equal = 0x0202,
    Gequal = 0x0206,
    Greater = 0x0204,
    Lequal = 0x0203,
    Less = 0x0201,
    Never = 0x0200,
    Notequal = 0x0205,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum StencilFunction

enum class StencilOp : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Decr = 0x1E03,
    DecrWrap = 0x8508,
    Incr = 0x1E02,
    IncrWrap = 0x8507,
    Invert = 0x150A,
    Keep = 0x1E00,
    Replace = 0x1E01,
    Zero = 0,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum StencilOp

enum class StringName : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Extensions = 0x1F03,
    Renderer = 0x1F01,
    ShadingLanguageVersion = 0x8B8C,
    Vendor = 0x1F00,
    Version = 0x1F02,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum StringName

enum class SubroutineParameterName : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    UniformNameLength = 0x8A39,
    UniformSize = 0x8A38,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum SubroutineParameterName

enum class SyncCondition : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    SyncGpuCommandsComplete = 0x9117,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum SyncCondition

enum class SyncObjectMask : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    SyncFlushCommandsBit = 0x00000001,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum SyncObjectMask
constexpr core::Bitfield<SyncObjectMask> operator|(SyncObjectMask left, SyncObjectMask right) { return core::Bitfield<SyncObjectMask>(left) | right;}

enum class SyncParameterName : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    ObjectType = 0x9112,
    SyncCondition = 0x9113,
    SyncFlags = 0x9115,
    SyncStatus = 0x9114,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum SyncParameterName

enum class SyncStatus : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    AlreadySignaled = 0x911A,
    ConditionSatisfied = 0x911C,
    TimeoutExpired = 0x911B,
    WaitFailed = 0x911D,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum SyncStatus

enum class TexCoordPointerType : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum TexCoordPointerType

enum class TextureCompareMode : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    None = 0,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    CompareRefToTexture = 0x884E,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum TextureCompareMode

enum class TextureEnvMode : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Blend = 0x0BE2,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum TextureEnvMode

enum class TextureMagFilter : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Linear = 0x2601,
    Nearest = 0x2600,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum TextureMagFilter

enum class TextureMinFilter : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Linear = 0x2601,
    LinearMipmapLinear = 0x2703,
    LinearMipmapNearest = 0x2701,
    Nearest = 0x2600,
    NearestMipmapLinear = 0x2702,
    NearestMipmapNearest = 0x2700,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum TextureMinFilter

enum class TextureParameterName : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    TextureMagFilter = 0x2800,
    TextureMinFilter = 0x2801,
    TextureWrapS = 0x2802,
    TextureWrapT = 0x2803,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    TextureBaseLevel = 0x813C,
    TextureCompareFunc = 0x884D,
    TextureCompareMode = 0x884C,
    TextureMaxLevel = 0x813D,
    TextureMaxLod = 0x813B,
    TextureMinLod = 0x813A,
    TextureSwizzleA = 0x8E45,
    TextureSwizzleB = 0x8E44,
    TextureSwizzleG = 0x8E43,
    TextureSwizzleR = 0x8E42,
    TextureWrapR = 0x8072,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    DepthStencilTextureMode = 0x90EA,
    TextureAlphaSize = 0x805F,
    TextureBlueSize = 0x805E,
    TextureGreenSize = 0x805D,
    TextureHeight = 0x1001,
    TextureInternalFormat = 0x1003,
    TextureRedSize = 0x805C,
    TextureWidth = 0x1000,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    TextureBorderColor = 0x1004,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum TextureParameterName

enum class TextureSwizzle : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Alpha = 0x1906,
    One = 1,
    Zero = 0,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Blue = 0x1905,
    Green = 0x1904,
    Red = 0x1903,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum TextureSwizzle

enum class TextureTarget : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Texture2d = 0x0DE1,
    TextureCubeMap = 0x8513,
    TextureCubeMapNegativeX = 0x8516,
    TextureCubeMapNegativeY = 0x8518,
    TextureCubeMapNegativeZ = 0x851A,
    TextureCubeMapPositiveX = 0x8515,
    TextureCubeMapPositiveY = 0x8517,
    TextureCubeMapPositiveZ = 0x8519,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Texture2dArray = 0x8C1A,
    Texture3d = 0x806F,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Texture2dMultisample = 0x9100,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    Texture2dMultisampleArray = 0x9102,
    TextureCubeMapArray = 0x9009,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum TextureTarget

enum class TextureUnit : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Texture0 = 0x84C0,
    Texture1 = 0x84C1,
    Texture10 = 0x84CA,
    Texture11 = 0x84CB,
    Texture12 = 0x84CC,
    Texture13 = 0x84CD,
    Texture14 = 0x84CE,
    Texture15 = 0x84CF,
    Texture16 = 0x84D0,
    Texture17 = 0x84D1,
    Texture18 = 0x84D2,
    Texture19 = 0x84D3,
    Texture2 = 0x84C2,
    Texture20 = 0x84D4,
    Texture21 = 0x84D5,
    Texture22 = 0x84D6,
    Texture23 = 0x84D7,
    Texture24 = 0x84D8,
    Texture25 = 0x84D9,
    Texture26 = 0x84DA,
    Texture27 = 0x84DB,
    Texture28 = 0x84DC,
    Texture29 = 0x84DD,
    Texture3 = 0x84C3,
    Texture30 = 0x84DE,
    Texture31 = 0x84DF,
    Texture4 = 0x84C4,
    Texture5 = 0x84C5,
    Texture6 = 0x84C6,
    Texture7 = 0x84C7,
    Texture8 = 0x84C8,
    Texture9 = 0x84C9,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum TextureUnit

enum class TextureWrapMode : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    ClampToEdge = 0x812F,
    LinearMipmapLinear = 0x2703,
    MirroredRepeat = 0x8370,
    Repeat = 0x2901,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    ClampToBorder = 0x812D,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum TextureWrapMode

enum class TransformFeedbackBufferMode : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    InterleavedAttribs = 0x8C8C,
    SeparateAttribs = 0x8C8D,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum TransformFeedbackBufferMode

enum class TransformFeedbackPName : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    TransformFeedbackActive = 0x8E24,
    TransformFeedbackBufferBinding = 0x8C8F,
    TransformFeedbackBufferSize = 0x8C85,
    TransformFeedbackBufferStart = 0x8C84,
    TransformFeedbackPaused = 0x8E23,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum TransformFeedbackPName

enum class UniformBlockPName : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    UniformBlockActiveUniforms = 0x8A42,
    UniformBlockActiveUniformIndices = 0x8A43,
    UniformBlockBinding = 0x8A3F,
    UniformBlockDataSize = 0x8A40,
    UniformBlockNameLength = 0x8A41,
    UniformBlockReferencedByFragmentShader = 0x8A46,
    UniformBlockReferencedByVertexShader = 0x8A44,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum UniformBlockPName

enum class UniformPName : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    UniformArrayStride = 0x8A3C,
    UniformBlockIndex = 0x8A3A,
    UniformIsRowMajor = 0x8A3E,
    UniformMatrixStride = 0x8A3D,
    UniformNameLength = 0x8A39,
    UniformOffset = 0x8A3B,
    UniformSize = 0x8A38,
    UniformType = 0x8A37,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum UniformPName

enum class UniformType : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Bool = 0x8B56,
    BoolVec2 = 0x8B57,
    BoolVec3 = 0x8B58,
    BoolVec4 = 0x8B59,
    Float = 0x1406,
    FloatMat2 = 0x8B5A,
    FloatMat3 = 0x8B5B,
    FloatMat4 = 0x8B5C,
    FloatVec2 = 0x8B50,
    FloatVec3 = 0x8B51,
    FloatVec4 = 0x8B52,
    Int = 0x1404,
    IntVec2 = 0x8B53,
    IntVec3 = 0x8B54,
    IntVec4 = 0x8B55,
    Sampler2d = 0x8B5E,
    SamplerCube = 0x8B60,
    UnsignedInt = 0x1405,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    IntSampler2d = 0x8DCA,
    IntSampler2dArray = 0x8DCF,
    IntSampler3d = 0x8DCB,
    IntSamplerCube = 0x8DCC,
    Sampler2dArray = 0x8DC1,
    Sampler2dArrayShadow = 0x8DC4,
    Sampler2dShadow = 0x8B62,
    Sampler3d = 0x8B5F,
    SamplerCubeShadow = 0x8DC5,
    UnsignedIntSampler2d = 0x8DD2,
    UnsignedIntSampler2dArray = 0x8DD7,
    UnsignedIntSampler3d = 0x8DD3,
    UnsignedIntSamplerCube = 0x8DD4,
    UnsignedIntVec2 = 0x8DC6,
    UnsignedIntVec3 = 0x8DC7,
    UnsignedIntVec4 = 0x8DC8,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    IntSampler2dMultisample = 0x9109,
    Sampler2dMultisample = 0x9108,
    UnsignedIntSampler2dMultisample = 0x910A,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    IntSampler2dMultisampleArray = 0x910C,
    IntSamplerBuffer = 0x8DD0,
    Sampler2dMultisampleArray = 0x910B,
    SamplerBuffer = 0x8DC2,
    UnsignedIntSampler2dMultisampleArray = 0x910D,
    UnsignedIntSamplerBuffer = 0x8DD8,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum UniformType

enum class UseProgramStageMask : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    AllShaderBits = 0xFFFFFFFF,
    ComputeShaderBit = 0x00000020,
    FragmentShaderBit = 0x00000002,
    VertexShaderBit = 0x00000001,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    GeometryShaderBit = 0x00000004,
    TessControlShaderBit = 0x00000008,
    TessEvaluationShaderBit = 0x00000010,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum UseProgramStageMask
constexpr core::Bitfield<UseProgramStageMask> operator|(UseProgramStageMask left, UseProgramStageMask right) { return core::Bitfield<UseProgramStageMask>(left) | right;}

enum class VertexArrayPName : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    VertexAttribArrayEnabled = 0x8622,
    VertexAttribArrayNormalized = 0x886A,
    VertexAttribArraySize = 0x8623,
    VertexAttribArrayStride = 0x8624,
    VertexAttribArrayType = 0x8625,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    VertexAttribArrayDivisor = 0x88FE,
    VertexAttribArrayInteger = 0x88FD,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    VertexAttribRelativeOffset = 0x82D5,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum VertexArrayPName

enum class VertexAttribEnum : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    CurrentVertexAttrib = 0x8626,
    VertexAttribArrayBufferBinding = 0x889F,
    VertexAttribArrayEnabled = 0x8622,
    VertexAttribArrayNormalized = 0x886A,
    VertexAttribArraySize = 0x8623,
    VertexAttribArrayStride = 0x8624,
    VertexAttribArrayType = 0x8625,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    VertexAttribArrayDivisor = 0x88FE,
    VertexAttribArrayInteger = 0x88FD,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum VertexAttribEnum

enum class VertexAttribIType : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Byte = 0x1400,
    Int = 0x1404,
    Short = 0x1402,
    UnsignedByte = 0x1401,
    UnsignedInt = 0x1405,
    UnsignedShort = 0x1403,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum VertexAttribIType

enum class VertexAttribPointerPropertyARB : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    VertexAttribArrayPointer = 0x8645,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum VertexAttribPointerPropertyARB

enum class VertexAttribPointerType : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Byte = 0x1400,
    Fixed = 0x140C,
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
    UnsignedByte = 0x1401,
    UnsignedInt = 0x1405,
    UnsignedShort = 0x1403,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    HalfFloat = 0x140B,
    Int2101010Rev = 0x8D9F,
    UnsignedInt10f11f11fRev = 0x8C3B,
    UnsignedInt2101010Rev = 0x8368,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum VertexAttribPointerType

enum class VertexAttribPropertyARB : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    CurrentVertexAttrib = 0x8626,
    VertexAttribArrayBufferBinding = 0x889F,
    VertexAttribArrayEnabled = 0x8622,
    VertexAttribArrayNormalized = 0x886A,
    VertexAttribArraySize = 0x8623,
    VertexAttribArrayStride = 0x8624,
    VertexAttribArrayType = 0x8625,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    VertexAttribArrayDivisor = 0x88FE,
    VertexAttribArrayInteger = 0x88FD,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    VertexAttribBinding = 0x82D4,
    VertexAttribRelativeOffset = 0x82D5,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum VertexAttribPropertyARB

enum class VertexAttribType : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Byte = 0x1400,
    Fixed = 0x140C,
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
    UnsignedByte = 0x1401,
    UnsignedInt = 0x1405,
    UnsignedShort = 0x1403,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    HalfFloat = 0x140B,
    Int2101010Rev = 0x8D9F,
    UnsignedInt10f11f11fRev = 0x8C3B,
    UnsignedInt2101010Rev = 0x8368,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum VertexAttribType

enum class VertexBufferObjectParameter : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    BufferSize = 0x8764,
    BufferUsage = 0x8765,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    BufferAccessFlags = 0x911F,
    BufferMapped = 0x88BC,
    BufferMapLength = 0x9120,
    BufferMapOffset = 0x9121,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum VertexBufferObjectParameter

enum class VertexBufferObjectUsage : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    DynamicDraw = 0x88E8,
    StaticDraw = 0x88E4,
    StreamDraw = 0x88E0,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    DynamicCopy = 0x88EA,
    DynamicRead = 0x88E9,
    StaticCopy = 0x88E6,
    StaticRead = 0x88E5,
    StreamCopy = 0x88E2,
    StreamRead = 0x88E1,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum VertexBufferObjectUsage

enum class VertexPointerType : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum VertexPointerType

enum class VertexProvokingMode : core::EnumType
{
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
    FirstVertexConvention = 0x8E4D,
    LastVertexConvention = 0x8E4E,
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)
}; // enum VertexProvokingMode

// commands
#if defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
extern void getTexParameter(TextureTarget target, GetTextureParameter pname, float *params);
extern void readPixel(int32_t x, int32_t y, core::SizeType width, core::SizeType height, PixelFormat format, PixelType type, void *pixels);
extern bool isRenderbuffer(uint32_t renderbuffer);
extern void clearStencil(int32_t s);
extern void generateMipmap(TextureTarget target);
extern void stencilMask(uint32_t mask);
extern void drawArrays(PrimitiveType mode, int32_t first, core::SizeType count);
extern void stencilOpSeparate(StencilFaceDirection face, StencilOp sfail, StencilOp dpfail, StencilOp dppass);
extern bool isEnable(EnableCap cap);
extern void vertexAttrib1(uint32_t index, const float *v);
extern void getAttachedShaders(uint32_t program, core::SizeType maxCount, core::SizeType *count, uint32_t *shaders);
extern void vertexAttrib2(uint32_t index, const float *v);
extern void bindTexture(TextureTarget target, uint32_t texture);
extern void uniform4(int32_t location, core::SizeType count, const int32_t *value);
extern void uniform2(int32_t location, int32_t v0, int32_t v1);
extern void getVertexAttrib(uint32_t index, VertexAttribPropertyARB pname, int32_t *params);
extern void getShaderPrecisionFormat(ShaderType shadertype, PrecisionType precisiontype, int32_t *range, int32_t *precision);
extern void depthRange(float n, float f);
extern void getVertexAttribPointerv(uint32_t index, VertexAttribPointerPropertyARB pname, void **pointer);
extern void getBufferParameter(BufferTargetARB target, BufferPNameARB pname, int32_t *params);
extern void uniformMatrix3(int32_t location, core::SizeType count, bool transpose, const float *value);
extern void polygonOffset(float factor, float units);
extern void uniform1(int32_t location, float v0);
extern void uniformMatrix2(int32_t location, core::SizeType count, bool transpose, const float *value);
extern void uniform3(int32_t location, int32_t v0, int32_t v1, int32_t v2);
extern void blendEquation(BlendEquationModeEXT mode);
extern void stencilFunc(StencilFunction func, int32_t ref, uint32_t mask);
extern void pixelStore(PixelStoreParameter pname, int32_t param);
extern void vertexAttrib1(uint32_t index, float x);
extern void vertexAttrib3(uint32_t index, float x, float y, float z);
extern void uniform2(int32_t location, float v0, float v1);
extern uint32_t createShader(ShaderType type);
extern core::EnumType getError();
extern void frontFace(FrontFaceDirection mode);
extern void getActiveUniform(uint32_t program, uint32_t index, core::SizeType bufSize, core::SizeType *length, int32_t *size, UniformType *type, char *name);
extern void uniform3(int32_t location, core::SizeType count, const float *value);
extern void framebufferTexture2D(FramebufferTarget target, FramebufferAttachment attachment, TextureTarget textarget, uint32_t texture, int32_t level);
extern void bufferSubData(BufferTargetARB target, std::intptr_t offset, std::size_t size, const void *data);
extern void getIntegerv(GetPName pname, int32_t *data);
extern void vertexAttrib3(uint32_t index, const float *v);
extern void copyTexSubImage2D(TextureTarget target, int32_t level, int32_t xoffset, int32_t yoffset, int32_t x, int32_t y, core::SizeType width, core::SizeType height);
extern void stencilOp(StencilOp fail, StencilOp zfail, StencilOp zpass);
extern void uniform3(int32_t location, core::SizeType count, const int32_t *value);
extern void compileShader(uint32_t shader);
extern void texParameter(TextureTarget target, TextureParameterName pname, float param);
extern void getBooleanv(GetPName pname, bool *data);
extern void genRenderbuffers(core::SizeType n, uint32_t *renderbuffers);
extern void getShaderInfoLog(uint32_t shader, core::SizeType bufSize, core::SizeType *length, char *infoLog);
extern void compressedTexImage2D(TextureTarget target, int32_t level, InternalFormat internalformat, core::SizeType width, core::SizeType height, int32_t border, core::SizeType imageSize, const void *data);
extern void attachShader(uint32_t program, uint32_t shader);
extern void enableVertexAttribArray(uint32_t index);
extern void getRenderbufferParameter(RenderbufferTarget target, RenderbufferParameterName pname, int32_t *params);
extern void getUniform(uint32_t program, int32_t location, float *params);
extern void shaderBinary(core::SizeType count, const uint32_t *shaders, core::EnumType binaryformat, const void *binary, core::SizeType length);
extern void useProgram(uint32_t program);
extern void vertexAttribPointer(uint32_t index, int32_t size, VertexAttribPointerType type, bool normalized, core::SizeType stride, const void *pointer);
extern bool isProgram(uint32_t program);
extern void uniform2(int32_t location, core::SizeType count, const float *value);
extern bool isTexture(uint32_t texture);
extern void blendFuncSeparate(BlendingFactor sfactorRGB, BlendingFactor dfactorRGB, BlendingFactor sfactorAlpha, BlendingFactor dfactorAlpha);
extern void uniform1(int32_t location, int32_t v0);
extern void texParameter(TextureTarget target, TextureParameterName pname, const int32_t *params);
extern void disable(EnableCap cap);
extern void disableVertexAttribArray(uint32_t index);
extern const uint8_t *getString(StringName name);
extern bool isFramebuffer(uint32_t framebuffer);
extern void hint(HintTarget target, HintMode mode);
extern void sampleCoverage(float value, bool invert);
extern void uniform1(int32_t location, core::SizeType count, const int32_t *value);
extern void getActiveAttri(uint32_t program, uint32_t index, core::SizeType bufSize, core::SizeType *length, int32_t *size, AttributeType *type, char *name);
extern void bindFramebuffer(FramebufferTarget target, uint32_t framebuffer);
extern uint32_t createProgram();
extern void viewport(int32_t x, int32_t y, core::SizeType width, core::SizeType height);
extern void stencilMaskSeparate(StencilFaceDirection face, uint32_t mask);
extern void genTextures(core::SizeType n, uint32_t *textures);
extern void depthMask(bool flag);
extern void clearColor(float red, float green, float blue, float alpha);
extern void validateProgram(uint32_t program);
extern void colorMask(bool red, bool green, bool blue, bool alpha);
extern void lineWidth(float width);
extern bool isShader(uint32_t shader);
extern void texSubImage2D(TextureTarget target, int32_t level, int32_t xoffset, int32_t yoffset, core::SizeType width, core::SizeType height, PixelFormat format, PixelType type, const void *pixels);
extern void drawElements(PrimitiveType mode, core::SizeType count, DrawElementsType type, const void *indices);
extern int32_t getAttribLocation(uint32_t program, const char *name);
extern void texImage2D(TextureTarget target, int32_t level, InternalFormat internalformat, core::SizeType width, core::SizeType height, int32_t border, PixelFormat format, PixelType type, const void *pixels);
extern void releaseShaderCompiler();
extern void depthFunc(DepthFunction func);
extern void getVertexAttrib(uint32_t index, VertexAttribPropertyARB pname, float *params);
extern void getTexParameter(TextureTarget target, GetTextureParameter pname, int32_t *params);
extern void bufferData(BufferTargetARB target, std::size_t size, const void *data, BufferUsageARB usage);
extern void clearDepth(float d);
extern void getUniform(uint32_t program, int32_t location, int32_t *params);
extern void uniform3(int32_t location, float v0, float v1, float v2);
extern void deleteFramebuffers(core::SizeType n, const uint32_t *framebuffers);
extern void shaderSource(uint32_t shader, core::SizeType count, const char *const*string, const int32_t *length);
extern void stencilFuncSeparate(StencilFaceDirection face, StencilFunction func, int32_t ref, uint32_t mask);
extern void getShaderSource(uint32_t shader, core::SizeType bufSize, core::SizeType *length, char *source);
extern void genBuffers(core::SizeType n, uint32_t *buffers);
extern void linkProgram(uint32_t program);
extern void uniform2(int32_t location, core::SizeType count, const int32_t *value);
extern void deleteShader(uint32_t shader);
extern void finish();
extern void bindBuffer(BufferTargetARB target, uint32_t buffer);
extern void copyTexImage2D(TextureTarget target, int32_t level, InternalFormat internalformat, int32_t x, int32_t y, core::SizeType width, core::SizeType height, int32_t border);
extern void vertexAttrib4(uint32_t index, float x, float y, float z, float w);
extern void deleteRenderbuffers(core::SizeType n, const uint32_t *renderbuffers);
extern void renderbufferStorage(RenderbufferTarget target, InternalFormat internalformat, core::SizeType width, core::SizeType height);
extern core::EnumType checkFramebufferStatus(FramebufferTarget target);
extern void deleteTextures(core::SizeType n, const uint32_t *textures);
extern void genFramebuffers(core::SizeType n, uint32_t *framebuffers);
extern void vertexAttrib2(uint32_t index, float x, float y);
extern void blendFunc(BlendingFactor sfactor, BlendingFactor dfactor);
extern void texParameter(TextureTarget target, TextureParameterName pname, const float *params);
extern void compressedTexSubImage2D(TextureTarget target, int32_t level, int32_t xoffset, int32_t yoffset, core::SizeType width, core::SizeType height, PixelFormat format, core::SizeType imageSize, const void *data);
extern void cullFace(CullFaceMode mode);
extern void clear(core::Bitfield<ClearBufferMask> mask);
extern void detachShader(uint32_t program, uint32_t shader);
extern void flush();
extern void getFramebufferAttachmentParameter(FramebufferTarget target, FramebufferAttachment attachment, FramebufferAttachmentParameterName pname, int32_t *params);
extern void framebufferRenderbuffer(FramebufferTarget target, FramebufferAttachment attachment, RenderbufferTarget renderbuffertarget, uint32_t renderbuffer);
extern void blendEquationSeparate(BlendEquationModeEXT modeRGB, BlendEquationModeEXT modeAlpha);
extern void bindAttribLocation(uint32_t program, uint32_t index, const char *name);
extern void activeTexture(TextureUnit texture);
extern bool isBuffer(uint32_t buffer);
extern void deleteProgram(uint32_t program);
extern void uniformMatrix4(int32_t location, core::SizeType count, bool transpose, const float *value);
extern void scissor(int32_t x, int32_t y, core::SizeType width, core::SizeType height);
extern void uniform4(int32_t location, core::SizeType count, const float *value);
extern void getProgramInfoLog(uint32_t program, core::SizeType bufSize, core::SizeType *length, char *infoLog);
extern void getShader(uint32_t shader, ShaderParameterName pname, int32_t *params);
extern void bindRenderbuffer(RenderbufferTarget target, uint32_t renderbuffer);
extern void enable(EnableCap cap);
extern int32_t getUniformLocation(uint32_t program, const char *name);
extern void uniform1(int32_t location, core::SizeType count, const float *value);
extern void getFloatv(GetPName pname, float *data);
extern void uniform4(int32_t location, int32_t v0, int32_t v1, int32_t v2, int32_t v3);
extern void blendColor(float red, float green, float blue, float alpha);
extern void uniform4(int32_t location, float v0, float v1, float v2, float v3);
extern void deleteBuffers(core::SizeType n, const uint32_t *buffers);
extern void vertexAttrib4(uint32_t index, const float *v);
extern void texParameter(TextureTarget target, TextureParameterName pname, int32_t param);
extern void getProgram(uint32_t program, ProgramPropertyARB pname, int32_t *params);
#endif // defined(API_LEVEL_GL_ES_VERSION_2_0) || defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
extern void readBuffer(ReadBufferMode src);
extern void vertexAttribI4(uint32_t index, int32_t x, int32_t y, int32_t z, int32_t w);
extern void uniform1(int32_t location, uint32_t v0);
extern void compressedTexImage3D(TextureTarget target, int32_t level, InternalFormat internalformat, core::SizeType width, core::SizeType height, core::SizeType depth, int32_t border, core::SizeType imageSize, const void *data);
extern void *mapBufferRange(BufferTargetARB target, std::intptr_t offset, std::size_t length, core::Bitfield<MapBufferAccessMask> access);
extern void samplerParameter(uint32_t sampler, SamplerParameterF pname, float param);
extern void bindVertexArray(uint32_t array);
extern void getBufferPointerv(BufferTargetARB target, BufferPointerNameARB pname, void **params);
extern void getInteger64i_v(core::EnumType target, uint32_t index, int64_t *data);
extern void uniform4(int32_t location, uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3);
extern void getVertexAttribI(uint32_t index, VertexAttribEnum pname, uint32_t *params);
extern void uniform3(int32_t location, uint32_t v0, uint32_t v1, uint32_t v2);
extern void bindSampler(uint32_t unit, uint32_t sampler);
extern void texStorage3D(TextureTarget target, core::SizeType levels, InternalFormat internalformat, core::SizeType width, core::SizeType height, core::SizeType depth);
extern void drawRangeElements(PrimitiveType mode, uint32_t start, uint32_t end, core::SizeType count, DrawElementsType type, const void *indices);
extern void uniform3(int32_t location, core::SizeType count, const uint32_t *value);
extern void getSamplerParameter(uint32_t sampler, SamplerParameterI pname, int32_t *params);
extern void uniformMatrix3x2(int32_t location, core::SizeType count, bool transpose, const float *value);
extern bool isVertexArray(uint32_t array);
extern void samplerParameter(uint32_t sampler, SamplerParameterI pname, const int32_t *param);
extern void texStorage2D(TextureTarget target, core::SizeType levels, InternalFormat internalformat, core::SizeType width, core::SizeType height);
extern void deleteSync(core::Sync sync);
extern void getActiveUniformBlockName(uint32_t program, uint32_t uniformBlockIndex, core::SizeType bufSize, core::SizeType *length, char *uniformBlockName);
extern void flushMappedBufferRange(BufferTargetARB target, std::intptr_t offset, std::size_t length);
extern void beginTransformFeedback(PrimitiveType primitiveMode);
extern void clearBuffer(Buffer buffer, int32_t drawbuffer, const float *value);
extern void uniform4(int32_t location, core::SizeType count, const uint32_t *value);
extern void clearBuffer(Buffer buffer, int32_t drawbuffer, const int32_t *value);
extern void clearBuffer(Buffer buffer, int32_t drawbuffer, const uint32_t *value);
extern void blitFramebuffer(int32_t srcX0, int32_t srcY0, int32_t srcX1, int32_t srcY1, int32_t dstX0, int32_t dstY0, int32_t dstX1, int32_t dstY1, core::Bitfield<ClearBufferMask> mask, BlitFramebufferFilter filter);
extern void genSampler(core::SizeType count, uint32_t *samplers);
extern void programParameter(uint32_t program, ProgramParameterPName pname, int32_t value);
extern void copyTexSubImage3D(TextureTarget target, int32_t level, int32_t xoffset, int32_t yoffset, int32_t zoffset, int32_t x, int32_t y, core::SizeType width, core::SizeType height);
extern void getSamplerParameter(uint32_t sampler, SamplerParameterF pname, float *params);
extern void renderbufferStorageMultisample(RenderbufferTarget target, core::SizeType samples, InternalFormat internalformat, core::SizeType width, core::SizeType height);
extern void invalidateFramebuffer(FramebufferTarget target, core::SizeType numAttachments, const FramebufferAttachment *attachments);
extern int32_t getFragDataLocation(uint32_t program, const char *name);
extern bool isSync(core::Sync sync);
extern bool isSampler(uint32_t sampler);
extern void uniformMatrix3x4(int32_t location, core::SizeType count, bool transpose, const float *value);
extern void drawArraysInstance(PrimitiveType mode, int32_t first, core::SizeType count, core::SizeType instancecount);
extern void getQueryObject(uint32_t id, QueryObjectParameterName pname, uint32_t *params);
extern void getTransformFeedbackVarying(uint32_t program, uint32_t index, core::SizeType bufSize, core::SizeType *length, core::SizeType *size, GlslTypeToken *type, char *name);
extern void uniformMatrix4x2(int32_t location, core::SizeType count, bool transpose, const float *value);
extern void texImage3D(TextureTarget target, int32_t level, int32_t internalformat, core::SizeType width, core::SizeType height, core::SizeType depth, int32_t border, PixelFormat format, PixelType type, const void *pixels);
extern bool unmapBuffer(BufferTargetARB target);
extern void compressedTexSubImage3D(TextureTarget target, int32_t level, int32_t xoffset, int32_t yoffset, int32_t zoffset, core::SizeType width, core::SizeType height, core::SizeType depth, PixelFormat format, core::SizeType imageSize, const void *data);
extern void genVertexArrays(core::SizeType n, uint32_t *arrays);
extern void deleteSampler(core::SizeType count, const uint32_t *samplers);
extern void getActiveUniformBlock(uint32_t program, uint32_t uniformBlockIndex, UniformBlockPName pname, int32_t *params);
extern void getBufferParameter(BufferTargetARB target, BufferPNameARB pname, int64_t *params);
extern void genTransformFeedback(core::SizeType n, uint32_t *ids);
extern void copyBufferSubData(CopyBufferSubDataTarget readTarget, CopyBufferSubDataTarget writeTarget, std::intptr_t readOffset, std::intptr_t writeOffset, std::size_t size);
extern void uniform2(int32_t location, uint32_t v0, uint32_t v1);
extern void uniformMatrix2x3(int32_t location, core::SizeType count, bool transpose, const float *value);
extern void vertexAttribIPointer(uint32_t index, int32_t size, VertexAttribPointerType type, core::SizeType stride, const void *pointer);
extern bool isQuery(uint32_t id);
extern void getUniform(uint32_t program, int32_t location, uint32_t *params);
extern void getInteger64v(GetPName pname, int64_t *data);
extern void getInternalformat(TextureTarget target, InternalFormat internalformat, InternalFormatPName pname, core::SizeType bufSize, int32_t *params);
extern void beginQuery(QueryTarget target, uint32_t id);
extern void deleteVertexArrays(core::SizeType n, const uint32_t *arrays);
extern void getQuery(QueryTarget target, QueryParameterName pname, int32_t *params);
extern void genQuerie(core::SizeType n, uint32_t *ids);
extern const uint8_t *getString(StringName name, uint32_t index);
extern bool isTransformFeedback(uint32_t id);
extern void waitSync(core::Sync sync, uint32_t flags, uint64_t timeout);
extern void pauseTransformFeedback();
extern void getActiveUniforms(uint32_t program, core::SizeType uniformCount, const uint32_t *uniformIndices, UniformPName pname, int32_t *params);
extern void getSync(core::Sync sync, SyncParameterName pname, core::SizeType bufSize, core::SizeType *length, int32_t *values);
extern uint32_t getUniformBlockIndex(uint32_t program, const char *uniformBlockName);
extern void vertexAttribI4(uint32_t index, uint32_t x, uint32_t y, uint32_t z, uint32_t w);
extern void uniform1(int32_t location, core::SizeType count, const uint32_t *value);
extern void drawElementsInstance(PrimitiveType mode, core::SizeType count, DrawElementsType type, const void *indices, core::SizeType instancecount);
extern void invalidateSubFramebuffer(FramebufferTarget target, core::SizeType numAttachments, const FramebufferAttachment *attachments, int32_t x, int32_t y, core::SizeType width, core::SizeType height);
extern core::EnumType clientWaitSync(core::Sync sync, core::Bitfield<SyncObjectMask> flags, uint64_t timeout);
extern void resumeTransformFeedback();
extern void bindBufferBase(BufferTargetARB target, uint32_t index, uint32_t buffer);
extern void samplerParameter(uint32_t sampler, SamplerParameterF pname, const float *param);
extern void vertexAttribI4(uint32_t index, const int32_t *v);
extern void uniformMatrix2x4(int32_t location, core::SizeType count, bool transpose, const float *value);
extern void vertexAttribI4(uint32_t index, const uint32_t *v);
extern core::Sync fenceSync(SyncCondition condition, uint32_t flags);
extern void transformFeedbackVarying(uint32_t program, core::SizeType count, const char *const*varyings, TransformFeedbackBufferMode bufferMode);
extern void endQuery(QueryTarget target);
extern void clearBufferf(Buffer buffer, int32_t drawbuffer, float depth, int32_t stencil);
extern void getUniformIndice(uint32_t program, core::SizeType uniformCount, const char *const*uniformNames, uint32_t *uniformIndices);
extern void getProgramBinary(uint32_t program, core::SizeType bufSize, core::SizeType *length, core::EnumType *binaryFormat, void *binary);
extern void drawBuffers(core::SizeType n, const DrawBufferMode *bufs);
extern void deleteQuerie(core::SizeType n, const uint32_t *ids);
extern void uniformBlockBinding(uint32_t program, uint32_t uniformBlockIndex, uint32_t uniformBlockBinding);
extern void uniform2(int32_t location, core::SizeType count, const uint32_t *value);
extern void getVertexAttribI(uint32_t index, VertexAttribEnum pname, int32_t *params);
extern void endTransformFeedback();
extern void uniformMatrix4x3(int32_t location, core::SizeType count, bool transpose, const float *value);
extern void bindTransformFeedback(BindTransformFeedbackTarget target, uint32_t id);
extern void bindBufferRange(BufferTargetARB target, uint32_t index, uint32_t buffer, std::intptr_t offset, std::size_t size);
extern void getIntegeri_v(core::EnumType target, uint32_t index, int32_t *data);
extern void framebufferTextureLayer(FramebufferTarget target, FramebufferAttachment attachment, uint32_t texture, int32_t level, int32_t layer);
extern void samplerParameter(uint32_t sampler, SamplerParameterI pname, int32_t param);
extern void texSubImage3D(TextureTarget target, int32_t level, int32_t xoffset, int32_t yoffset, int32_t zoffset, core::SizeType width, core::SizeType height, core::SizeType depth, PixelFormat format, PixelType type, const void *pixels);
extern void programBinary(uint32_t program, core::EnumType binaryFormat, const void *binary, core::SizeType length);
extern void vertexAttribDivisor(uint32_t index, uint32_t divisor);
extern void deleteTransformFeedback(core::SizeType n, const uint32_t *ids);
#endif // defined(API_LEVEL_GL_ES_VERSION_3_0) || defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
extern void genProgramPipeline(core::SizeType n, uint32_t *pipelines);
extern void texStorage2DMultisample(TextureTarget target, core::SizeType samples, InternalFormat internalformat, core::SizeType width, core::SizeType height, bool fixedsamplelocations);
extern void programUniform2(uint32_t program, int32_t location, core::SizeType count, const float *value);
extern uint32_t createShaderProgramv(ShaderType type, core::SizeType count, const char *const*strings);
extern void getTexLevelParameter(TextureTarget target, int32_t level, GetTextureParameter pname, int32_t *params);
extern void programUniformMatrix3x2(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float *value);
extern void getBooleani_v(BufferTargetARB target, uint32_t index, bool *data);
extern void programUniformMatrix2(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float *value);
extern void deleteProgramPipeline(core::SizeType n, const uint32_t *pipelines);
extern void getProgramResource(uint32_t program, ProgramInterface programInterface, uint32_t index, core::SizeType propCount, const ProgramResourceProperty *props, core::SizeType bufSize, core::SizeType *length, int32_t *params);
extern void programUniform4(uint32_t program, int32_t location, core::SizeType count, const int32_t *value);
extern void bindVertexBuffer(uint32_t bindingindex, uint32_t buffer, std::intptr_t offset, core::SizeType stride);
extern void programUniform3(uint32_t program, int32_t location, float v0, float v1, float v2);
extern void programUniform3(uint32_t program, int32_t location, core::SizeType count, const float *value);
extern void programUniformMatrix4(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float *value);
extern void useProgramStage(uint32_t pipeline, core::Bitfield<UseProgramStageMask> stages, uint32_t program);
extern void vertexAttribFormat(uint32_t attribindex, int32_t size, VertexAttribType type, bool normalized, uint32_t relativeoffset);
extern void getProgramPipeline(uint32_t pipeline, PipelineParameterName pname, int32_t *params);
extern void programUniform1(uint32_t program, int32_t location, float v0);
extern void programUniform1(uint32_t program, int32_t location, uint32_t v0);
extern void dispatchCompute(uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z);
extern void programUniformMatrix2x4(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float *value);
extern uint32_t getProgramResourceIndex(uint32_t program, ProgramInterface programInterface, const char *name);
extern void sampleMask(uint32_t maskNumber, uint32_t mask);
extern void programUniformMatrix3(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float *value);
extern void bindImageTexture(uint32_t unit, uint32_t texture, int32_t level, bool layered, int32_t layer, BufferAccessARB access, InternalFormat format);
extern void programUniform2(uint32_t program, int32_t location, float v0, float v1);
extern void vertexAttribIFormat(uint32_t attribindex, int32_t size, VertexAttribIType type, uint32_t relativeoffset);
extern void getProgramResourceName(uint32_t program, ProgramInterface programInterface, uint32_t index, core::SizeType bufSize, core::SizeType *length, char *name);
extern void programUniform4(uint32_t program, int32_t location, uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3);
extern void programUniform2(uint32_t program, int32_t location, core::SizeType count, const uint32_t *value);
extern void programUniform1(uint32_t program, int32_t location, core::SizeType count, const float *value);
extern void drawElementsIndirect(PrimitiveType mode, DrawElementsType type, const void *indirect);
extern void programUniform1(uint32_t program, int32_t location, core::SizeType count, const int32_t *value);
extern void getProgramPipelineInfoLog(uint32_t pipeline, core::SizeType bufSize, core::SizeType *length, char *infoLog);
extern void programUniform4(uint32_t program, int32_t location, float v0, float v1, float v2, float v3);
extern void getMultisample(GetMultisamplePNameNV pname, uint32_t index, float *val);
extern void bindProgramPipeline(uint32_t pipeline);
extern void validateProgramPipeline(uint32_t pipeline);
extern void programUniformMatrix4x3(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float *value);
extern void framebufferParameter(FramebufferTarget target, FramebufferParameterName pname, int32_t param);
extern void programUniform4(uint32_t program, int32_t location, int32_t v0, int32_t v1, int32_t v2, int32_t v3);
extern void programUniform4(uint32_t program, int32_t location, core::SizeType count, const uint32_t *value);
extern void programUniformMatrix2x3(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float *value);
extern void programUniformMatrix3x4(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float *value);
extern void drawArraysIndirect(PrimitiveType mode, const void *indirect);
extern void programUniform1(uint32_t program, int32_t location, core::SizeType count, const uint32_t *value);
extern void memoryBarrierByRegion(core::Bitfield<MemoryBarrierMask> barriers);
extern void programUniformMatrix4x2(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float *value);
extern void programUniform3(uint32_t program, int32_t location, int32_t v0, int32_t v1, int32_t v2);
extern int32_t getProgramResourceLocation(uint32_t program, ProgramInterface programInterface, const char *name);
extern void programUniform3(uint32_t program, int32_t location, uint32_t v0, uint32_t v1, uint32_t v2);
extern void dispatchComputeIndirect(std::intptr_t indirect);
extern void getTexLevelParameter(TextureTarget target, int32_t level, GetTextureParameter pname, float *params);
extern void getProgramInterface(uint32_t program, ProgramInterface programInterface, ProgramInterfacePName pname, int32_t *params);
extern void memoryBarrier(core::Bitfield<MemoryBarrierMask> barriers);
extern void programUniform1(uint32_t program, int32_t location, int32_t v0);
extern bool isProgramPipeline(uint32_t pipeline);
extern void programUniform4(uint32_t program, int32_t location, core::SizeType count, const float *value);
extern void programUniform3(uint32_t program, int32_t location, core::SizeType count, const uint32_t *value);
extern void activeShaderProgram(uint32_t pipeline, uint32_t program);
extern void getFramebufferParameter(FramebufferTarget target, FramebufferAttachmentParameterName pname, int32_t *params);
extern void programUniform2(uint32_t program, int32_t location, core::SizeType count, const int32_t *value);
extern void programUniform2(uint32_t program, int32_t location, uint32_t v0, uint32_t v1);
extern void programUniform3(uint32_t program, int32_t location, core::SizeType count, const int32_t *value);
extern void programUniform2(uint32_t program, int32_t location, int32_t v0, int32_t v1);
extern void vertexBindingDivisor(uint32_t bindingindex, uint32_t divisor);
extern void vertexAttribBinding(uint32_t attribindex, uint32_t bindingindex);
#endif // defined(API_LEVEL_GL_ES_VERSION_3_1) || defined(API_LEVEL_GL_ES_VERSION_3_2)
#if defined(API_LEVEL_GL_ES_VERSION_3_2)
extern void copyImageSubData(uint32_t srcName, TextureTarget srcTarget, int32_t srcLevel, int32_t srcX, int32_t srcY, int32_t srcZ, uint32_t dstName, TextureTarget dstTarget, int32_t dstLevel, int32_t dstX, int32_t dstY, int32_t dstZ, core::SizeType srcWidth, core::SizeType srcHeight, core::SizeType srcDepth);
extern void debugMessageInsert(DebugSource source, DebugType type, uint32_t id, DebugSeverity severity, core::SizeType length, const char *buf);
extern void disable(EnableCap target, uint32_t index);
extern void getnUniform(uint32_t program, int32_t location, core::SizeType bufSize, int32_t *params);
extern uint32_t getDebugMessageLog(uint32_t count, core::SizeType bufSize, DebugSource *sources, DebugType *types, uint32_t *ids, DebugSeverity *severities, core::SizeType *lengths, char *messageLog);
extern void popDebugGroup();
extern core::EnumType getGraphicsResetStatus();
extern void samplerParameterI(uint32_t sampler, SamplerParameterI pname, const int32_t *param);
extern void blendFuncSeparate(uint32_t buf, BlendingFactor srcRGB, BlendingFactor dstRGB, BlendingFactor srcAlpha, BlendingFactor dstAlpha);
extern void getSamplerParameterI(uint32_t sampler, SamplerParameterI pname, uint32_t *params);
extern void getObjectPtrLabel(const void *ptr, core::SizeType bufSize, core::SizeType *length, char *label);
extern void texBufferRange(TextureTarget target, InternalFormat internalformat, uint32_t buffer, std::intptr_t offset, std::size_t size);
extern void texStorage3DMultisample(TextureTarget target, core::SizeType samples, InternalFormat internalformat, core::SizeType width, core::SizeType height, core::SizeType depth, bool fixedsamplelocations);
extern void enable(EnableCap target, uint32_t index);
extern void getPointerv(GetPointervPName pname, void **params);
extern void patchParameter(PatchParameterName pname, int32_t value);
extern void drawElementsBaseVertex(PrimitiveType mode, core::SizeType count, DrawElementsType type, const void *indices, int32_t basevertex);
extern void texBuffer(TextureTarget target, InternalFormat internalformat, uint32_t buffer);
extern void getSamplerParameterI(uint32_t sampler, SamplerParameterI pname, int32_t *params);
extern void getnUniform(uint32_t program, int32_t location, core::SizeType bufSize, float *params);
extern void objectLabel(ObjectIdentifier identifier, uint32_t name, core::SizeType length, const char *label);
extern void getnUniform(uint32_t program, int32_t location, core::SizeType bufSize, uint32_t *params);
extern void getTexParameterI(TextureTarget target, GetTextureParameter pname, int32_t *params);
extern void drawElementsInstancedBaseVertex(PrimitiveType mode, core::SizeType count, DrawElementsType type, const void *indices, core::SizeType instancecount, int32_t basevertex);
extern void texParameterI(TextureTarget target, TextureParameterName pname, const int32_t *params);
extern void blendEquationSeparate(uint32_t buf, BlendEquationModeEXT modeRGB, BlendEquationModeEXT modeAlpha);
extern void getObjectLabel(ObjectIdentifier identifier, uint32_t name, core::SizeType bufSize, core::SizeType *length, char *label);
extern void getTexParameterI(TextureTarget target, GetTextureParameter pname, uint32_t *params);
extern void texParameterI(TextureTarget target, TextureParameterName pname, const uint32_t *params);
extern void objectPtrLabel(const void *ptr, core::SizeType length, const char *label);
extern void primitiveBoundingBox(float minX, float minY, float minZ, float minW, float maxX, float maxY, float maxZ, float maxW);
extern void debugMessageCallback(core::DebugProc callback, const void *userParam);
extern void framebufferTexture(FramebufferTarget target, FramebufferAttachment attachment, uint32_t texture, int32_t level);
extern void blendFunc(uint32_t buf, BlendingFactor src, BlendingFactor dst);
extern void readnPixel(int32_t x, int32_t y, core::SizeType width, core::SizeType height, PixelFormat format, PixelType type, core::SizeType bufSize, void *data);
extern void colorMask(uint32_t index, bool r, bool g, bool b, bool a);
extern void debugMessageControl(DebugSource source, DebugType type, DebugSeverity severity, core::SizeType count, const uint32_t *ids, bool enabled);
extern void pushDebugGroup(DebugSource source, uint32_t id, core::SizeType length, const char *message);
extern void blendEquation(uint32_t buf, BlendEquationModeEXT mode);
extern bool isEnabled(EnableCap target, uint32_t index);
extern void samplerParameterI(uint32_t sampler, SamplerParameterI pname, const uint32_t *param);
extern void drawRangeElementsBaseVertex(PrimitiveType mode, uint32_t start, uint32_t end, core::SizeType count, DrawElementsType type, const void *indices, int32_t basevertex);
extern void blendBarrier();
extern void minSampleShading(float value);
#endif // defined(API_LEVEL_GL_ES_VERSION_3_2)

}
