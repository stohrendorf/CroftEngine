#pragma once
#include "soglb_core.hpp"
#include "gl_enabled_features.hpp"
namespace gl
{
// API feature levels
// #define API_LEVEL_GL_VERSION_1_0
// #define API_LEVEL_GL_VERSION_1_1
// #define API_LEVEL_GL_VERSION_1_2
// #define API_LEVEL_GL_VERSION_1_3
// #define API_LEVEL_GL_VERSION_1_4
// #define API_LEVEL_GL_VERSION_1_5
// #define API_LEVEL_GL_VERSION_2_0
// #define API_LEVEL_GL_VERSION_2_1
// #define API_LEVEL_GL_VERSION_3_0
// #define API_LEVEL_GL_VERSION_3_1
// #define API_LEVEL_GL_VERSION_3_2_core
// #define API_LEVEL_GL_VERSION_3_3_compatibility
// #define API_LEVEL_GL_VERSION_3_3_core
// #define API_LEVEL_GL_VERSION_4_0_compatibility
// #define API_LEVEL_GL_VERSION_4_0_core
// #define API_LEVEL_GL_VERSION_4_1_compatibility
// #define API_LEVEL_GL_VERSION_4_1_core
// #define API_LEVEL_GL_VERSION_4_2_compatibility
// #define API_LEVEL_GL_VERSION_4_2_core
// #define API_LEVEL_GL_VERSION_4_3_compatibility
// #define API_LEVEL_GL_VERSION_4_3_core
// #define API_LEVEL_GL_VERSION_4_4_compatibility
// #define API_LEVEL_GL_VERSION_4_4_core
// #define API_LEVEL_GL_VERSION_4_5_compatibility
// #define API_LEVEL_GL_VERSION_4_5_core
// #define API_LEVEL_GL_VERSION_4_6_compatibility
// #define API_LEVEL_GL_VERSION_4_6_core

// API extensions
// #define WITH_API_EXTENSION_GL_3DFX_multisample
// #define WITH_API_EXTENSION_GL_3DFX_tbuffer
// #define WITH_API_EXTENSION_GL_3DFX_texture_compression_FXT1
// #define WITH_API_EXTENSION_GL_AMD_blend_minmax_factor
// #define WITH_API_EXTENSION_GL_AMD_debug_output
// #define WITH_API_EXTENSION_GL_AMD_depth_clamp_separate
// #define WITH_API_EXTENSION_GL_AMD_draw_buffers_blend
// #define WITH_API_EXTENSION_GL_AMD_framebuffer_multisample_advanced
// #define WITH_API_EXTENSION_GL_AMD_framebuffer_sample_positions
// #define WITH_API_EXTENSION_GL_AMD_gpu_shader_half_float
// #define WITH_API_EXTENSION_GL_AMD_gpu_shader_int64
// #define WITH_API_EXTENSION_GL_AMD_interleaved_elements
// #define WITH_API_EXTENSION_GL_AMD_multi_draw_indirect
// #define WITH_API_EXTENSION_GL_AMD_name_gen_delete
// #define WITH_API_EXTENSION_GL_AMD_occlusion_query_event
// #define WITH_API_EXTENSION_GL_AMD_performance_monitor
// #define WITH_API_EXTENSION_GL_AMD_pinned_memory
// #define WITH_API_EXTENSION_GL_AMD_query_buffer_object
// #define WITH_API_EXTENSION_GL_AMD_sample_positions
// #define WITH_API_EXTENSION_GL_AMD_seamless_cubemap_per_texture
// #define WITH_API_EXTENSION_GL_AMD_sparse_texture
// #define WITH_API_EXTENSION_GL_AMD_stencil_operation_extended
// #define WITH_API_EXTENSION_GL_AMD_transform_feedback4
// #define WITH_API_EXTENSION_GL_AMD_vertex_shader_tessellator
// #define WITH_API_EXTENSION_GL_APPLE_aux_depth_stencil
// #define WITH_API_EXTENSION_GL_APPLE_client_storage
// #define WITH_API_EXTENSION_GL_APPLE_element_array
// #define WITH_API_EXTENSION_GL_APPLE_fence
// #define WITH_API_EXTENSION_GL_APPLE_float_pixels
// #define WITH_API_EXTENSION_GL_APPLE_flush_buffer_range
// #define WITH_API_EXTENSION_GL_APPLE_object_purgeable
// #define WITH_API_EXTENSION_GL_APPLE_rgb_422
// #define WITH_API_EXTENSION_GL_APPLE_row_bytes
// #define WITH_API_EXTENSION_GL_APPLE_specular_vector
// #define WITH_API_EXTENSION_GL_APPLE_texture_range
// #define WITH_API_EXTENSION_GL_APPLE_transform_hint
// #define WITH_API_EXTENSION_GL_APPLE_vertex_array_object
// #define WITH_API_EXTENSION_GL_APPLE_vertex_array_range
// #define WITH_API_EXTENSION_GL_APPLE_vertex_program_evaluators
// #define WITH_API_EXTENSION_GL_APPLE_ycbcr_422
// #define WITH_API_EXTENSION_GL_ARB_ES2_compatibility
// #define WITH_API_EXTENSION_GL_ARB_ES3_1_compatibility
// #define WITH_API_EXTENSION_GL_ARB_ES3_2_compatibility
// #define WITH_API_EXTENSION_GL_ARB_ES3_compatibility
// #define WITH_API_EXTENSION_GL_ARB_base_instance
// #define WITH_API_EXTENSION_GL_ARB_bindless_texture
// #define WITH_API_EXTENSION_GL_ARB_blend_func_extended
// #define WITH_API_EXTENSION_GL_ARB_buffer_storage
// #define WITH_API_EXTENSION_GL_ARB_cl_event
// #define WITH_API_EXTENSION_GL_ARB_clear_buffer_object
// #define WITH_API_EXTENSION_GL_ARB_clear_texture
// #define WITH_API_EXTENSION_GL_ARB_clip_control
// #define WITH_API_EXTENSION_GL_ARB_color_buffer_float
// #define WITH_API_EXTENSION_GL_ARB_compressed_texture_pixel_storage
// #define WITH_API_EXTENSION_GL_ARB_compute_shader
// #define WITH_API_EXTENSION_GL_ARB_compute_variable_group_size
// #define WITH_API_EXTENSION_GL_ARB_conditional_render_inverted
// #define WITH_API_EXTENSION_GL_ARB_copy_buffer
// #define WITH_API_EXTENSION_GL_ARB_copy_image
// #define WITH_API_EXTENSION_GL_ARB_cull_distance
// #define WITH_API_EXTENSION_GL_ARB_debug_output
// #define WITH_API_EXTENSION_GL_ARB_depth_buffer_float
// #define WITH_API_EXTENSION_GL_ARB_depth_clamp
// #define WITH_API_EXTENSION_GL_ARB_depth_texture
// #define WITH_API_EXTENSION_GL_ARB_direct_state_access
// #define WITH_API_EXTENSION_GL_ARB_draw_buffers
// #define WITH_API_EXTENSION_GL_ARB_draw_buffers_blend
// #define WITH_API_EXTENSION_GL_ARB_draw_elements_base_vertex
// #define WITH_API_EXTENSION_GL_ARB_draw_indirect
// #define WITH_API_EXTENSION_GL_ARB_draw_instanced
// #define WITH_API_EXTENSION_GL_ARB_enhanced_layouts
// #define WITH_API_EXTENSION_GL_ARB_explicit_uniform_location
// #define WITH_API_EXTENSION_GL_ARB_fragment_program
// #define WITH_API_EXTENSION_GL_ARB_fragment_shader
// #define WITH_API_EXTENSION_GL_ARB_framebuffer_no_attachments
// #define WITH_API_EXTENSION_GL_ARB_framebuffer_object
// #define WITH_API_EXTENSION_GL_ARB_framebuffer_sRGB
// #define WITH_API_EXTENSION_GL_ARB_geometry_shader4
// #define WITH_API_EXTENSION_GL_ARB_get_program_binary
// #define WITH_API_EXTENSION_GL_ARB_get_texture_sub_image
// #define WITH_API_EXTENSION_GL_ARB_gl_spirv
// #define WITH_API_EXTENSION_GL_ARB_gpu_shader5
// #define WITH_API_EXTENSION_GL_ARB_gpu_shader_fp64
// #define WITH_API_EXTENSION_GL_ARB_gpu_shader_int64
// #define WITH_API_EXTENSION_GL_ARB_half_float_pixel
// #define WITH_API_EXTENSION_GL_ARB_half_float_vertex
// #define WITH_API_EXTENSION_GL_ARB_imaging
// #define WITH_API_EXTENSION_GL_ARB_indirect_parameters
// #define WITH_API_EXTENSION_GL_ARB_instanced_arrays
// #define WITH_API_EXTENSION_GL_ARB_internalformat_query
// #define WITH_API_EXTENSION_GL_ARB_internalformat_query2
// #define WITH_API_EXTENSION_GL_ARB_invalidate_subdata
// #define WITH_API_EXTENSION_GL_ARB_map_buffer_alignment
// #define WITH_API_EXTENSION_GL_ARB_map_buffer_range
// #define WITH_API_EXTENSION_GL_ARB_matrix_palette
// #define WITH_API_EXTENSION_GL_ARB_multi_bind
// #define WITH_API_EXTENSION_GL_ARB_multi_draw_indirect
// #define WITH_API_EXTENSION_GL_ARB_multisample
// #define WITH_API_EXTENSION_GL_ARB_multitexture
// #define WITH_API_EXTENSION_GL_ARB_occlusion_query
// #define WITH_API_EXTENSION_GL_ARB_occlusion_query2
// #define WITH_API_EXTENSION_GL_ARB_parallel_shader_compile
// #define WITH_API_EXTENSION_GL_ARB_pipeline_statistics_query
// #define WITH_API_EXTENSION_GL_ARB_pixel_buffer_object
// #define WITH_API_EXTENSION_GL_ARB_point_parameters
// #define WITH_API_EXTENSION_GL_ARB_point_sprite
// #define WITH_API_EXTENSION_GL_ARB_polygon_offset_clamp
// #define WITH_API_EXTENSION_GL_ARB_program_interface_query
// #define WITH_API_EXTENSION_GL_ARB_provoking_vertex
// #define WITH_API_EXTENSION_GL_ARB_query_buffer_object
// #define WITH_API_EXTENSION_GL_ARB_robustness
// #define WITH_API_EXTENSION_GL_ARB_sample_locations
// #define WITH_API_EXTENSION_GL_ARB_sample_shading
// #define WITH_API_EXTENSION_GL_ARB_sampler_objects
// #define WITH_API_EXTENSION_GL_ARB_seamless_cube_map
// #define WITH_API_EXTENSION_GL_ARB_seamless_cubemap_per_texture
// #define WITH_API_EXTENSION_GL_ARB_separate_shader_objects
// #define WITH_API_EXTENSION_GL_ARB_shader_atomic_counters
// #define WITH_API_EXTENSION_GL_ARB_shader_image_load_store
// #define WITH_API_EXTENSION_GL_ARB_shader_objects
// #define WITH_API_EXTENSION_GL_ARB_shader_storage_buffer_object
// #define WITH_API_EXTENSION_GL_ARB_shader_subroutine
// #define WITH_API_EXTENSION_GL_ARB_shading_language_100
// #define WITH_API_EXTENSION_GL_ARB_shading_language_include
// #define WITH_API_EXTENSION_GL_ARB_shadow
// #define WITH_API_EXTENSION_GL_ARB_shadow_ambient
// #define WITH_API_EXTENSION_GL_ARB_sparse_buffer
// #define WITH_API_EXTENSION_GL_ARB_sparse_texture
// #define WITH_API_EXTENSION_GL_ARB_spirv_extensions
// #define WITH_API_EXTENSION_GL_ARB_stencil_texturing
// #define WITH_API_EXTENSION_GL_ARB_sync
// #define WITH_API_EXTENSION_GL_ARB_tessellation_shader
// #define WITH_API_EXTENSION_GL_ARB_texture_barrier
// #define WITH_API_EXTENSION_GL_ARB_texture_border_clamp
// #define WITH_API_EXTENSION_GL_ARB_texture_buffer_object
// #define WITH_API_EXTENSION_GL_ARB_texture_buffer_object_rgb32
// #define WITH_API_EXTENSION_GL_ARB_texture_buffer_range
// #define WITH_API_EXTENSION_GL_ARB_texture_compression
// #define WITH_API_EXTENSION_GL_ARB_texture_compression_bptc
// #define WITH_API_EXTENSION_GL_ARB_texture_compression_rgtc
// #define WITH_API_EXTENSION_GL_ARB_texture_cube_map
// #define WITH_API_EXTENSION_GL_ARB_texture_cube_map_array
// #define WITH_API_EXTENSION_GL_ARB_texture_env_combine
// #define WITH_API_EXTENSION_GL_ARB_texture_env_dot3
// #define WITH_API_EXTENSION_GL_ARB_texture_filter_anisotropic
// #define WITH_API_EXTENSION_GL_ARB_texture_filter_minmax
// #define WITH_API_EXTENSION_GL_ARB_texture_float
// #define WITH_API_EXTENSION_GL_ARB_texture_gather
// #define WITH_API_EXTENSION_GL_ARB_texture_mirror_clamp_to_edge
// #define WITH_API_EXTENSION_GL_ARB_texture_mirrored_repeat
// #define WITH_API_EXTENSION_GL_ARB_texture_multisample
// #define WITH_API_EXTENSION_GL_ARB_texture_rectangle
// #define WITH_API_EXTENSION_GL_ARB_texture_rg
// #define WITH_API_EXTENSION_GL_ARB_texture_rgb10_a2ui
// #define WITH_API_EXTENSION_GL_ARB_texture_stencil8
// #define WITH_API_EXTENSION_GL_ARB_texture_storage
// #define WITH_API_EXTENSION_GL_ARB_texture_storage_multisample
// #define WITH_API_EXTENSION_GL_ARB_texture_swizzle
// #define WITH_API_EXTENSION_GL_ARB_texture_view
// #define WITH_API_EXTENSION_GL_ARB_timer_query
// #define WITH_API_EXTENSION_GL_ARB_transform_feedback2
// #define WITH_API_EXTENSION_GL_ARB_transform_feedback3
// #define WITH_API_EXTENSION_GL_ARB_transform_feedback_instanced
// #define WITH_API_EXTENSION_GL_ARB_transform_feedback_overflow_query
// #define WITH_API_EXTENSION_GL_ARB_transpose_matrix
// #define WITH_API_EXTENSION_GL_ARB_uniform_buffer_object
// #define WITH_API_EXTENSION_GL_ARB_vertex_array_bgra
// #define WITH_API_EXTENSION_GL_ARB_vertex_array_object
// #define WITH_API_EXTENSION_GL_ARB_vertex_attrib_64bit
// #define WITH_API_EXTENSION_GL_ARB_vertex_attrib_binding
// #define WITH_API_EXTENSION_GL_ARB_vertex_blend
// #define WITH_API_EXTENSION_GL_ARB_vertex_buffer_object
// #define WITH_API_EXTENSION_GL_ARB_vertex_program
// #define WITH_API_EXTENSION_GL_ARB_vertex_shader
// #define WITH_API_EXTENSION_GL_ARB_vertex_type_10f_11f_11f_rev
// #define WITH_API_EXTENSION_GL_ARB_vertex_type_2_10_10_10_rev
// #define WITH_API_EXTENSION_GL_ARB_viewport_array
// #define WITH_API_EXTENSION_GL_ARB_window_pos
// #define WITH_API_EXTENSION_GL_ATI_draw_buffers
// #define WITH_API_EXTENSION_GL_ATI_element_array
// #define WITH_API_EXTENSION_GL_ATI_envmap_bumpmap
// #define WITH_API_EXTENSION_GL_ATI_fragment_shader
// #define WITH_API_EXTENSION_GL_ATI_map_object_buffer
// #define WITH_API_EXTENSION_GL_ATI_meminfo
// #define WITH_API_EXTENSION_GL_ATI_pixel_format_float
// #define WITH_API_EXTENSION_GL_ATI_pn_triangles
// #define WITH_API_EXTENSION_GL_ATI_separate_stencil
// #define WITH_API_EXTENSION_GL_ATI_text_fragment_shader
// #define WITH_API_EXTENSION_GL_ATI_texture_env_combine3
// #define WITH_API_EXTENSION_GL_ATI_texture_float
// #define WITH_API_EXTENSION_GL_ATI_texture_mirror_once
// #define WITH_API_EXTENSION_GL_ATI_vertex_array_object
// #define WITH_API_EXTENSION_GL_ATI_vertex_attrib_array_object
// #define WITH_API_EXTENSION_GL_ATI_vertex_streams
// #define WITH_API_EXTENSION_GL_EXT_422_pixels
// #define WITH_API_EXTENSION_GL_EXT_EGL_image_storage
// #define WITH_API_EXTENSION_GL_EXT_abgr
// #define WITH_API_EXTENSION_GL_EXT_bgra
// #define WITH_API_EXTENSION_GL_EXT_bindable_uniform
// #define WITH_API_EXTENSION_GL_EXT_blend_color
// #define WITH_API_EXTENSION_GL_EXT_blend_equation_separate
// #define WITH_API_EXTENSION_GL_EXT_blend_func_separate
// #define WITH_API_EXTENSION_GL_EXT_blend_minmax
// #define WITH_API_EXTENSION_GL_EXT_blend_subtract
// #define WITH_API_EXTENSION_GL_EXT_clip_volume_hint
// #define WITH_API_EXTENSION_GL_EXT_cmyka
// #define WITH_API_EXTENSION_GL_EXT_color_subtable
// #define WITH_API_EXTENSION_GL_EXT_compiled_vertex_array
// #define WITH_API_EXTENSION_GL_EXT_convolution
// #define WITH_API_EXTENSION_GL_EXT_coordinate_frame
// #define WITH_API_EXTENSION_GL_EXT_copy_texture
// #define WITH_API_EXTENSION_GL_EXT_cull_vertex
// #define WITH_API_EXTENSION_GL_EXT_debug_label
// #define WITH_API_EXTENSION_GL_EXT_debug_marker
// #define WITH_API_EXTENSION_GL_EXT_depth_bounds_test
// #define WITH_API_EXTENSION_GL_EXT_direct_state_access
// #define WITH_API_EXTENSION_GL_EXT_draw_buffers2
// #define WITH_API_EXTENSION_GL_EXT_draw_instanced
// #define WITH_API_EXTENSION_GL_EXT_draw_range_elements
// #define WITH_API_EXTENSION_GL_EXT_external_buffer
// #define WITH_API_EXTENSION_GL_EXT_fog_coord
// #define WITH_API_EXTENSION_GL_EXT_framebuffer_blit
// #define WITH_API_EXTENSION_GL_EXT_framebuffer_multisample
// #define WITH_API_EXTENSION_GL_EXT_framebuffer_multisample_blit_scaled
// #define WITH_API_EXTENSION_GL_EXT_framebuffer_object
// #define WITH_API_EXTENSION_GL_EXT_framebuffer_sRGB
// #define WITH_API_EXTENSION_GL_EXT_geometry_shader4
// #define WITH_API_EXTENSION_GL_EXT_gpu_program_parameters
// #define WITH_API_EXTENSION_GL_EXT_gpu_shader4
// #define WITH_API_EXTENSION_GL_EXT_histogram
// #define WITH_API_EXTENSION_GL_EXT_index_array_formats
// #define WITH_API_EXTENSION_GL_EXT_index_func
// #define WITH_API_EXTENSION_GL_EXT_index_material
// #define WITH_API_EXTENSION_GL_EXT_light_texture
// #define WITH_API_EXTENSION_GL_EXT_memory_object
// #define WITH_API_EXTENSION_GL_EXT_memory_object_fd
// #define WITH_API_EXTENSION_GL_EXT_memory_object_win32
// #define WITH_API_EXTENSION_GL_EXT_multi_draw_arrays
// #define WITH_API_EXTENSION_GL_EXT_multisample
// #define WITH_API_EXTENSION_GL_EXT_packed_depth_stencil
// #define WITH_API_EXTENSION_GL_EXT_packed_float
// #define WITH_API_EXTENSION_GL_EXT_packed_pixels
// #define WITH_API_EXTENSION_GL_EXT_paletted_texture
// #define WITH_API_EXTENSION_GL_EXT_pixel_buffer_object
// #define WITH_API_EXTENSION_GL_EXT_pixel_transform
// #define WITH_API_EXTENSION_GL_EXT_point_parameters
// #define WITH_API_EXTENSION_GL_EXT_polygon_offset
// #define WITH_API_EXTENSION_GL_EXT_polygon_offset_clamp
// #define WITH_API_EXTENSION_GL_EXT_provoking_vertex
// #define WITH_API_EXTENSION_GL_EXT_raster_multisample
// #define WITH_API_EXTENSION_GL_EXT_rescale_normal
// #define WITH_API_EXTENSION_GL_EXT_secondary_color
// #define WITH_API_EXTENSION_GL_EXT_semaphore
// #define WITH_API_EXTENSION_GL_EXT_semaphore_fd
// #define WITH_API_EXTENSION_GL_EXT_semaphore_win32
// #define WITH_API_EXTENSION_GL_EXT_separate_shader_objects
// #define WITH_API_EXTENSION_GL_EXT_separate_specular_color
// #define WITH_API_EXTENSION_GL_EXT_shader_framebuffer_fetch
// #define WITH_API_EXTENSION_GL_EXT_shader_framebuffer_fetch_non_coherent
// #define WITH_API_EXTENSION_GL_EXT_shader_image_load_store
// #define WITH_API_EXTENSION_GL_EXT_shared_texture_palette
// #define WITH_API_EXTENSION_GL_EXT_stencil_clear_tag
// #define WITH_API_EXTENSION_GL_EXT_stencil_two_side
// #define WITH_API_EXTENSION_GL_EXT_stencil_wrap
// #define WITH_API_EXTENSION_GL_EXT_subtexture
// #define WITH_API_EXTENSION_GL_EXT_texture
// #define WITH_API_EXTENSION_GL_EXT_texture3D
// #define WITH_API_EXTENSION_GL_EXT_texture_array
// #define WITH_API_EXTENSION_GL_EXT_texture_buffer_object
// #define WITH_API_EXTENSION_GL_EXT_texture_compression_latc
// #define WITH_API_EXTENSION_GL_EXT_texture_compression_rgtc
// #define WITH_API_EXTENSION_GL_EXT_texture_compression_s3tc
// #define WITH_API_EXTENSION_GL_EXT_texture_cube_map
// #define WITH_API_EXTENSION_GL_EXT_texture_env_combine
// #define WITH_API_EXTENSION_GL_EXT_texture_env_dot3
// #define WITH_API_EXTENSION_GL_EXT_texture_filter_anisotropic
// #define WITH_API_EXTENSION_GL_EXT_texture_filter_minmax
// #define WITH_API_EXTENSION_GL_EXT_texture_integer
// #define WITH_API_EXTENSION_GL_EXT_texture_lod_bias
// #define WITH_API_EXTENSION_GL_EXT_texture_mirror_clamp
// #define WITH_API_EXTENSION_GL_EXT_texture_object
// #define WITH_API_EXTENSION_GL_EXT_texture_perturb_normal
// #define WITH_API_EXTENSION_GL_EXT_texture_sRGB
// #define WITH_API_EXTENSION_GL_EXT_texture_sRGB_R8
// #define WITH_API_EXTENSION_GL_EXT_texture_sRGB_decode
// #define WITH_API_EXTENSION_GL_EXT_texture_shared_exponent
// #define WITH_API_EXTENSION_GL_EXT_texture_snorm
// #define WITH_API_EXTENSION_GL_EXT_texture_swizzle
// #define WITH_API_EXTENSION_GL_EXT_timer_query
// #define WITH_API_EXTENSION_GL_EXT_transform_feedback
// #define WITH_API_EXTENSION_GL_EXT_vertex_array
// #define WITH_API_EXTENSION_GL_EXT_vertex_array_bgra
// #define WITH_API_EXTENSION_GL_EXT_vertex_attrib_64bit
// #define WITH_API_EXTENSION_GL_EXT_vertex_shader
// #define WITH_API_EXTENSION_GL_EXT_vertex_weighting
// #define WITH_API_EXTENSION_GL_EXT_win32_keyed_mutex
// #define WITH_API_EXTENSION_GL_EXT_window_rectangles
// #define WITH_API_EXTENSION_GL_EXT_x11_sync_object
// #define WITH_API_EXTENSION_GL_GREMEDY_frame_terminator
// #define WITH_API_EXTENSION_GL_GREMEDY_string_marker
// #define WITH_API_EXTENSION_GL_HP_convolution_border_modes
// #define WITH_API_EXTENSION_GL_HP_image_transform
// #define WITH_API_EXTENSION_GL_HP_occlusion_test
// #define WITH_API_EXTENSION_GL_HP_texture_lighting
// #define WITH_API_EXTENSION_GL_IBM_cull_vertex
// #define WITH_API_EXTENSION_GL_IBM_multimode_draw_arrays
// #define WITH_API_EXTENSION_GL_IBM_rasterpos_clip
// #define WITH_API_EXTENSION_GL_IBM_static_data
// #define WITH_API_EXTENSION_GL_IBM_texture_mirrored_repeat
// #define WITH_API_EXTENSION_GL_IBM_vertex_array_lists
// #define WITH_API_EXTENSION_GL_INGR_blend_func_separate
// #define WITH_API_EXTENSION_GL_INGR_color_clamp
// #define WITH_API_EXTENSION_GL_INGR_interlace_read
// #define WITH_API_EXTENSION_GL_INTEL_blackhole_render
// #define WITH_API_EXTENSION_GL_INTEL_conservative_rasterization
// #define WITH_API_EXTENSION_GL_INTEL_framebuffer_CMAA
// #define WITH_API_EXTENSION_GL_INTEL_map_texture
// #define WITH_API_EXTENSION_GL_INTEL_parallel_arrays
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
// #define WITH_API_EXTENSION_GL_MESAX_texture_stack
// #define WITH_API_EXTENSION_GL_MESA_pack_invert
// #define WITH_API_EXTENSION_GL_MESA_program_binary_formats
// #define WITH_API_EXTENSION_GL_MESA_resize_buffers
// #define WITH_API_EXTENSION_GL_MESA_tile_raster_order
// #define WITH_API_EXTENSION_GL_MESA_window_pos
// #define WITH_API_EXTENSION_GL_MESA_ycbcr_texture
// #define WITH_API_EXTENSION_GL_NVX_conditional_render
// #define WITH_API_EXTENSION_GL_NVX_gpu_memory_info
// #define WITH_API_EXTENSION_GL_NVX_linked_gpu_multicast
// #define WITH_API_EXTENSION_GL_NV_alpha_to_coverage_dither_control
// #define WITH_API_EXTENSION_GL_NV_bindless_multi_draw_indirect
// #define WITH_API_EXTENSION_GL_NV_bindless_multi_draw_indirect_count
// #define WITH_API_EXTENSION_GL_NV_bindless_texture
// #define WITH_API_EXTENSION_GL_NV_blend_equation_advanced
// #define WITH_API_EXTENSION_GL_NV_blend_equation_advanced_coherent
// #define WITH_API_EXTENSION_GL_NV_blend_minmax_factor
// #define WITH_API_EXTENSION_GL_NV_clip_space_w_scaling
// #define WITH_API_EXTENSION_GL_NV_command_list
// #define WITH_API_EXTENSION_GL_NV_compute_program5
// #define WITH_API_EXTENSION_GL_NV_conditional_render
// #define WITH_API_EXTENSION_GL_NV_conservative_raster
// #define WITH_API_EXTENSION_GL_NV_conservative_raster_dilate
// #define WITH_API_EXTENSION_GL_NV_conservative_raster_pre_snap
// #define WITH_API_EXTENSION_GL_NV_conservative_raster_pre_snap_triangles
// #define WITH_API_EXTENSION_GL_NV_copy_depth_to_color
// #define WITH_API_EXTENSION_GL_NV_copy_image
// #define WITH_API_EXTENSION_GL_NV_deep_texture3D
// #define WITH_API_EXTENSION_GL_NV_depth_buffer_float
// #define WITH_API_EXTENSION_GL_NV_depth_clamp
// #define WITH_API_EXTENSION_GL_NV_draw_texture
// #define WITH_API_EXTENSION_GL_NV_draw_vulkan_image
// #define WITH_API_EXTENSION_GL_NV_evaluators
// #define WITH_API_EXTENSION_GL_NV_explicit_multisample
// #define WITH_API_EXTENSION_GL_NV_fence
// #define WITH_API_EXTENSION_GL_NV_fill_rectangle
// #define WITH_API_EXTENSION_GL_NV_float_buffer
// #define WITH_API_EXTENSION_GL_NV_fog_distance
// #define WITH_API_EXTENSION_GL_NV_fragment_coverage_to_color
// #define WITH_API_EXTENSION_GL_NV_fragment_program
// #define WITH_API_EXTENSION_GL_NV_fragment_program2
// #define WITH_API_EXTENSION_GL_NV_framebuffer_mixed_samples
// #define WITH_API_EXTENSION_GL_NV_framebuffer_multisample_coverage
// #define WITH_API_EXTENSION_GL_NV_geometry_program4
// #define WITH_API_EXTENSION_GL_NV_gpu_multicast
// #define WITH_API_EXTENSION_GL_NV_gpu_program4
// #define WITH_API_EXTENSION_GL_NV_gpu_program5
// #define WITH_API_EXTENSION_GL_NV_gpu_shader5
// #define WITH_API_EXTENSION_GL_NV_half_float
// #define WITH_API_EXTENSION_GL_NV_internalformat_sample_query
// #define WITH_API_EXTENSION_GL_NV_light_max_exponent
// #define WITH_API_EXTENSION_GL_NV_memory_attachment
// #define WITH_API_EXTENSION_GL_NV_mesh_shader
// #define WITH_API_EXTENSION_GL_NV_multisample_coverage
// #define WITH_API_EXTENSION_GL_NV_multisample_filter_hint
// #define WITH_API_EXTENSION_GL_NV_occlusion_query
// #define WITH_API_EXTENSION_GL_NV_packed_depth_stencil
// #define WITH_API_EXTENSION_GL_NV_parameter_buffer_object
// #define WITH_API_EXTENSION_GL_NV_path_rendering
// #define WITH_API_EXTENSION_GL_NV_path_rendering_shared_edge
// #define WITH_API_EXTENSION_GL_NV_pixel_data_range
// #define WITH_API_EXTENSION_GL_NV_point_sprite
// #define WITH_API_EXTENSION_GL_NV_present_video
// #define WITH_API_EXTENSION_GL_NV_primitive_restart
// #define WITH_API_EXTENSION_GL_NV_query_resource
// #define WITH_API_EXTENSION_GL_NV_query_resource_tag
// #define WITH_API_EXTENSION_GL_NV_register_combiners
// #define WITH_API_EXTENSION_GL_NV_register_combiners2
// #define WITH_API_EXTENSION_GL_NV_representative_fragment_test
// #define WITH_API_EXTENSION_GL_NV_robustness_video_memory_purge
// #define WITH_API_EXTENSION_GL_NV_sample_locations
// #define WITH_API_EXTENSION_GL_NV_scissor_exclusive
// #define WITH_API_EXTENSION_GL_NV_shader_buffer_load
// #define WITH_API_EXTENSION_GL_NV_shader_buffer_store
// #define WITH_API_EXTENSION_GL_NV_shader_thread_group
// #define WITH_API_EXTENSION_GL_NV_shading_rate_image
// #define WITH_API_EXTENSION_GL_NV_tessellation_program5
// #define WITH_API_EXTENSION_GL_NV_texgen_emboss
// #define WITH_API_EXTENSION_GL_NV_texgen_reflection
// #define WITH_API_EXTENSION_GL_NV_texture_barrier
// #define WITH_API_EXTENSION_GL_NV_texture_env_combine4
// #define WITH_API_EXTENSION_GL_NV_texture_expand_normal
// #define WITH_API_EXTENSION_GL_NV_texture_multisample
// #define WITH_API_EXTENSION_GL_NV_texture_rectangle
// #define WITH_API_EXTENSION_GL_NV_texture_shader
// #define WITH_API_EXTENSION_GL_NV_texture_shader2
// #define WITH_API_EXTENSION_GL_NV_texture_shader3
// #define WITH_API_EXTENSION_GL_NV_transform_feedback
// #define WITH_API_EXTENSION_GL_NV_transform_feedback2
// #define WITH_API_EXTENSION_GL_NV_uniform_buffer_unified_memory
// #define WITH_API_EXTENSION_GL_NV_vdpau_interop
// #define WITH_API_EXTENSION_GL_NV_vdpau_interop2
// #define WITH_API_EXTENSION_GL_NV_vertex_array_range
// #define WITH_API_EXTENSION_GL_NV_vertex_array_range2
// #define WITH_API_EXTENSION_GL_NV_vertex_attrib_integer_64bit
// #define WITH_API_EXTENSION_GL_NV_vertex_buffer_unified_memory
// #define WITH_API_EXTENSION_GL_NV_vertex_program
// #define WITH_API_EXTENSION_GL_NV_vertex_program2_option
// #define WITH_API_EXTENSION_GL_NV_vertex_program3
// #define WITH_API_EXTENSION_GL_NV_vertex_program4
// #define WITH_API_EXTENSION_GL_NV_video_capture
// #define WITH_API_EXTENSION_GL_NV_viewport_swizzle
// #define WITH_API_EXTENSION_GL_OES_byte_coordinates
// #define WITH_API_EXTENSION_GL_OES_compressed_paletted_texture
// #define WITH_API_EXTENSION_GL_OES_fixed_point
// #define WITH_API_EXTENSION_GL_OES_query_matrix
// #define WITH_API_EXTENSION_GL_OES_read_format
// #define WITH_API_EXTENSION_GL_OES_single_precision
// #define WITH_API_EXTENSION_GL_OML_interlace
// #define WITH_API_EXTENSION_GL_OML_resample
// #define WITH_API_EXTENSION_GL_OML_subsample
// #define WITH_API_EXTENSION_GL_OVR_multiview
// #define WITH_API_EXTENSION_GL_PGI_misc_hints
// #define WITH_API_EXTENSION_GL_PGI_vertex_hints
// #define WITH_API_EXTENSION_GL_REND_screen_coordinates
// #define WITH_API_EXTENSION_GL_S3_s3tc
// #define WITH_API_EXTENSION_GL_SGIS_detail_texture
// #define WITH_API_EXTENSION_GL_SGIS_fog_function
// #define WITH_API_EXTENSION_GL_SGIS_generate_mipmap
// #define WITH_API_EXTENSION_GL_SGIS_multisample
// #define WITH_API_EXTENSION_GL_SGIS_pixel_texture
// #define WITH_API_EXTENSION_GL_SGIS_point_line_texgen
// #define WITH_API_EXTENSION_GL_SGIS_point_parameters
// #define WITH_API_EXTENSION_GL_SGIS_sharpen_texture
// #define WITH_API_EXTENSION_GL_SGIS_texture4D
// #define WITH_API_EXTENSION_GL_SGIS_texture_border_clamp
// #define WITH_API_EXTENSION_GL_SGIS_texture_color_mask
// #define WITH_API_EXTENSION_GL_SGIS_texture_edge_clamp
// #define WITH_API_EXTENSION_GL_SGIS_texture_filter4
// #define WITH_API_EXTENSION_GL_SGIS_texture_lod
// #define WITH_API_EXTENSION_GL_SGIS_texture_select
// #define WITH_API_EXTENSION_GL_SGIX_async
// #define WITH_API_EXTENSION_GL_SGIX_async_histogram
// #define WITH_API_EXTENSION_GL_SGIX_async_pixel
// #define WITH_API_EXTENSION_GL_SGIX_blend_alpha_minmax
// #define WITH_API_EXTENSION_GL_SGIX_calligraphic_fragment
// #define WITH_API_EXTENSION_GL_SGIX_clipmap
// #define WITH_API_EXTENSION_GL_SGIX_convolution_accuracy
// #define WITH_API_EXTENSION_GL_SGIX_depth_texture
// #define WITH_API_EXTENSION_GL_SGIX_flush_raster
// #define WITH_API_EXTENSION_GL_SGIX_fog_offset
// #define WITH_API_EXTENSION_GL_SGIX_fragment_lighting
// #define WITH_API_EXTENSION_GL_SGIX_framezoom
// #define WITH_API_EXTENSION_GL_SGIX_igloo_interface
// #define WITH_API_EXTENSION_GL_SGIX_instruments
// #define WITH_API_EXTENSION_GL_SGIX_interlace
// #define WITH_API_EXTENSION_GL_SGIX_ir_instrument1
// #define WITH_API_EXTENSION_GL_SGIX_list_priority
// #define WITH_API_EXTENSION_GL_SGIX_pixel_texture
// #define WITH_API_EXTENSION_GL_SGIX_pixel_tiles
// #define WITH_API_EXTENSION_GL_SGIX_polynomial_ffd
// #define WITH_API_EXTENSION_GL_SGIX_reference_plane
// #define WITH_API_EXTENSION_GL_SGIX_resample
// #define WITH_API_EXTENSION_GL_SGIX_scalebias_hint
// #define WITH_API_EXTENSION_GL_SGIX_shadow
// #define WITH_API_EXTENSION_GL_SGIX_shadow_ambient
// #define WITH_API_EXTENSION_GL_SGIX_sprite
// #define WITH_API_EXTENSION_GL_SGIX_subsample
// #define WITH_API_EXTENSION_GL_SGIX_tag_sample_buffer
// #define WITH_API_EXTENSION_GL_SGIX_texture_add_env
// #define WITH_API_EXTENSION_GL_SGIX_texture_coordinate_clamp
// #define WITH_API_EXTENSION_GL_SGIX_texture_lod_bias
// #define WITH_API_EXTENSION_GL_SGIX_texture_multi_buffer
// #define WITH_API_EXTENSION_GL_SGIX_texture_scale_bias
// #define WITH_API_EXTENSION_GL_SGIX_vertex_preclip
// #define WITH_API_EXTENSION_GL_SGIX_ycrcb
// #define WITH_API_EXTENSION_GL_SGIX_ycrcba
// #define WITH_API_EXTENSION_GL_SGI_color_matrix
// #define WITH_API_EXTENSION_GL_SGI_color_table
// #define WITH_API_EXTENSION_GL_SGI_texture_color_table
// #define WITH_API_EXTENSION_GL_SUNX_constant_data
// #define WITH_API_EXTENSION_GL_SUN_convolution_border_modes
// #define WITH_API_EXTENSION_GL_SUN_global_alpha
// #define WITH_API_EXTENSION_GL_SUN_mesh_array
// #define WITH_API_EXTENSION_GL_SUN_slice_accum
// #define WITH_API_EXTENSION_GL_SUN_triangle_list
// #define WITH_API_EXTENSION_GL_SUN_vertex
// #define WITH_API_EXTENSION_GL_WIN_phong_shading
// #define WITH_API_EXTENSION_GL_WIN_specular_fog

// enums
enum class AccumOp : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    Accum = 0x0100,
    Add = 0x0104,
    Load = 0x0101,
    Mult = 0x0103,
    Return = 0x0102,
#endif
};

enum class AlphaFunction : core::EnumType
{
    Always = 0x0207,
    Equal = 0x0202,
    Gequal = 0x0206,
    Greater = 0x0204,
    Lequal = 0x0203,
    Less = 0x0201,
    Never = 0x0200,
    Notequal = 0x0205,
};

enum class AtomicCounterBufferPName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    AtomicCounterBufferActiveAtomicCounters = 0x92C5,
    AtomicCounterBufferActiveAtomicCounterIndices = 0x92C6,
    AtomicCounterBufferBinding = 0x92C1,
    AtomicCounterBufferDataSize = 0x92C4,
    AtomicCounterBufferReferencedByFragmentShader = 0x92CB,
    AtomicCounterBufferReferencedByGeometryShader = 0x92CA,
    AtomicCounterBufferReferencedByTessControlShader = 0x92C8,
    AtomicCounterBufferReferencedByTessEvaluationShader = 0x92C9,
    AtomicCounterBufferReferencedByVertexShader = 0x92C7,
#endif
#if defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    AtomicCounterBufferReferencedByComputeShader = 0x90ED,
#endif
};

enum class AttribMask : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    AccumBufferBit = 0x00000200,
    AllAttribBits = 0xFFFFFFFF,
    CurrentBit = 0x00000001,
    EnableBit = 0x00002000,
    EvalBit = 0x00010000,
    FogBit = 0x00000080,
    HintBit = 0x00008000,
    LightingBit = 0x00000040,
    LineBit = 0x00000004,
    ListBit = 0x00020000,
    PixelModeBit = 0x00000020,
    PointBit = 0x00000002,
    PolygonBit = 0x00000008,
    PolygonStippleBit = 0x00000010,
    ScissorBit = 0x00080000,
    TextureBit = 0x00040000,
    TransformBit = 0x00001000,
    ViewportBit = 0x00000800,
#endif
    ColorBufferBit = 0x00004000,
    DepthBufferBit = 0x00000100,
    StencilBufferBit = 0x00000400,
#if defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    MultisampleBit = 0x20000000,
#endif
};
constexpr core::Bitfield<AttribMask> operator|(AttribMask left, AttribMask right) { return core::Bitfield<AttribMask>(left) | right;}

enum class AttributeType : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
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
#endif
#if defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    FloatMat2x3 = 0x8B65,
    FloatMat2x4 = 0x8B66,
    FloatMat3x2 = 0x8B67,
    FloatMat3x4 = 0x8B68,
    FloatMat4x2 = 0x8B69,
    FloatMat4x3 = 0x8B6A,
#endif
};

enum class BindTransformFeedbackTarget : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    TransformFeedback = 0x8E22,
#endif
};

enum class BlendEquationModeEXT : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    FuncAdd = 0x8006,
    FuncReverseSubtract = 0x800B,
    FuncSubtract = 0x800A,
    Max = 0x8008,
    Min = 0x8007,
#endif
};

enum class BlendingFactor : core::EnumType
{
    DstAlpha = 0x0304,
    DstColor = 0x0306,
    One = 1,
    OneMinusDstAlpha = 0x0305,
    OneMinusDstColor = 0x0307,
    OneMinusSrcAlpha = 0x0303,
    OneMinusSrcColor = 0x0301,
    SrcAlpha = 0x0302,
    SrcAlphaSaturate = 0x0308,
    SrcColor = 0x0300,
    Zero = 0,
#if defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ConstantAlpha = 0x8003,
    ConstantColor = 0x8001,
    OneMinusConstantAlpha = 0x8004,
    OneMinusConstantColor = 0x8002,
#endif
#if defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    Src1Alpha = 0x8589,
#endif
#if defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    OneMinusSrc1Alpha = 0x88FB,
    OneMinusSrc1Color = 0x88FA,
    Src1Color = 0x88F9,
#endif
};

enum class BlitFramebufferFilter : core::EnumType
{
    Linear = 0x2601,
    Nearest = 0x2600,
};

enum class Boolean : core::EnumType
{
    False = 0,
    True = 1,
};

enum class Buffer : core::EnumType
{
    Color = 0x1800,
    Depth = 0x1801,
    Stencil = 0x1802,
};

enum class BufferAccessARB : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ReadOnly = 0x88B8,
    ReadWrite = 0x88BA,
    WriteOnly = 0x88B9,
#endif
};

enum class BufferPNameARB : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    BufferAccess = 0x88BB,
    BufferMapped = 0x88BC,
    BufferSize = 0x8764,
    BufferUsage = 0x8765,
#endif
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    BufferAccessFlags = 0x911F,
    BufferMapLength = 0x9120,
    BufferMapOffset = 0x9121,
#endif
#if defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    BufferImmutableStorage = 0x821F,
    BufferStorageFlags = 0x8220,
#endif
};

enum class BufferPointerNameARB : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    BufferMapPointer = 0x88BD,
#endif
};

enum class BufferStorageMask : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    MapReadBit = 0x0001,
    MapWriteBit = 0x0002,
#endif
#if defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ClientStorageBit = 0x0200,
    DynamicStorageBit = 0x0100,
    MapCoherentBit = 0x0080,
    MapPersistentBit = 0x0040,
#endif
};
constexpr core::Bitfield<BufferStorageMask> operator|(BufferStorageMask left, BufferStorageMask right) { return core::Bitfield<BufferStorageMask>(left) | right;}

enum class BufferStorageTarget : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ArrayBuffer = 0x8892,
    ElementArrayBuffer = 0x8893,
#endif
#if defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    PixelPackBuffer = 0x88EB,
    PixelUnpackBuffer = 0x88EC,
#endif
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    TransformFeedbackBuffer = 0x8C8E,
#endif
#if defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    CopyReadBuffer = 0x8F36,
    CopyWriteBuffer = 0x8F37,
    TextureBuffer = 0x8C2A,
    UniformBuffer = 0x8A11,
#endif
#if defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    DrawIndirectBuffer = 0x8F3F,
#endif
#if defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    AtomicCounterBuffer = 0x92C0,
#endif
#if defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    DispatchIndirectBuffer = 0x90EE,
    ShaderStorageBuffer = 0x90D2,
#endif
#if defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    QueryBuffer = 0x9192,
#endif
};

enum class BufferTargetARB : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ArrayBuffer = 0x8892,
    ElementArrayBuffer = 0x8893,
#endif
#if defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    PixelPackBuffer = 0x88EB,
    PixelUnpackBuffer = 0x88EC,
#endif
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    TransformFeedbackBuffer = 0x8C8E,
#endif
#if defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    CopyReadBuffer = 0x8F36,
    CopyWriteBuffer = 0x8F37,
    TextureBuffer = 0x8C2A,
    UniformBuffer = 0x8A11,
#endif
#if defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    DrawIndirectBuffer = 0x8F3F,
#endif
#if defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    AtomicCounterBuffer = 0x92C0,
#endif
#if defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    DispatchIndirectBuffer = 0x90EE,
    ShaderStorageBuffer = 0x90D2,
#endif
#if defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    QueryBuffer = 0x9192,
#endif
#if defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ParameterBuffer = 0x80EE,
#endif
};

enum class BufferUsageARB : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    DynamicCopy = 0x88EA,
    DynamicDraw = 0x88E8,
    DynamicRead = 0x88E9,
    StaticCopy = 0x88E6,
    StaticDraw = 0x88E4,
    StaticRead = 0x88E5,
    StreamCopy = 0x88E2,
    StreamDraw = 0x88E0,
    StreamRead = 0x88E1,
#endif
};

enum class CheckFramebufferStatusTarget : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    DrawFramebuffer = 0x8CA9,
    Framebuffer = 0x8D40,
    ReadFramebuffer = 0x8CA8,
#endif
};

enum class ClampColorModeARB : core::EnumType
{
    False = 0,
    True = 1,
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    FixedOnly = 0x891D,
#endif
};

enum class ClampColorTargetARB : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ClampReadColor = 0x891C,
#endif
};

enum class ClearBufferMask : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    AccumBufferBit = 0x00000200,
#endif
    ColorBufferBit = 0x00004000,
    DepthBufferBit = 0x00000100,
    StencilBufferBit = 0x00000400,
};
constexpr core::Bitfield<ClearBufferMask> operator|(ClearBufferMask left, ClearBufferMask right) { return core::Bitfield<ClearBufferMask>(left) | right;}

enum class ClientAttribMask : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    ClientAllAttribBits = 0xFFFFFFFF,
    ClientPixelStoreBit = 0x00000001,
    ClientVertexArrayBit = 0x00000002,
#endif
};
constexpr core::Bitfield<ClientAttribMask> operator|(ClientAttribMask left, ClientAttribMask right) { return core::Bitfield<ClientAttribMask>(left) | right;}

enum class ClipControlDepth : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    NegativeOneToOne = 0x935E,
    ZeroToOne = 0x935F,
#endif
};

enum class ClipControlOrigin : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    LowerLeft = 0x8CA1,
    UpperLeft = 0x8CA2,
#endif
};

enum class ClipPlaneName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    ClipPlane0 = 0x3000,
    ClipPlane1 = 0x3001,
    ClipPlane2 = 0x3002,
    ClipPlane3 = 0x3003,
    ClipPlane4 = 0x3004,
    ClipPlane5 = 0x3005,
#endif
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ClipDistance0 = 0x3000,
    ClipDistance1 = 0x3001,
    ClipDistance2 = 0x3002,
    ClipDistance3 = 0x3003,
    ClipDistance4 = 0x3004,
    ClipDistance5 = 0x3005,
    ClipDistance6 = 0x3006,
    ClipDistance7 = 0x3007,
#endif
};

enum class ColorBuffer : core::EnumType
{
    Back = 0x0405,
    BackLeft = 0x0402,
    BackRight = 0x0403,
    Front = 0x0404,
    FrontAndBack = 0x0408,
    FrontLeft = 0x0400,
    FrontRight = 0x0401,
    Left = 0x0406,
    None = 0,
    Right = 0x0407,
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ColorAttachment0 = 0x8CE0,
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
#endif
};

enum class ColorMaterialFace : core::EnumType
{
    Back = 0x0405,
    Front = 0x0404,
    FrontAndBack = 0x0408,
};

enum class ColorMaterialParameter : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    Ambient = 0x1200,
    AmbientAndDiffuse = 0x1602,
    Diffuse = 0x1201,
    Emission = 0x1600,
    Specular = 0x1202,
#endif
};

enum class ColorPointerType : core::EnumType
{
    Byte = 0x1400,
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
    UnsignedByte = 0x1401,
    UnsignedInt = 0x1405,
    UnsignedShort = 0x1403,
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    Double = 0x140A,
#endif
};

enum class CombinerBiasNV : core::EnumType
{
    None = 0,
};

enum class CombinerScaleNV : core::EnumType
{
    None = 0,
};

enum class ConditionalRenderMode : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    QueryByRegionNoWait = 0x8E16,
    QueryByRegionWait = 0x8E15,
    QueryNoWait = 0x8E14,
    QueryWait = 0x8E13,
#endif
#if defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    QueryByRegionNoWaitInverted = 0x8E1A,
    QueryByRegionWaitInverted = 0x8E19,
    QueryNoWaitInverted = 0x8E18,
    QueryWaitInverted = 0x8E17,
#endif
};

enum class ContextFlagMask : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ContextFlagForwardCompatibleBit = 0x00000001,
#endif
#if defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ContextFlagDebugBit = 0x00000002,
#endif
#if defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ContextFlagRobustAccessBit = 0x00000004,
#endif
#if defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ContextFlagNoErrorBit = 0x00000008,
#endif
};
constexpr core::Bitfield<ContextFlagMask> operator|(ContextFlagMask left, ContextFlagMask right) { return core::Bitfield<ContextFlagMask>(left) | right;}

enum class ContextProfileMask : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ContextCompatibilityProfileBit = 0x00000002,
    ContextCoreProfileBit = 0x00000001,
#endif
};
constexpr core::Bitfield<ContextProfileMask> operator|(ContextProfileMask left, ContextProfileMask right) { return core::Bitfield<ContextProfileMask>(left) | right;}

enum class CopyBufferSubDataTarget : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ArrayBuffer = 0x8892,
    ElementArrayBuffer = 0x8893,
#endif
#if defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    PixelPackBuffer = 0x88EB,
    PixelUnpackBuffer = 0x88EC,
#endif
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    TransformFeedbackBuffer = 0x8C8E,
#endif
#if defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    CopyReadBuffer = 0x8F36,
    CopyWriteBuffer = 0x8F37,
    TextureBuffer = 0x8C2A,
    UniformBuffer = 0x8A11,
#endif
#if defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    DrawIndirectBuffer = 0x8F3F,
#endif
#if defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    AtomicCounterBuffer = 0x92C0,
#endif
#if defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    DispatchIndirectBuffer = 0x90EE,
    ShaderStorageBuffer = 0x90D2,
#endif
#if defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    QueryBuffer = 0x9192,
#endif
};

enum class CullFaceMode : core::EnumType
{
    Back = 0x0405,
    Front = 0x0404,
    FrontAndBack = 0x0408,
};

enum class DebugSeverity : core::EnumType
{
    DontCare = 0x1100,
#if defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    DebugSeverityHigh = 0x9146,
    DebugSeverityLow = 0x9148,
    DebugSeverityMedium = 0x9147,
    DebugSeverityNotification = 0x826B,
#endif
};

enum class DebugSource : core::EnumType
{
    DontCare = 0x1100,
#if defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    DebugSourceApi = 0x8246,
    DebugSourceApplication = 0x824A,
    DebugSourceOther = 0x824B,
    DebugSourceShaderCompiler = 0x8248,
    DebugSourceThirdParty = 0x8249,
    DebugSourceWindowSystem = 0x8247,
#endif
};

enum class DebugType : core::EnumType
{
    DontCare = 0x1100,
#if defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    DebugTypeDeprecatedBehavior = 0x824D,
    DebugTypeError = 0x824C,
    DebugTypeMarker = 0x8268,
    DebugTypeOther = 0x8251,
    DebugTypePerformance = 0x8250,
    DebugTypePopGroup = 0x826A,
    DebugTypePortability = 0x824F,
    DebugTypePushGroup = 0x8269,
    DebugTypeUndefinedBehavior = 0x824E,
#endif
};

enum class DepthFunction : core::EnumType
{
    Always = 0x0207,
    Equal = 0x0202,
    Gequal = 0x0206,
    Greater = 0x0204,
    Lequal = 0x0203,
    Less = 0x0201,
    Never = 0x0200,
    Notequal = 0x0205,
};

enum class DrawBufferMode : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    Aux0 = 0x0409,
    Aux1 = 0x040A,
    Aux2 = 0x040B,
    Aux3 = 0x040C,
#endif
    Back = 0x0405,
    BackLeft = 0x0402,
    BackRight = 0x0403,
    Front = 0x0404,
    FrontAndBack = 0x0408,
    FrontLeft = 0x0400,
    FrontRight = 0x0401,
    Left = 0x0406,
    None = 0,
    Right = 0x0407,
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ColorAttachment0 = 0x8CE0,
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
#endif
};

enum class DrawElementsType : core::EnumType
{
    UnsignedByte = 0x1401,
    UnsignedInt = 0x1405,
    UnsignedShort = 0x1403,
};

enum class EnableCap : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    AlphaTest = 0x0BC0,
    AutoNormal = 0x0D80,
    ClipPlane0 = 0x3000,
    ClipPlane1 = 0x3001,
    ClipPlane2 = 0x3002,
    ClipPlane3 = 0x3003,
    ClipPlane4 = 0x3004,
    ClipPlane5 = 0x3005,
    ColorMaterial = 0x0B57,
    Fog = 0x0B60,
    Light0 = 0x4000,
    Light1 = 0x4001,
    Light2 = 0x4002,
    Light3 = 0x4003,
    Light4 = 0x4004,
    Light5 = 0x4005,
    Light6 = 0x4006,
    Light7 = 0x4007,
    Lighting = 0x0B50,
    LineStipple = 0x0B24,
    Map1Color4 = 0x0D90,
    Map1Index = 0x0D91,
    Map1Normal = 0x0D92,
    Map1TextureCoord1 = 0x0D93,
    Map1TextureCoord2 = 0x0D94,
    Map1TextureCoord3 = 0x0D95,
    Map1TextureCoord4 = 0x0D96,
    Map1Vertex3 = 0x0D97,
    Map1Vertex4 = 0x0D98,
    Map2Color4 = 0x0DB0,
    Map2Index = 0x0DB1,
    Map2Normal = 0x0DB2,
    Map2TextureCoord1 = 0x0DB3,
    Map2TextureCoord2 = 0x0DB4,
    Map2TextureCoord3 = 0x0DB5,
    Map2TextureCoord4 = 0x0DB6,
    Map2Vertex3 = 0x0DB7,
    Map2Vertex4 = 0x0DB8,
    Normalize = 0x0BA1,
    PointSmooth = 0x0B10,
    PolygonStipple = 0x0B42,
    TextureGenQ = 0x0C63,
    TextureGenR = 0x0C62,
    TextureGenS = 0x0C60,
    TextureGenT = 0x0C61,
#endif
    Blend = 0x0BE2,
    CullFace = 0x0B44,
    DepthTest = 0x0B71,
    Dither = 0x0BD0,
    LineSmooth = 0x0B20,
    PolygonSmooth = 0x0B41,
    ScissorTest = 0x0C11,
    StencilTest = 0x0B90,
    Texture1d = 0x0DE0,
    Texture2d = 0x0DE1,
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    ColorArray = 0x8076,
    EdgeFlagArray = 0x8079,
    IndexArray = 0x8077,
    IndexLogicOp = 0x0BF1,
    NormalArray = 0x8075,
    TextureCoordArray = 0x8078,
#endif
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ColorLogicOp = 0x0BF2,
    PolygonOffsetFill = 0x8037,
    PolygonOffsetLine = 0x2A02,
    PolygonOffsetPoint = 0x2A01,
#endif
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    VertexArray = 0x8074,
#endif
#if defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    Multisample = 0x809D,
    SampleAlphaToCoverage = 0x809E,
    SampleAlphaToOne = 0x809F,
    SampleCoverage = 0x80A0,
#endif
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ClipDistance0 = 0x3000,
    ClipDistance1 = 0x3001,
    ClipDistance2 = 0x3002,
    ClipDistance3 = 0x3003,
    ClipDistance4 = 0x3004,
    ClipDistance5 = 0x3005,
    ClipDistance6 = 0x3006,
    ClipDistance7 = 0x3007,
    FramebufferSrgb = 0x8DB9,
    RasterizerDiscard = 0x8C89,
#endif
#if defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    PrimitiveRestart = 0x8F9D,
#endif
#if defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    DepthClamp = 0x864F,
    ProgramPointSize = 0x8642,
    SampleMask = 0x8E51,
    TextureCubeMapSeamless = 0x884F,
#endif
#if defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    SampleShading = 0x8C36,
#endif
#if defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    DebugOutput = 0x92E0,
    DebugOutputSynchronous = 0x8242,
    PrimitiveRestartFixedIndex = 0x8D69,
#endif
};

enum class ErrorCode : core::EnumType
{
    InvalidEnum = 0x0500,
    InvalidOperation = 0x0502,
    InvalidValue = 0x0501,
    NoError = 0,
    OutOfMemory = 0x0505,
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    StackOverflow = 0x0503,
    StackUnderflow = 0x0504,
#endif
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    InvalidFramebufferOperation = 0x0506,
#endif
};

enum class FeedBackToken : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    BitmapToken = 0x0704,
    CopyPixelToken = 0x0706,
    DrawPixelToken = 0x0705,
    LineResetToken = 0x0707,
    LineToken = 0x0702,
    PassThroughToken = 0x0700,
    PointToken = 0x0701,
    PolygonToken = 0x0703,
#endif
};

enum class FeedbackType : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    _2d = 0x0600,
    _3d = 0x0601,
    _3dColor = 0x0602,
    _3dColorTexture = 0x0603,
    _4dColorTexture = 0x0604,
#endif
};

enum class FogCoordinatePointerType : core::EnumType
{
    Float = 0x1406,
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    Double = 0x140A,
#endif
};

enum class FogMode : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    Exp = 0x0800,
    Exp2 = 0x0801,
#endif
    Linear = 0x2601,
};

enum class FogPName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    FogDensity = 0x0B62,
    FogEnd = 0x0B64,
    FogIndex = 0x0B61,
    FogMode = 0x0B65,
    FogStart = 0x0B63,
#endif
#if defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    FogCoordSrc = 0x8450,
#endif
};

enum class FogParameter : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    FogColor = 0x0B66,
    FogDensity = 0x0B62,
    FogEnd = 0x0B64,
    FogIndex = 0x0B61,
    FogMode = 0x0B65,
    FogStart = 0x0B63,
#endif
};

enum class FogPointerTypeEXT : core::EnumType
{
    Float = 0x1406,
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    Double = 0x140A,
#endif
};

enum class FogPointerTypeIBM : core::EnumType
{
    Float = 0x1406,
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    Double = 0x140A,
#endif
};

enum class FramebufferAttachment : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ColorAttachment0 = 0x8CE0,
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
    DepthAttachment = 0x8D00,
    DepthStencilAttachment = 0x821A,
    MaxColorAttachments = 0x8CDF,
#endif
};

enum class FramebufferAttachmentParameterName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    FramebufferAttachmentAlphaSize = 0x8215,
    FramebufferAttachmentBlueSize = 0x8214,
    FramebufferAttachmentColorEncoding = 0x8210,
    FramebufferAttachmentComponentType = 0x8211,
    FramebufferAttachmentDepthSize = 0x8216,
    FramebufferAttachmentGreenSize = 0x8213,
    FramebufferAttachmentObjectName = 0x8CD1,
    FramebufferAttachmentRedSize = 0x8212,
    FramebufferAttachmentStencilSize = 0x8217,
    FramebufferAttachmentTextureCubeMapFace = 0x8CD3,
    FramebufferAttachmentTextureLayer = 0x8CD4,
    FramebufferAttachmentTextureLevel = 0x8CD2,
#endif
#if defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    FramebufferAttachmentLayered = 0x8DA7,
#endif
};

enum class FramebufferParameterName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    FramebufferDefaultFixedSampleLocations = 0x9314,
    FramebufferDefaultHeight = 0x9311,
    FramebufferDefaultLayers = 0x9312,
    FramebufferDefaultSamples = 0x9313,
    FramebufferDefaultWidth = 0x9310,
#endif
};

enum class FramebufferStatus : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    FramebufferComplete = 0x8CD5,
    FramebufferIncompleteAttachment = 0x8CD6,
    FramebufferIncompleteDrawBuffer = 0x8CDB,
    FramebufferIncompleteMissingAttachment = 0x8CD7,
    FramebufferIncompleteMultisample = 0x8D56,
    FramebufferIncompleteReadBuffer = 0x8CDC,
    FramebufferUndefined = 0x8219,
    FramebufferUnsupported = 0x8CDD,
#endif
#if defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    FramebufferIncompleteLayerTargets = 0x8DA8,
#endif
};

enum class FramebufferTarget : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    DrawFramebuffer = 0x8CA9,
    Framebuffer = 0x8D40,
    ReadFramebuffer = 0x8CA8,
#endif
};

enum class FrontFaceDirection : core::EnumType
{
    Ccw = 0x0901,
    Cw = 0x0900,
};

enum class GetFramebufferParameter : core::EnumType
{
    Doublebuffer = 0x0C32,
    Stereo = 0x0C33,
#if defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    Samples = 0x80A9,
    SampleBuffers = 0x80A8,
#endif
#if defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ImplementationColorReadFormat = 0x8B9B,
    ImplementationColorReadType = 0x8B9A,
#endif
#if defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    FramebufferDefaultFixedSampleLocations = 0x9314,
    FramebufferDefaultHeight = 0x9311,
    FramebufferDefaultLayers = 0x9312,
    FramebufferDefaultSamples = 0x9313,
    FramebufferDefaultWidth = 0x9310,
#endif
};

enum class GetMapQuery : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    Coeff = 0x0A00,
    Domain = 0x0A02,
    Order = 0x0A01,
#endif
};

enum class GetMultisamplePNameNV : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    SamplePosition = 0x8E50,
#endif
};

enum class GetPName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    AccumAlphaBits = 0x0D5B,
    AccumBlueBits = 0x0D5A,
    AccumClearValue = 0x0B80,
    AccumGreenBits = 0x0D59,
    AccumRedBits = 0x0D58,
    AlphaBias = 0x0D1D,
    AlphaBits = 0x0D55,
    AlphaScale = 0x0D1C,
    AlphaTest = 0x0BC0,
    AlphaTestFunc = 0x0BC1,
    AlphaTestRef = 0x0BC2,
    AttribStackDepth = 0x0BB0,
    AutoNormal = 0x0D80,
    AuxBuffers = 0x0C00,
    BlueBias = 0x0D1B,
    BlueBits = 0x0D54,
    BlueScale = 0x0D1A,
    ClipPlane0 = 0x3000,
    ClipPlane1 = 0x3001,
    ClipPlane2 = 0x3002,
    ClipPlane3 = 0x3003,
    ClipPlane4 = 0x3004,
    ClipPlane5 = 0x3005,
    ColorMaterial = 0x0B57,
    ColorMaterialFace = 0x0B55,
    ColorMaterialParameter = 0x0B56,
    CurrentColor = 0x0B00,
    CurrentIndex = 0x0B01,
    CurrentNormal = 0x0B02,
    CurrentRasterColor = 0x0B04,
    CurrentRasterDistance = 0x0B09,
    CurrentRasterIndex = 0x0B05,
    CurrentRasterPosition = 0x0B07,
    CurrentRasterPositionValid = 0x0B08,
    CurrentRasterTextureCoords = 0x0B06,
    CurrentTextureCoords = 0x0B03,
    DepthBias = 0x0D1F,
    DepthBits = 0x0D56,
    DepthScale = 0x0D1E,
    EdgeFlag = 0x0B43,
    Fog = 0x0B60,
    FogColor = 0x0B66,
    FogDensity = 0x0B62,
    FogEnd = 0x0B64,
    FogHint = 0x0C54,
    FogIndex = 0x0B61,
    FogMode = 0x0B65,
    FogStart = 0x0B63,
    GreenBias = 0x0D19,
    GreenBits = 0x0D53,
    GreenScale = 0x0D18,
    IndexBits = 0x0D51,
    IndexClearValue = 0x0C20,
    IndexMode = 0x0C30,
    IndexOffset = 0x0D13,
    IndexShift = 0x0D12,
    IndexWritemask = 0x0C21,
    Light0 = 0x4000,
    Light1 = 0x4001,
    Light2 = 0x4002,
    Light3 = 0x4003,
    Light4 = 0x4004,
    Light5 = 0x4005,
    Light6 = 0x4006,
    Light7 = 0x4007,
    Lighting = 0x0B50,
    LightModelAmbient = 0x0B53,
    LightModelLocalViewer = 0x0B51,
    LightModelTwoSide = 0x0B52,
    LineStipple = 0x0B24,
    LineStipplePattern = 0x0B25,
    LineStippleRepeat = 0x0B26,
    ListBase = 0x0B32,
    ListIndex = 0x0B33,
    ListMode = 0x0B30,
    LogicOp = 0x0BF1,
    Map1Color4 = 0x0D90,
    Map1GridDomain = 0x0DD0,
    Map1GridSegments = 0x0DD1,
    Map1Index = 0x0D91,
    Map1Normal = 0x0D92,
    Map1TextureCoord1 = 0x0D93,
    Map1TextureCoord2 = 0x0D94,
    Map1TextureCoord3 = 0x0D95,
    Map1TextureCoord4 = 0x0D96,
    Map1Vertex3 = 0x0D97,
    Map1Vertex4 = 0x0D98,
    Map2Color4 = 0x0DB0,
    Map2GridDomain = 0x0DD2,
    Map2GridSegments = 0x0DD3,
    Map2Index = 0x0DB1,
    Map2Normal = 0x0DB2,
    Map2TextureCoord1 = 0x0DB3,
    Map2TextureCoord2 = 0x0DB4,
    Map2TextureCoord3 = 0x0DB5,
    Map2TextureCoord4 = 0x0DB6,
    Map2Vertex3 = 0x0DB7,
    Map2Vertex4 = 0x0DB8,
    MapColor = 0x0D10,
    MapStencil = 0x0D11,
    MatrixMode = 0x0BA0,
    MaxAttribStackDepth = 0x0D35,
    MaxClipPlanes = 0x0D32,
    MaxEvalOrder = 0x0D30,
    MaxLights = 0x0D31,
    MaxListNesting = 0x0B31,
    MaxModelviewStackDepth = 0x0D36,
    MaxNameStackDepth = 0x0D37,
    MaxPixelMapTable = 0x0D34,
    MaxProjectionStackDepth = 0x0D38,
    MaxTextureStackDepth = 0x0D39,
    ModelviewMatrix = 0x0BA6,
    ModelviewStackDepth = 0x0BA3,
    NameStackDepth = 0x0D70,
    Normalize = 0x0BA1,
    PerspectiveCorrectionHint = 0x0C50,
    PixelMapAToASize = 0x0CB9,
    PixelMapBToBSize = 0x0CB8,
    PixelMapGToGSize = 0x0CB7,
    PixelMapIToASize = 0x0CB5,
    PixelMapIToBSize = 0x0CB4,
    PixelMapIToGSize = 0x0CB3,
    PixelMapIToISize = 0x0CB0,
    PixelMapIToRSize = 0x0CB2,
    PixelMapRToRSize = 0x0CB6,
    PixelMapSToSSize = 0x0CB1,
    PointSmooth = 0x0B10,
    PointSmoothHint = 0x0C51,
    PolygonStipple = 0x0B42,
    ProjectionMatrix = 0x0BA7,
    ProjectionStackDepth = 0x0BA4,
    RedBias = 0x0D15,
    RedBits = 0x0D52,
    RedScale = 0x0D14,
    RenderMode = 0x0C40,
    RgbaMode = 0x0C31,
    ShadeModel = 0x0B54,
    StencilBits = 0x0D57,
    TextureGenQ = 0x0C63,
    TextureGenR = 0x0C62,
    TextureGenS = 0x0C60,
    TextureGenT = 0x0C61,
    TextureMatrix = 0x0BA8,
    TextureStackDepth = 0x0BA5,
    ZoomX = 0x0D16,
    ZoomY = 0x0D17,
#endif
    Blend = 0x0BE2,
    BlendDst = 0x0BE0,
    BlendSrc = 0x0BE1,
    ColorClearValue = 0x0C22,
    ColorWritemask = 0x0C23,
    CullFace = 0x0B44,
    CullFaceMode = 0x0B45,
    DepthClearValue = 0x0B73,
    DepthFunc = 0x0B74,
    DepthRange = 0x0B70,
    DepthTest = 0x0B71,
    DepthWritemask = 0x0B72,
    Dither = 0x0BD0,
    Doublebuffer = 0x0C32,
    DrawBuffer = 0x0C01,
    FrontFace = 0x0B46,
    LineSmooth = 0x0B20,
    LineSmoothHint = 0x0C52,
    LineWidth = 0x0B21,
    LineWidthGranularity = 0x0B23,
    LineWidthRange = 0x0B22,
    LogicOpMode = 0x0BF0,
    MaxTextureSize = 0x0D33,
    MaxViewportDims = 0x0D3A,
    PackAlignment = 0x0D05,
    PackLsbFirst = 0x0D01,
    PackRowLength = 0x0D02,
    PackSkipPixels = 0x0D04,
    PackSkipRows = 0x0D03,
    PackSwapBytes = 0x0D00,
    PointSize = 0x0B11,
    PointSizeGranularity = 0x0B13,
    PointSizeRange = 0x0B12,
    PolygonMode = 0x0B40,
    PolygonSmooth = 0x0B41,
    PolygonSmoothHint = 0x0C53,
    ReadBuffer = 0x0C02,
    ScissorBox = 0x0C10,
    ScissorTest = 0x0C11,
    StencilClearValue = 0x0B91,
    StencilFail = 0x0B94,
    StencilFunc = 0x0B92,
    StencilPassDepthFail = 0x0B95,
    StencilPassDepthPass = 0x0B96,
    StencilRef = 0x0B97,
    StencilTest = 0x0B90,
    StencilValueMask = 0x0B93,
    StencilWritemask = 0x0B98,
    Stereo = 0x0C33,
    SubpixelBits = 0x0D50,
    Texture1d = 0x0DE0,
    Texture2d = 0x0DE1,
    UnpackAlignment = 0x0CF5,
    UnpackLsbFirst = 0x0CF1,
    UnpackRowLength = 0x0CF2,
    UnpackSkipPixels = 0x0CF4,
    UnpackSkipRows = 0x0CF3,
    UnpackSwapBytes = 0x0CF0,
    Viewport = 0x0BA2,
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    ClientAttribStackDepth = 0x0BB1,
    ColorArray = 0x8076,
    ColorArraySize = 0x8081,
    ColorArrayStride = 0x8083,
    ColorArrayType = 0x8082,
    EdgeFlagArray = 0x8079,
    EdgeFlagArrayStride = 0x808C,
    FeedbackBufferSize = 0x0DF1,
    FeedbackBufferType = 0x0DF2,
    IndexArray = 0x8077,
    IndexArrayStride = 0x8086,
    IndexArrayType = 0x8085,
    IndexLogicOp = 0x0BF1,
    MaxClientAttribStackDepth = 0x0D3B,
    NormalArray = 0x8075,
    NormalArrayStride = 0x807F,
    NormalArrayType = 0x807E,
    SelectionBufferSize = 0x0DF4,
    TextureCoordArray = 0x8078,
    TextureCoordArraySize = 0x8088,
    TextureCoordArrayStride = 0x808A,
    TextureCoordArrayType = 0x8089,
    VertexArraySize = 0x807A,
    VertexArrayStride = 0x807C,
    VertexArrayType = 0x807B,
#endif
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ColorLogicOp = 0x0BF2,
    PolygonOffsetFactor = 0x8038,
    PolygonOffsetFill = 0x8037,
    PolygonOffsetLine = 0x2A02,
    PolygonOffsetPoint = 0x2A01,
    PolygonOffsetUnits = 0x2A00,
    TextureBinding1d = 0x8068,
    TextureBinding2d = 0x8069,
#endif
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    VertexArray = 0x8074,
#endif
#if defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    AliasedLineWidthRange = 0x846E,
    Max3dTextureSize = 0x8073,
    MaxElementsIndices = 0x80E9,
    MaxElementsVertices = 0x80E8,
    PackImageHeight = 0x806C,
    PackSkipImages = 0x806B,
    SmoothLineWidthGranularity = 0x0B23,
    SmoothLineWidthRange = 0x0B22,
    SmoothPointSizeGranularity = 0x0B13,
    SmoothPointSizeRange = 0x0B12,
    TextureBinding3d = 0x806A,
    UnpackImageHeight = 0x806E,
    UnpackSkipImages = 0x806D,
#endif
#if defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    AliasedPointSizeRange = 0x846D,
    LightModelColorControl = 0x81F8,
#endif
#if defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ActiveTexture = 0x84E0,
    CompressedTextureFormats = 0x86A3,
    MaxCubeMapTextureSize = 0x851C,
    NumCompressedTextureFormats = 0x86A2,
    Samples = 0x80A9,
    SampleBuffers = 0x80A8,
    SampleCoverageInvert = 0x80AB,
    SampleCoverageValue = 0x80AA,
    TextureBindingCubeMap = 0x8514,
    TextureCompressionHint = 0x84EF,
#endif
#if defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    BlendColor = 0x8005,
    BlendDstAlpha = 0x80CA,
    BlendDstRgb = 0x80C8,
    BlendSrcAlpha = 0x80CB,
    BlendSrcRgb = 0x80C9,
    MaxTextureLodBias = 0x84FD,
    PointFadeThresholdSize = 0x8128,
#endif
#if defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ArrayBufferBinding = 0x8894,
    ElementArrayBufferBinding = 0x8895,
#endif
#if defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    BlendEquationAlpha = 0x883D,
    BlendEquationRgb = 0x8009,
    CurrentProgram = 0x8B8D,
    FragmentShaderDerivativeHint = 0x8B8B,
    MaxCombinedTextureImageUnits = 0x8B4D,
    MaxDrawBuffers = 0x8824,
    MaxFragmentUniformComponents = 0x8B49,
    MaxTextureImageUnits = 0x8872,
    MaxVaryingFloats = 0x8B4B,
    MaxVertexAttribs = 0x8869,
    MaxVertexTextureImageUnits = 0x8B4C,
    MaxVertexUniformComponents = 0x8B4A,
    StencilBackFail = 0x8801,
    StencilBackFunc = 0x8800,
    StencilBackPassDepthFail = 0x8802,
    StencilBackPassDepthPass = 0x8803,
    StencilBackRef = 0x8CA3,
    StencilBackValueMask = 0x8CA4,
    StencilBackWritemask = 0x8CA5,
#endif
#if defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    PixelPackBufferBinding = 0x88ED,
    PixelUnpackBufferBinding = 0x88EF,
#endif
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ContextFlags = 0x821E,
    DrawFramebufferBinding = 0x8CA6,
    MajorVersion = 0x821B,
    MaxArrayTextureLayers = 0x88FF,
    MaxClipDistances = 0x0D32,
    MaxProgramTexelOffset = 0x8905,
    MaxRenderbufferSize = 0x84E8,
    MaxVaryingComponents = 0x8B4B,
    MinorVersion = 0x821C,
    MinProgramTexelOffset = 0x8904,
    NumExtensions = 0x821D,
    ReadFramebufferBinding = 0x8CAA,
    RenderbufferBinding = 0x8CA7,
    TextureBinding1dArray = 0x8C1C,
    TextureBinding2dArray = 0x8C1D,
    TransformFeedbackBufferBinding = 0x8C8F,
    TransformFeedbackBufferSize = 0x8C85,
    TransformFeedbackBufferStart = 0x8C84,
    VertexArrayBinding = 0x85B5,
#endif
#if defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    MaxCombinedFragmentUniformComponents = 0x8A33,
    MaxCombinedGeometryUniformComponents = 0x8A32,
    MaxCombinedUniformBlocks = 0x8A2E,
    MaxCombinedVertexUniformComponents = 0x8A31,
    MaxFragmentUniformBlocks = 0x8A2D,
    MaxGeometryUniformBlocks = 0x8A2C,
    MaxRectangleTextureSize = 0x84F8,
    MaxTextureBufferSize = 0x8C2B,
    MaxUniformBlockSize = 0x8A30,
    MaxUniformBufferBindings = 0x8A2F,
    MaxVertexUniformBlocks = 0x8A2B,
    PrimitiveRestartIndex = 0x8F9E,
    TextureBindingBuffer = 0x8C2C,
    TextureBindingRectangle = 0x84F6,
    UniformBufferBinding = 0x8A28,
    UniformBufferOffsetAlignment = 0x8A34,
    UniformBufferSize = 0x8A2A,
    UniformBufferStart = 0x8A29,
#endif
#if defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    MaxColorTextureSamples = 0x910E,
    MaxDepthTextureSamples = 0x910F,
    MaxFragmentInputComponents = 0x9125,
    MaxGeometryInputComponents = 0x9123,
    MaxGeometryOutputComponents = 0x9124,
    MaxGeometryTextureImageUnits = 0x8C29,
    MaxGeometryUniformComponents = 0x8DDF,
    MaxIntegerSamples = 0x9110,
    MaxSampleMaskWords = 0x8E59,
    MaxServerWaitTimeout = 0x9111,
    MaxVertexOutputComponents = 0x9122,
    ProgramPointSize = 0x8642,
    ProvokingVertex = 0x8E4F,
    TextureBinding2dMultisample = 0x9104,
    TextureBinding2dMultisampleArray = 0x9105,
#endif
#if defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    MaxDualSourceDrawBuffers = 0x88FC,
    SamplerBinding = 0x8919,
    Timestamp = 0x8E28,
#endif
#if defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ImplementationColorReadFormat = 0x8B9B,
    ImplementationColorReadType = 0x8B9A,
    LayerProvokingVertex = 0x825E,
    MaxFragmentUniformVectors = 0x8DFD,
    MaxVaryingVectors = 0x8DFC,
    MaxVertexUniformVectors = 0x8DFB,
    MaxViewports = 0x825B,
    NumProgramBinaryFormats = 0x87FE,
    NumShaderBinaryFormats = 0x8DF9,
    ProgramBinaryFormats = 0x87FF,
    ProgramPipelineBinding = 0x825A,
    ShaderCompiler = 0x8DFA,
    ViewportBoundsRange = 0x825D,
    ViewportIndexProvokingVertex = 0x825F,
    ViewportSubpixelBits = 0x825C,
#endif
#if defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    MaxCombinedAtomicCounters = 0x92D7,
    MaxFragmentAtomicCounters = 0x92D6,
    MaxGeometryAtomicCounters = 0x92D5,
    MaxTessControlAtomicCounters = 0x92D3,
    MaxTessEvaluationAtomicCounters = 0x92D4,
    MaxVertexAtomicCounters = 0x92D2,
    MinMapBufferAlignment = 0x90BC,
#endif
#if defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    DebugGroupStackDepth = 0x826D,
    DispatchIndirectBufferBinding = 0x90EF,
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
    MaxDebugGroupStackDepth = 0x826C,
    MaxElementIndex = 0x8D6B,
    MaxFragmentShaderStorageBlocks = 0x90DA,
    MaxFramebufferHeight = 0x9316,
    MaxFramebufferLayers = 0x9317,
    MaxFramebufferSamples = 0x9318,
    MaxFramebufferWidth = 0x9315,
    MaxGeometryShaderStorageBlocks = 0x90D7,
    MaxLabelLength = 0x82E8,
    MaxShaderStorageBufferBindings = 0x90DD,
    MaxTessControlShaderStorageBlocks = 0x90D8,
    MaxTessEvaluationShaderStorageBlocks = 0x90D9,
    MaxUniformLocations = 0x826E,
    MaxVertexAttribBindings = 0x82DA,
    MaxVertexAttribRelativeOffset = 0x82D9,
    MaxVertexShaderStorageBlocks = 0x90D6,
    ShaderStorageBufferBinding = 0x90D3,
    ShaderStorageBufferOffsetAlignment = 0x90DF,
    ShaderStorageBufferSize = 0x90D5,
    ShaderStorageBufferStart = 0x90D4,
    TextureBufferOffsetAlignment = 0x919F,
    VertexBindingDivisor = 0x82D6,
    VertexBindingOffset = 0x82D7,
    VertexBindingStride = 0x82D8,
#endif
};

enum class GetPixelMap : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    PixelMapAToA = 0x0C79,
    PixelMapBToB = 0x0C78,
    PixelMapGToG = 0x0C77,
    PixelMapIToA = 0x0C75,
    PixelMapIToB = 0x0C74,
    PixelMapIToG = 0x0C73,
    PixelMapIToI = 0x0C70,
    PixelMapIToR = 0x0C72,
    PixelMapRToR = 0x0C76,
    PixelMapSToS = 0x0C71,
#endif
};

enum class GetPointervPName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    ColorArrayPointer = 0x8090,
    EdgeFlagArrayPointer = 0x8093,
    FeedbackBufferPointer = 0x0DF0,
    IndexArrayPointer = 0x8091,
    NormalArrayPointer = 0x808F,
    SelectionBufferPointer = 0x0DF3,
    TextureCoordArrayPointer = 0x8092,
    VertexArrayPointer = 0x808E,
#endif
#if defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    DebugCallbackFunction = 0x8244,
    DebugCallbackUserParam = 0x8245,
#endif
};

enum class GetTextureParameter : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    TextureBorder = 0x1005,
    TextureComponents = 0x1003,
#endif
    TextureBorderColor = 0x1004,
    TextureHeight = 0x1001,
    TextureMagFilter = 0x2800,
    TextureMinFilter = 0x2801,
    TextureWidth = 0x1000,
    TextureWrapS = 0x2802,
    TextureWrapT = 0x2803,
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    TextureAlphaSize = 0x805F,
    TextureBlueSize = 0x805E,
    TextureGreenSize = 0x805D,
    TextureInternalFormat = 0x1003,
    TextureRedSize = 0x805C,
#endif
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    TextureIntensitySize = 0x8061,
    TextureLuminanceSize = 0x8060,
    TexturePriority = 0x8066,
    TextureResident = 0x8067,
#endif
};

enum class GlslTypeToken : core::EnumType
{
    Float = 0x1406,
    Int = 0x1404,
    UnsignedInt = 0x1405,
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    Double = 0x140A,
#endif
#if defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
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
    Sampler1d = 0x8B5D,
    Sampler1dShadow = 0x8B61,
    Sampler2d = 0x8B5E,
    Sampler2dShadow = 0x8B62,
    Sampler3d = 0x8B5F,
    SamplerCube = 0x8B60,
#endif
#if defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    FloatMat2x3 = 0x8B65,
    FloatMat2x4 = 0x8B66,
    FloatMat3x2 = 0x8B67,
    FloatMat3x4 = 0x8B68,
    FloatMat4x2 = 0x8B69,
    FloatMat4x3 = 0x8B6A,
#endif
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    IntSampler1d = 0x8DC9,
    IntSampler1dArray = 0x8DCE,
    IntSampler2d = 0x8DCA,
    IntSampler2dArray = 0x8DCF,
    IntSampler3d = 0x8DCB,
    IntSamplerCube = 0x8DCC,
    Sampler1dArray = 0x8DC0,
    Sampler1dArrayShadow = 0x8DC3,
    Sampler2dArray = 0x8DC1,
    Sampler2dArrayShadow = 0x8DC4,
    SamplerCubeShadow = 0x8DC5,
    UnsignedIntSampler1d = 0x8DD1,
    UnsignedIntSampler1dArray = 0x8DD6,
    UnsignedIntSampler2d = 0x8DD2,
    UnsignedIntSampler2dArray = 0x8DD7,
    UnsignedIntSampler3d = 0x8DD3,
    UnsignedIntSamplerCube = 0x8DD4,
    UnsignedIntVec2 = 0x8DC6,
    UnsignedIntVec3 = 0x8DC7,
    UnsignedIntVec4 = 0x8DC8,
#endif
#if defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    IntSampler2dRect = 0x8DCD,
    IntSamplerBuffer = 0x8DD0,
    Sampler2dRect = 0x8B63,
    Sampler2dRectShadow = 0x8B64,
    SamplerBuffer = 0x8DC2,
    UnsignedIntSampler2dRect = 0x8DD5,
    UnsignedIntSamplerBuffer = 0x8DD8,
#endif
#if defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    IntSampler2dMultisample = 0x9109,
    IntSampler2dMultisampleArray = 0x910C,
    Sampler2dMultisample = 0x9108,
    Sampler2dMultisampleArray = 0x910B,
    UnsignedIntSampler2dMultisample = 0x910A,
    UnsignedIntSampler2dMultisampleArray = 0x910D,
#endif
#if defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    DoubleMat2 = 0x8F46,
    DoubleMat3 = 0x8F47,
    DoubleMat4 = 0x8F48,
    DoubleVec2 = 0x8FFC,
    DoubleVec3 = 0x8FFD,
    DoubleVec4 = 0x8FFE,
    IntSamplerCubeMapArray = 0x900E,
    SamplerCubeMapArray = 0x900C,
    SamplerCubeMapArrayShadow = 0x900D,
    UnsignedIntSamplerCubeMapArray = 0x900F,
#endif
#if defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    Image1d = 0x904C,
    Image1dArray = 0x9052,
    Image2d = 0x904D,
    Image2dArray = 0x9053,
    Image2dMultisample = 0x9055,
    Image2dMultisampleArray = 0x9056,
    Image2dRect = 0x904F,
    Image3d = 0x904E,
    ImageBuffer = 0x9051,
    ImageCube = 0x9050,
    ImageCubeMapArray = 0x9054,
    IntImage1d = 0x9057,
    IntImage1dArray = 0x905D,
    IntImage2d = 0x9058,
    IntImage2dArray = 0x905E,
    IntImage2dMultisample = 0x9060,
    IntImage2dMultisampleArray = 0x9061,
    IntImage2dRect = 0x905A,
    IntImage3d = 0x9059,
    IntImageBuffer = 0x905C,
    IntImageCube = 0x905B,
    IntImageCubeMapArray = 0x905F,
    UnsignedIntAtomicCounter = 0x92DB,
    UnsignedIntImage1d = 0x9062,
    UnsignedIntImage1dArray = 0x9068,
    UnsignedIntImage2d = 0x9063,
    UnsignedIntImage2dArray = 0x9069,
    UnsignedIntImage2dMultisample = 0x906B,
    UnsignedIntImage2dMultisampleArray = 0x906C,
    UnsignedIntImage2dRect = 0x9065,
    UnsignedIntImage3d = 0x9064,
    UnsignedIntImageBuffer = 0x9067,
    UnsignedIntImageCube = 0x9066,
    UnsignedIntImageCubeMapArray = 0x906A,
#endif
};

enum class GraphicsResetStatus : core::EnumType
{
    NoError = 0,
#if defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    GuiltyContextReset = 0x8253,
    InnocentContextReset = 0x8254,
    UnknownContextReset = 0x8255,
#endif
};

enum class HintMode : core::EnumType
{
    DontCare = 0x1100,
    Fastest = 0x1101,
    Nicest = 0x1102,
};

enum class HintTarget : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    FogHint = 0x0C54,
    PerspectiveCorrectionHint = 0x0C50,
    PointSmoothHint = 0x0C51,
#endif
    LineSmoothHint = 0x0C52,
    PolygonSmoothHint = 0x0C53,
#if defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    TextureCompressionHint = 0x84EF,
#endif
#if defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    GenerateMipmapHint = 0x8192,
#endif
#if defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    FragmentShaderDerivativeHint = 0x8B8B,
#endif
#if defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ProgramBinaryRetrievableHint = 0x8257,
#endif
};

enum class IndexPointerType : core::EnumType
{
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    Double = 0x140A,
#endif
};

enum class InterleavedArrayFormat : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    C3fV3f = 0x2A24,
    C4fN3fV3f = 0x2A26,
    C4ubV2f = 0x2A22,
    C4ubV3f = 0x2A23,
    N3fV3f = 0x2A25,
    T2fC3fV3f = 0x2A2A,
    T2fC4fN3fV3f = 0x2A2C,
    T2fC4ubV3f = 0x2A29,
    T2fN3fV3f = 0x2A2B,
    T2fV3f = 0x2A27,
    T4fC4fN3fV4f = 0x2A2D,
    T4fV4f = 0x2A28,
    V2f = 0x2A20,
    V3f = 0x2A21,
#endif
};

enum class InternalFormat : core::EnumType
{
    DepthComponent = 0x1902,
    Red = 0x1903,
    Rgb = 0x1907,
    Rgba = 0x1908,
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    Alpha12 = 0x803D,
    Alpha16 = 0x803E,
    Alpha4 = 0x803B,
    Alpha8 = 0x803C,
    Intensity = 0x8049,
    Intensity12 = 0x804C,
    Intensity16 = 0x804D,
    Intensity4 = 0x804A,
    Intensity8 = 0x804B,
    Luminance12 = 0x8041,
    Luminance12Alpha12 = 0x8047,
    Luminance12Alpha4 = 0x8046,
    Luminance16 = 0x8042,
    Luminance16Alpha16 = 0x8048,
    Luminance4 = 0x803F,
    Luminance4Alpha4 = 0x8043,
    Luminance6Alpha2 = 0x8044,
    Luminance8 = 0x8040,
    Luminance8Alpha8 = 0x8045,
#endif
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    R3G3B2 = 0x2A10,
    Rgb10 = 0x8052,
    Rgb10A2 = 0x8059,
    Rgb12 = 0x8053,
    Rgb16 = 0x8054,
    Rgb4 = 0x804F,
    Rgb5 = 0x8050,
    Rgb5A1 = 0x8057,
    Rgb8 = 0x8051,
    Rgba12 = 0x805A,
    Rgba16 = 0x805B,
    Rgba4 = 0x8056,
    Rgba8 = 0x8058,
#endif
#if defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    CompressedRgb = 0x84ED,
    CompressedRgba = 0x84EE,
#endif
#if defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    DepthComponent16 = 0x81A5,
#endif
#if defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    CompressedSrgb = 0x8C48,
    CompressedSrgbAlpha = 0x8C49,
    Srgb = 0x8C40,
    Srgb8 = 0x8C41,
    Srgb8Alpha8 = 0x8C43,
    SrgbAlpha = 0x8C42,
#endif
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    CompressedRed = 0x8225,
    CompressedRedRgtc1 = 0x8DBB,
    CompressedRg = 0x8226,
    CompressedRgRgtc2 = 0x8DBD,
    CompressedSignedRedRgtc1 = 0x8DBC,
    CompressedSignedRgRgtc2 = 0x8DBE,
    Depth24Stencil8 = 0x88F0,
    Depth32fStencil8 = 0x8CAD,
    DepthComponent32f = 0x8CAC,
    DepthStencil = 0x84F9,
    R11fG11fB10f = 0x8C3A,
    R16 = 0x822A,
    R16f = 0x822D,
    R16i = 0x8233,
    R16ui = 0x8234,
    R32f = 0x822E,
    R32i = 0x8235,
    R32ui = 0x8236,
    R8 = 0x8229,
    R8i = 0x8231,
    R8ui = 0x8232,
    Rg = 0x8227,
    Rg16 = 0x822C,
    Rg16f = 0x822F,
    Rg16i = 0x8239,
    Rg16ui = 0x823A,
    Rg32f = 0x8230,
    Rg32i = 0x823B,
    Rg32ui = 0x823C,
    Rg8 = 0x822B,
    Rg8i = 0x8237,
    Rg8ui = 0x8238,
    Rgb16f = 0x881B,
    Rgb16i = 0x8D89,
    Rgb16ui = 0x8D77,
    Rgb32f = 0x8815,
    Rgb32i = 0x8D83,
    Rgb32ui = 0x8D71,
    Rgb8i = 0x8D8F,
    Rgb8ui = 0x8D7D,
    Rgb9E5 = 0x8C3D,
    Rgba16f = 0x881A,
    Rgba16i = 0x8D88,
    Rgba16ui = 0x8D76,
    Rgba32f = 0x8814,
    Rgba32i = 0x8D82,
    Rgba32ui = 0x8D70,
    Rgba8i = 0x8D8E,
    Rgba8ui = 0x8D7C,
#endif
#if defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    R16Snorm = 0x8F98,
    R8Snorm = 0x8F94,
    Rg16Snorm = 0x8F99,
    Rg8Snorm = 0x8F95,
    Rgb16Snorm = 0x8F9A,
    Rgb8Snorm = 0x8F96,
    Rgba8Snorm = 0x8F97,
#endif
#if defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    Rgb10A2ui = 0x906F,
#endif
#if defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    CompressedRgbaBptcUnorm = 0x8E8C,
    CompressedRgbBptcSignedFloat = 0x8E8E,
    CompressedRgbBptcUnsignedFloat = 0x8E8F,
    CompressedSrgbAlphaBptcUnorm = 0x8E8D,
#endif
#if defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
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
#endif
};

enum class InternalFormatPName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    Samples = 0x80A9,
    TextureCompressed = 0x86A1,
#endif
#if defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    GenerateMipmap = 0x8191,
#endif
#if defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ImageFormatCompatibilityType = 0x90C7,
    NumSampleCounts = 0x9380,
#endif
#if defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    AutoGenerateMipmap = 0x8295,
    ClearBuffer = 0x82B4,
    ColorComponents = 0x8283,
    ColorEncoding = 0x8296,
    ColorRenderable = 0x8286,
    ComputeTexture = 0x82A0,
    DepthRenderable = 0x8287,
    Filter = 0x829A,
    FragmentTexture = 0x829F,
    FramebufferBlend = 0x828B,
    FramebufferRenderable = 0x8289,
    FramebufferRenderableLayered = 0x828A,
    GeometryTexture = 0x829E,
    GetTextureImageFormat = 0x8291,
    GetTextureImageType = 0x8292,
    ImageCompatibilityClass = 0x82A8,
    ImagePixelFormat = 0x82A9,
    ImagePixelType = 0x82AA,
    ImageTexelSize = 0x82A7,
    InternalformatAlphaSize = 0x8274,
    InternalformatAlphaType = 0x827B,
    InternalformatBlueSize = 0x8273,
    InternalformatBlueType = 0x827A,
    InternalformatDepthSize = 0x8275,
    InternalformatDepthType = 0x827C,
    InternalformatGreenSize = 0x8272,
    InternalformatGreenType = 0x8279,
    InternalformatPreferred = 0x8270,
    InternalformatRedSize = 0x8271,
    InternalformatRedType = 0x8278,
    InternalformatSharedSize = 0x8277,
    InternalformatStencilSize = 0x8276,
    InternalformatStencilType = 0x827D,
    InternalformatSupported = 0x826F,
    MaxDepth = 0x8280,
    MaxHeight = 0x827F,
    MaxLayers = 0x8281,
    MaxWidth = 0x827E,
    Mipmap = 0x8293,
    ReadPixels = 0x828C,
    ReadPixelsFormat = 0x828D,
    ReadPixelsType = 0x828E,
    ShaderImageAtomic = 0x82A6,
    ShaderImageLoad = 0x82A4,
    ShaderImageStore = 0x82A5,
    SimultaneousTextureAndDepthTest = 0x82AC,
    SimultaneousTextureAndDepthWrite = 0x82AE,
    SimultaneousTextureAndStencilTest = 0x82AD,
    SimultaneousTextureAndStencilWrite = 0x82AF,
    SrgbRead = 0x8297,
    SrgbWrite = 0x8298,
    StencilRenderable = 0x8288,
    TessControlTexture = 0x829C,
    TessEvaluationTexture = 0x829D,
    TextureCompressedBlockHeight = 0x82B2,
    TextureCompressedBlockSize = 0x82B3,
    TextureCompressedBlockWidth = 0x82B1,
    TextureGather = 0x82A2,
    TextureGatherShadow = 0x82A3,
    TextureImageFormat = 0x828F,
    TextureImageType = 0x8290,
    TextureShadow = 0x82A1,
    TextureView = 0x82B5,
    VertexTexture = 0x829B,
    ViewCompatibilityClass = 0x82B6,
#endif
#if defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ClearTexture = 0x9365,
#endif
};

enum class LightEnvModeSGIX : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    Add = 0x0104,
    Modulate = 0x2100,
#endif
    Replace = 0x1E01,
};

enum class LightModelColorControl : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    SeparateSpecularColor = 0x81FA,
    SingleColor = 0x81F9,
#endif
};

enum class LightModelParameter : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    LightModelAmbient = 0x0B53,
    LightModelLocalViewer = 0x0B51,
    LightModelTwoSide = 0x0B52,
#endif
#if defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    LightModelColorControl = 0x81F8,
#endif
};

enum class LightName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    Light0 = 0x4000,
    Light1 = 0x4001,
    Light2 = 0x4002,
    Light3 = 0x4003,
    Light4 = 0x4004,
    Light5 = 0x4005,
    Light6 = 0x4006,
    Light7 = 0x4007,
#endif
};

enum class LightParameter : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    Ambient = 0x1200,
    ConstantAttenuation = 0x1207,
    Diffuse = 0x1201,
    LinearAttenuation = 0x1208,
    Position = 0x1203,
    QuadraticAttenuation = 0x1209,
    Specular = 0x1202,
    SpotCutoff = 0x1206,
    SpotDirection = 0x1204,
    SpotExponent = 0x1205,
#endif
};

enum class ListMode : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    Compile = 0x1300,
    CompileAndExecute = 0x1301,
#endif
};

enum class ListNameType : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    _2Bytes = 0x1407,
    _3Bytes = 0x1408,
    _4Bytes = 0x1409,
#endif
    Byte = 0x1400,
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
    UnsignedByte = 0x1401,
    UnsignedInt = 0x1405,
    UnsignedShort = 0x1403,
};

enum class LogicOp : core::EnumType
{
    And = 0x1501,
    AndInverted = 0x1504,
    AndReverse = 0x1502,
    Clear = 0x1500,
    Copy = 0x1503,
    CopyInverted = 0x150C,
    Equiv = 0x1509,
    Invert = 0x150A,
    Nand = 0x150E,
    Noop = 0x1505,
    Nor = 0x1508,
    Or = 0x1507,
    OrInverted = 0x150D,
    OrReverse = 0x150B,
    Set = 0x150F,
    Xor = 0x1506,
};

enum class MapBufferAccessMask : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    MapFlushExplicitBit = 0x0010,
    MapInvalidateBufferBit = 0x0008,
    MapInvalidateRangeBit = 0x0004,
    MapReadBit = 0x0001,
    MapUnsynchronizedBit = 0x0020,
    MapWriteBit = 0x0002,
#endif
#if defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    MapCoherentBit = 0x0080,
    MapPersistentBit = 0x0040,
#endif
};
constexpr core::Bitfield<MapBufferAccessMask> operator|(MapBufferAccessMask left, MapBufferAccessMask right) { return core::Bitfield<MapBufferAccessMask>(left) | right;}

enum class MapQuery : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    Coeff = 0x0A00,
    Domain = 0x0A02,
    Order = 0x0A01,
#endif
};

enum class MapTarget : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    Map1Color4 = 0x0D90,
    Map1Index = 0x0D91,
    Map1Normal = 0x0D92,
    Map1TextureCoord1 = 0x0D93,
    Map1TextureCoord2 = 0x0D94,
    Map1TextureCoord3 = 0x0D95,
    Map1TextureCoord4 = 0x0D96,
    Map1Vertex3 = 0x0D97,
    Map1Vertex4 = 0x0D98,
    Map2Color4 = 0x0DB0,
    Map2Index = 0x0DB1,
    Map2Normal = 0x0DB2,
    Map2TextureCoord1 = 0x0DB3,
    Map2TextureCoord2 = 0x0DB4,
    Map2TextureCoord3 = 0x0DB5,
    Map2TextureCoord4 = 0x0DB6,
    Map2Vertex3 = 0x0DB7,
    Map2Vertex4 = 0x0DB8,
#endif
};

enum class MaterialFace : core::EnumType
{
    Back = 0x0405,
    Front = 0x0404,
    FrontAndBack = 0x0408,
};

enum class MaterialParameter : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    Ambient = 0x1200,
    AmbientAndDiffuse = 0x1602,
    ColorIndexes = 0x1603,
    Diffuse = 0x1201,
    Emission = 0x1600,
    Shininess = 0x1601,
    Specular = 0x1202,
#endif
};

enum class MatrixMode : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    Modelview = 0x1700,
    Projection = 0x1701,
#endif
    Texture = 0x1702,
};

enum class MemoryBarrierMask : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    AllBarrierBits = 0xFFFFFFFF,
    AtomicCounterBarrierBit = 0x00001000,
    BufferUpdateBarrierBit = 0x00000200,
    CommandBarrierBit = 0x00000040,
    ElementArrayBarrierBit = 0x00000002,
    FramebufferBarrierBit = 0x00000400,
    PixelBufferBarrierBit = 0x00000080,
    ShaderImageAccessBarrierBit = 0x00000020,
    TextureFetchBarrierBit = 0x00000008,
    TextureUpdateBarrierBit = 0x00000100,
    TransformFeedbackBarrierBit = 0x00000800,
    UniformBarrierBit = 0x00000004,
    VertexAttribArrayBarrierBit = 0x00000001,
#endif
#if defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ShaderStorageBarrierBit = 0x00002000,
#endif
#if defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ClientMappedBufferBarrierBit = 0x00004000,
    QueryBufferBarrierBit = 0x00008000,
#endif
};
constexpr core::Bitfield<MemoryBarrierMask> operator|(MemoryBarrierMask left, MemoryBarrierMask right) { return core::Bitfield<MemoryBarrierMask>(left) | right;}

enum class MeshMode1 : core::EnumType
{
    Line = 0x1B01,
    Point = 0x1B00,
};

enum class MeshMode2 : core::EnumType
{
    Fill = 0x1B02,
    Line = 0x1B01,
    Point = 0x1B00,
};

enum class NormalPointerType : core::EnumType
{
    Byte = 0x1400,
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    Double = 0x140A,
#endif
};

enum class ObjectIdentifier : core::EnumType
{
    Texture = 0x1702,
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    VertexArray = 0x8074,
#endif
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    Framebuffer = 0x8D40,
    Renderbuffer = 0x8D41,
#endif
#if defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    TransformFeedback = 0x8E22,
#endif
#if defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    Buffer = 0x82E0,
    Program = 0x82E2,
    ProgramPipeline = 0x82E4,
    Query = 0x82E3,
    Sampler = 0x82E6,
    Shader = 0x82E1,
#endif
};

enum class PatchParameterName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    PatchDefaultInnerLevel = 0x8E73,
    PatchDefaultOuterLevel = 0x8E74,
    PatchVertices = 0x8E72,
#endif
};

enum class PathColor : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    PrimaryColor = 0x8577,
#endif
};

enum class PathFillMode : core::EnumType
{
    Invert = 0x150A,
};

enum class PathFontStyle : core::EnumType
{
    None = 0,
};

enum class PathGenMode : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    EyeLinear = 0x2400,
    ObjectLinear = 0x2401,
#endif
    None = 0,
#if defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    Constant = 0x8576,
#endif
};

enum class PathTransformType : core::EnumType
{
    None = 0,
};

enum class PipelineParameterName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    FragmentShader = 0x8B30,
    InfoLogLength = 0x8B84,
    VertexShader = 0x8B31,
#endif
#if defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    GeometryShader = 0x8DD9,
#endif
#if defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    TessControlShader = 0x8E88,
    TessEvaluationShader = 0x8E87,
#endif
#if defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ActiveProgram = 0x8259,
#endif
};

enum class PixelCopyType : core::EnumType
{
    Color = 0x1800,
    Depth = 0x1801,
    Stencil = 0x1802,
};

enum class PixelFormat : core::EnumType
{
    Alpha = 0x1906,
    Blue = 0x1905,
    DepthComponent = 0x1902,
    Green = 0x1904,
    Red = 0x1903,
    Rgb = 0x1907,
    Rgba = 0x1908,
    StencilIndex = 0x1901,
    UnsignedInt = 0x1405,
    UnsignedShort = 0x1403,
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    ColorIndex = 0x1900,
    Luminance = 0x1909,
    LuminanceAlpha = 0x190A,
#endif
#if defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    Bgr = 0x80E0,
    Bgra = 0x80E1,
#endif
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    BgraInteger = 0x8D9B,
    BgrInteger = 0x8D9A,
    BlueInteger = 0x8D96,
    DepthStencil = 0x84F9,
    GreenInteger = 0x8D95,
    RedInteger = 0x8D94,
    Rg = 0x8227,
    RgbaInteger = 0x8D99,
    RgbInteger = 0x8D98,
    RgInteger = 0x8228,
#endif
};

enum class PixelMap : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    PixelMapAToA = 0x0C79,
    PixelMapBToB = 0x0C78,
    PixelMapGToG = 0x0C77,
    PixelMapIToA = 0x0C75,
    PixelMapIToB = 0x0C74,
    PixelMapIToG = 0x0C73,
    PixelMapIToI = 0x0C70,
    PixelMapIToR = 0x0C72,
    PixelMapRToR = 0x0C76,
    PixelMapSToS = 0x0C71,
#endif
};

enum class PixelStoreParameter : core::EnumType
{
    PackAlignment = 0x0D05,
    PackLsbFirst = 0x0D01,
    PackRowLength = 0x0D02,
    PackSkipPixels = 0x0D04,
    PackSkipRows = 0x0D03,
    PackSwapBytes = 0x0D00,
    UnpackAlignment = 0x0CF5,
    UnpackLsbFirst = 0x0CF1,
    UnpackRowLength = 0x0CF2,
    UnpackSkipPixels = 0x0CF4,
    UnpackSkipRows = 0x0CF3,
    UnpackSwapBytes = 0x0CF0,
#if defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    PackImageHeight = 0x806C,
    PackSkipImages = 0x806B,
    UnpackImageHeight = 0x806E,
    UnpackSkipImages = 0x806D,
#endif
};

enum class PixelTexGenMode : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    Luminance = 0x1909,
    LuminanceAlpha = 0x190A,
#endif
    None = 0,
    Rgb = 0x1907,
    Rgba = 0x1908,
};

enum class PixelTransferParameter : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    AlphaBias = 0x0D1D,
    AlphaScale = 0x0D1C,
    BlueBias = 0x0D1B,
    BlueScale = 0x0D1A,
    DepthBias = 0x0D1F,
    DepthScale = 0x0D1E,
    GreenBias = 0x0D19,
    GreenScale = 0x0D18,
    IndexOffset = 0x0D13,
    IndexShift = 0x0D12,
    MapColor = 0x0D10,
    MapStencil = 0x0D11,
    RedBias = 0x0D15,
    RedScale = 0x0D14,
#endif
};

enum class PixelType : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    Bitmap = 0x1A00,
#endif
    Byte = 0x1400,
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
    UnsignedByte = 0x1401,
    UnsignedInt = 0x1405,
    UnsignedShort = 0x1403,
#if defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    UnsignedByte332 = 0x8032,
    UnsignedInt1010102 = 0x8036,
    UnsignedInt8888 = 0x8035,
    UnsignedShort4444 = 0x8033,
    UnsignedShort5551 = 0x8034,
#endif
};

enum class PointParameterNameARB : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    PointFadeThresholdSize = 0x8128,
#endif
};

enum class PointParameterNameSGIS : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    PointDistanceAttenuation = 0x8129,
    PointSizeMax = 0x8127,
    PointSizeMin = 0x8126,
#endif
#if defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    PointFadeThresholdSize = 0x8128,
#endif
};

enum class PolygonMode : core::EnumType
{
    Fill = 0x1B02,
    Line = 0x1B01,
    Point = 0x1B00,
};

enum class PrecisionType : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    HighFloat = 0x8DF2,
    HighInt = 0x8DF5,
    LowFloat = 0x8DF0,
    LowInt = 0x8DF3,
    MediumFloat = 0x8DF1,
    MediumInt = 0x8DF4,
#endif
};

enum class PrimitiveType : core::EnumType
{
    Lines = 0x0001,
    LineLoop = 0x0002,
    LineStrip = 0x0003,
    Points = 0x0000,
    Triangles = 0x0004,
    TriangleFan = 0x0006,
    TriangleStrip = 0x0005,
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    Polygon = 0x0009,
    QuadStrip = 0x0008,
#endif
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    Quads = 0x0007,
#endif
#if defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    LinesAdjacency = 0x000A,
    LineStripAdjacency = 0x000B,
    TrianglesAdjacency = 0x000C,
    TriangleStripAdjacency = 0x000D,
#endif
#if defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    Patches = 0x000E,
#endif
};

enum class ProgramInterface : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    TransformFeedbackBuffer = 0x8C8E,
#endif
#if defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    BufferVariable = 0x92E5,
    ComputeSubroutine = 0x92ED,
    ComputeSubroutineUniform = 0x92F3,
    FragmentSubroutine = 0x92EC,
    FragmentSubroutineUniform = 0x92F2,
    GeometrySubroutine = 0x92EB,
    GeometrySubroutineUniform = 0x92F1,
    ProgramInput = 0x92E3,
    ProgramOutput = 0x92E4,
    ShaderStorageBlock = 0x92E6,
    TessControlSubroutine = 0x92E9,
    TessControlSubroutineUniform = 0x92EF,
    TessEvaluationSubroutine = 0x92EA,
    TessEvaluationSubroutineUniform = 0x92F0,
    TransformFeedbackVarying = 0x92F4,
    Uniform = 0x92E1,
    UniformBlock = 0x92E2,
    VertexSubroutine = 0x92E8,
    VertexSubroutineUniform = 0x92EE,
#endif
};

enum class ProgramInterfacePName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ActiveResources = 0x92F5,
    MaxNameLength = 0x92F6,
    MaxNumActiveVariables = 0x92F7,
    MaxNumCompatibleSubroutines = 0x92F8,
#endif
};

enum class ProgramParameterPName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ProgramBinaryRetrievableHint = 0x8257,
    ProgramSeparable = 0x8258,
#endif
};

enum class ProgramPropertyARB : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ActiveAttributes = 0x8B89,
    ActiveAttributeMaxLength = 0x8B8A,
    ActiveUniforms = 0x8B86,
    ActiveUniformMaxLength = 0x8B87,
    AttachedShaders = 0x8B85,
    DeleteStatus = 0x8B80,
    InfoLogLength = 0x8B84,
    LinkStatus = 0x8B82,
    ValidateStatus = 0x8B83,
#endif
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    TransformFeedbackBufferMode = 0x8C7F,
    TransformFeedbackVaryings = 0x8C83,
    TransformFeedbackVaryingMaxLength = 0x8C76,
#endif
#if defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ActiveUniformBlocks = 0x8A36,
    ActiveUniformBlockMaxNameLength = 0x8A35,
#endif
#if defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    GeometryInputType = 0x8917,
    GeometryOutputType = 0x8918,
    GeometryVerticesOut = 0x8916,
#endif
#if defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ProgramBinaryLength = 0x8741,
#endif
#if defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ActiveAtomicCounterBuffers = 0x92D9,
#endif
#if defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ComputeWorkGroupSize = 0x8267,
#endif
};

enum class ProgramResourceProperty : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    CompatibleSubroutines = 0x8E4B,
    NumCompatibleSubroutines = 0x8E4A,
#endif
#if defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ActiveVariables = 0x9305,
    ArraySize = 0x92FB,
    ArrayStride = 0x92FE,
    AtomicCounterBufferIndex = 0x9301,
    BlockIndex = 0x92FD,
    BufferBinding = 0x9302,
    BufferDataSize = 0x9303,
    IsPerPatch = 0x92E7,
    IsRowMajor = 0x9300,
    Location = 0x930E,
    LocationIndex = 0x930F,
    MatrixStride = 0x92FF,
    NameLength = 0x92F9,
    NumActiveVariables = 0x9304,
    Offset = 0x92FC,
    ReferencedByComputeShader = 0x930B,
    ReferencedByFragmentShader = 0x930A,
    ReferencedByGeometryShader = 0x9309,
    ReferencedByTessControlShader = 0x9307,
    ReferencedByTessEvaluationShader = 0x9308,
    ReferencedByVertexShader = 0x9306,
    TopLevelArraySize = 0x930C,
    TopLevelArrayStride = 0x930D,
    Type = 0x92FA,
    Uniform = 0x92E1,
#endif
#if defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    LocationComponent = 0x934A,
    TransformFeedbackBufferIndex = 0x934B,
    TransformFeedbackBufferStride = 0x934C,
#endif
};

enum class ProgramStagePName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ActiveSubroutines = 0x8DE5,
    ActiveSubroutineMaxLength = 0x8E48,
    ActiveSubroutineUniforms = 0x8DE6,
    ActiveSubroutineUniformLocations = 0x8E47,
    ActiveSubroutineUniformMaxLength = 0x8E49,
#endif
};

enum class QueryCounterTarget : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    Timestamp = 0x8E28,
#endif
};

enum class QueryObjectParameterName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    QueryResult = 0x8866,
    QueryResultAvailable = 0x8867,
#endif
#if defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    QueryResultNoWait = 0x9194,
#endif
#if defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    QueryTarget = 0x82EA,
#endif
};

enum class QueryParameterName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    CurrentQuery = 0x8865,
    QueryCounterBits = 0x8864,
#endif
};

enum class QueryTarget : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    SamplesPassed = 0x8914,
#endif
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    PrimitivesGenerated = 0x8C87,
    TransformFeedbackPrimitivesWritten = 0x8C88,
#endif
#if defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    AnySamplesPassed = 0x8C2F,
    TimeElapsed = 0x88BF,
#endif
#if defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    AnySamplesPassedConservative = 0x8D6A,
#endif
#if defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    PrimitivesSubmitted = 0x82EF,
    TransformFeedbackOverflow = 0x82EC,
    VertexShaderInvocations = 0x82F0,
    VerticesSubmitted = 0x82EE,
#endif
};

enum class ReadBufferMode : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    Aux0 = 0x0409,
    Aux1 = 0x040A,
    Aux2 = 0x040B,
    Aux3 = 0x040C,
#endif
    Back = 0x0405,
    BackLeft = 0x0402,
    BackRight = 0x0403,
    Front = 0x0404,
    FrontLeft = 0x0400,
    FrontRight = 0x0401,
    Left = 0x0406,
    Right = 0x0407,
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ColorAttachment0 = 0x8CE0,
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
#endif
};

enum class RenderbufferParameterName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    RenderbufferAlphaSize = 0x8D53,
    RenderbufferBlueSize = 0x8D52,
    RenderbufferDepthSize = 0x8D54,
    RenderbufferGreenSize = 0x8D51,
    RenderbufferHeight = 0x8D43,
    RenderbufferInternalFormat = 0x8D44,
    RenderbufferRedSize = 0x8D50,
    RenderbufferSamples = 0x8CAB,
    RenderbufferStencilSize = 0x8D55,
    RenderbufferWidth = 0x8D42,
#endif
};

enum class RenderbufferTarget : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    Renderbuffer = 0x8D41,
#endif
};

enum class RenderingMode : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    Feedback = 0x1C01,
    Render = 0x1C00,
    Select = 0x1C02,
#endif
};

enum class SamplerParameterF : core::EnumType
{
    TextureBorderColor = 0x1004,
#if defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    TextureMaxLod = 0x813B,
    TextureMinLod = 0x813A,
#endif
#if defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    TextureMaxAnisotropy = 0x84FE,
#endif
};

enum class SamplerParameterI : core::EnumType
{
    TextureMagFilter = 0x2800,
    TextureMinFilter = 0x2801,
    TextureWrapS = 0x2802,
    TextureWrapT = 0x2803,
#if defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    TextureWrapR = 0x8072,
#endif
#if defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    TextureCompareFunc = 0x884D,
    TextureCompareMode = 0x884C,
#endif
};

enum class ShaderParameterName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    CompileStatus = 0x8B81,
    DeleteStatus = 0x8B80,
    InfoLogLength = 0x8B84,
    ShaderSourceLength = 0x8B88,
    ShaderType = 0x8B4F,
#endif
};

enum class ShaderType : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    FragmentShader = 0x8B30,
    VertexShader = 0x8B31,
#endif
#if defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    GeometryShader = 0x8DD9,
#endif
#if defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    TessControlShader = 0x8E88,
    TessEvaluationShader = 0x8E87,
#endif
#if defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ComputeShader = 0x91B9,
#endif
};

enum class ShadingModel : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    Flat = 0x1D00,
    Smooth = 0x1D01,
#endif
};

enum class StencilFaceDirection : core::EnumType
{
    Back = 0x0405,
    Front = 0x0404,
    FrontAndBack = 0x0408,
};

enum class StencilFunction : core::EnumType
{
    Always = 0x0207,
    Equal = 0x0202,
    Gequal = 0x0206,
    Greater = 0x0204,
    Lequal = 0x0203,
    Less = 0x0201,
    Never = 0x0200,
    Notequal = 0x0205,
};

enum class StencilOp : core::EnumType
{
    Decr = 0x1E03,
    Incr = 0x1E02,
    Invert = 0x150A,
    Keep = 0x1E00,
    Replace = 0x1E01,
    Zero = 0,
#if defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    DecrWrap = 0x8508,
    IncrWrap = 0x8507,
#endif
};

enum class StringName : core::EnumType
{
    Extensions = 0x1F03,
    Renderer = 0x1F01,
    Vendor = 0x1F00,
    Version = 0x1F02,
#if defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ShadingLanguageVersion = 0x8B8C,
#endif
};

enum class SubroutineParameterName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    UniformNameLength = 0x8A39,
    UniformSize = 0x8A38,
#endif
#if defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    CompatibleSubroutines = 0x8E4B,
    NumCompatibleSubroutines = 0x8E4A,
#endif
};

enum class SyncCondition : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    SyncGpuCommandsComplete = 0x9117,
#endif
};

enum class SyncObjectMask : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    SyncFlushCommandsBit = 0x00000001,
#endif
};
constexpr core::Bitfield<SyncObjectMask> operator|(SyncObjectMask left, SyncObjectMask right) { return core::Bitfield<SyncObjectMask>(left) | right;}

enum class SyncParameterName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ObjectType = 0x9112,
    SyncCondition = 0x9113,
    SyncFlags = 0x9115,
    SyncStatus = 0x9114,
#endif
};

enum class SyncStatus : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    AlreadySignaled = 0x911A,
    ConditionSatisfied = 0x911C,
    TimeoutExpired = 0x911B,
    WaitFailed = 0x911D,
#endif
};

enum class TexCoordPointerType : core::EnumType
{
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    Double = 0x140A,
#endif
};

enum class TextureCompareMode : core::EnumType
{
    None = 0,
#if defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    CompareRToTexture = 0x884E,
#endif
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    CompareRefToTexture = 0x884E,
#endif
};

enum class TextureCoordName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    Q = 0x2003,
    R = 0x2002,
    S = 0x2000,
    T = 0x2001,
#endif
};

enum class TextureEnvMode : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    Add = 0x0104,
    Decal = 0x2101,
    Modulate = 0x2100,
#endif
    Blend = 0x0BE2,
};

enum class TextureEnvParameter : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    TextureEnvColor = 0x2201,
    TextureEnvMode = 0x2200,
#endif
};

enum class TextureEnvTarget : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    TextureEnv = 0x2300,
#endif
};

enum class TextureGenMode : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    EyeLinear = 0x2400,
    ObjectLinear = 0x2401,
    SphereMap = 0x2402,
#endif
};

enum class TextureGenParameter : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    EyePlane = 0x2502,
    ObjectPlane = 0x2501,
    TextureGenMode = 0x2500,
#endif
};

enum class TextureMagFilter : core::EnumType
{
    Linear = 0x2601,
    Nearest = 0x2600,
};

enum class TextureMinFilter : core::EnumType
{
    Linear = 0x2601,
    LinearMipmapLinear = 0x2703,
    LinearMipmapNearest = 0x2701,
    Nearest = 0x2600,
    NearestMipmapLinear = 0x2702,
    NearestMipmapNearest = 0x2700,
};

enum class TextureParameterName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    TextureBorder = 0x1005,
    TextureComponents = 0x1003,
#endif
    TextureBorderColor = 0x1004,
    TextureHeight = 0x1001,
    TextureMagFilter = 0x2800,
    TextureMinFilter = 0x2801,
    TextureWidth = 0x1000,
    TextureWrapS = 0x2802,
    TextureWrapT = 0x2803,
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    TextureAlphaSize = 0x805F,
    TextureBlueSize = 0x805E,
    TextureGreenSize = 0x805D,
    TextureInternalFormat = 0x1003,
    TextureRedSize = 0x805C,
#endif
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    TextureIntensitySize = 0x8061,
    TextureLuminanceSize = 0x8060,
    TexturePriority = 0x8066,
    TextureResident = 0x8067,
#endif
#if defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    TextureBaseLevel = 0x813C,
    TextureMaxLevel = 0x813D,
    TextureMaxLod = 0x813B,
    TextureMinLod = 0x813A,
    TextureWrapR = 0x8072,
#endif
#if defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    GenerateMipmap = 0x8191,
#endif
#if defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    TextureCompareFunc = 0x884D,
    TextureCompareMode = 0x884C,
    TextureLodBias = 0x8501,
#endif
#if defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    TextureSwizzleA = 0x8E45,
    TextureSwizzleB = 0x8E44,
    TextureSwizzleG = 0x8E43,
    TextureSwizzleR = 0x8E42,
    TextureSwizzleRgba = 0x8E46,
#endif
#if defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    DepthStencilTextureMode = 0x90EA,
#endif
};

enum class TextureSwizzle : core::EnumType
{
    Alpha = 0x1906,
    Blue = 0x1905,
    Green = 0x1904,
    One = 1,
    Red = 0x1903,
    Zero = 0,
};

enum class TextureTarget : core::EnumType
{
    Texture1d = 0x0DE0,
    Texture2d = 0x0DE1,
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ProxyTexture1d = 0x8063,
    ProxyTexture2d = 0x8064,
#endif
#if defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ProxyTexture3d = 0x8070,
    Texture3d = 0x806F,
#endif
#if defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ProxyTextureCubeMap = 0x851B,
    TextureCubeMap = 0x8513,
    TextureCubeMapNegativeX = 0x8516,
    TextureCubeMapNegativeY = 0x8518,
    TextureCubeMapNegativeZ = 0x851A,
    TextureCubeMapPositiveX = 0x8515,
    TextureCubeMapPositiveY = 0x8517,
    TextureCubeMapPositiveZ = 0x8519,
#endif
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ProxyTexture1dArray = 0x8C19,
    ProxyTexture2dArray = 0x8C1B,
    Texture1dArray = 0x8C18,
    Texture2dArray = 0x8C1A,
#endif
#if defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ProxyTextureRectangle = 0x84F7,
    TextureRectangle = 0x84F5,
#endif
#if defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ProxyTexture2dMultisample = 0x9101,
    ProxyTexture2dMultisampleArray = 0x9103,
    Texture2dMultisample = 0x9100,
    Texture2dMultisampleArray = 0x9102,
#endif
#if defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ProxyTextureCubeMapArray = 0x900B,
    TextureCubeMapArray = 0x9009,
#endif
};

enum class TextureUnit : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
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
#endif
};

enum class TextureWrapMode : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
    Clamp = 0x2900,
#endif
    LinearMipmapLinear = 0x2703,
    Repeat = 0x2901,
#if defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ClampToEdge = 0x812F,
#endif
#if defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ClampToBorder = 0x812D,
#endif
#if defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    MirroredRepeat = 0x8370,
#endif
};

enum class TransformFeedbackBufferMode : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    InterleavedAttribs = 0x8C8C,
    SeparateAttribs = 0x8C8D,
#endif
};

enum class TransformFeedbackPName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    TransformFeedbackBufferBinding = 0x8C8F,
    TransformFeedbackBufferSize = 0x8C85,
    TransformFeedbackBufferStart = 0x8C84,
#endif
#if defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    TransformFeedbackActive = 0x8E24,
    TransformFeedbackPaused = 0x8E23,
#endif
};

enum class UniformBlockPName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    UniformBlockActiveUniforms = 0x8A42,
    UniformBlockActiveUniformIndices = 0x8A43,
    UniformBlockBinding = 0x8A3F,
    UniformBlockDataSize = 0x8A40,
    UniformBlockNameLength = 0x8A41,
    UniformBlockReferencedByFragmentShader = 0x8A46,
    UniformBlockReferencedByGeometryShader = 0x8A45,
    UniformBlockReferencedByVertexShader = 0x8A44,
#endif
#if defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    UniformBlockReferencedByTessControlShader = 0x84F0,
    UniformBlockReferencedByTessEvaluationShader = 0x84F1,
#endif
#if defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    UniformBlockReferencedByComputeShader = 0x90EC,
#endif
};

enum class UniformPName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    UniformArrayStride = 0x8A3C,
    UniformBlockIndex = 0x8A3A,
    UniformIsRowMajor = 0x8A3E,
    UniformMatrixStride = 0x8A3D,
    UniformNameLength = 0x8A39,
    UniformOffset = 0x8A3B,
    UniformSize = 0x8A38,
    UniformType = 0x8A37,
#endif
#if defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    UniformAtomicCounterBufferIndex = 0x92DA,
#endif
};

enum class UniformType : core::EnumType
{
    Float = 0x1406,
    Int = 0x1404,
    UnsignedInt = 0x1405,
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    Double = 0x140A,
#endif
#if defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
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
    Sampler1d = 0x8B5D,
    Sampler1dShadow = 0x8B61,
    Sampler2d = 0x8B5E,
    Sampler2dShadow = 0x8B62,
    Sampler3d = 0x8B5F,
    SamplerCube = 0x8B60,
#endif
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    IntSampler1d = 0x8DC9,
    IntSampler1dArray = 0x8DCE,
    IntSampler2d = 0x8DCA,
    IntSampler2dArray = 0x8DCF,
    IntSampler3d = 0x8DCB,
    IntSamplerCube = 0x8DCC,
    Sampler1dArray = 0x8DC0,
    Sampler1dArrayShadow = 0x8DC3,
    Sampler2dArray = 0x8DC1,
    Sampler2dArrayShadow = 0x8DC4,
    SamplerCubeShadow = 0x8DC5,
    UnsignedIntSampler1d = 0x8DD1,
    UnsignedIntSampler1dArray = 0x8DD6,
    UnsignedIntSampler2d = 0x8DD2,
    UnsignedIntSampler2dArray = 0x8DD7,
    UnsignedIntSampler3d = 0x8DD3,
    UnsignedIntSamplerCube = 0x8DD4,
    UnsignedIntVec2 = 0x8DC6,
    UnsignedIntVec3 = 0x8DC7,
    UnsignedIntVec4 = 0x8DC8,
#endif
#if defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    IntSampler2dRect = 0x8DCD,
    IntSamplerBuffer = 0x8DD0,
    Sampler2dRect = 0x8B63,
    Sampler2dRectShadow = 0x8B64,
    SamplerBuffer = 0x8DC2,
    UnsignedIntSampler2dRect = 0x8DD5,
    UnsignedIntSamplerBuffer = 0x8DD8,
#endif
#if defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    IntSampler2dMultisample = 0x9109,
    IntSampler2dMultisampleArray = 0x910C,
    Sampler2dMultisample = 0x9108,
    Sampler2dMultisampleArray = 0x910B,
    UnsignedIntSampler2dMultisample = 0x910A,
    UnsignedIntSampler2dMultisampleArray = 0x910D,
#endif
#if defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    DoubleVec2 = 0x8FFC,
    DoubleVec3 = 0x8FFD,
    DoubleVec4 = 0x8FFE,
#endif
};

enum class UseProgramStageMask : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    AllShaderBits = 0xFFFFFFFF,
    FragmentShaderBit = 0x00000002,
    GeometryShaderBit = 0x00000004,
    TessControlShaderBit = 0x00000008,
    TessEvaluationShaderBit = 0x00000010,
    VertexShaderBit = 0x00000001,
#endif
#if defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    ComputeShaderBit = 0x00000020,
#endif
};
constexpr core::Bitfield<UseProgramStageMask> operator|(UseProgramStageMask left, UseProgramStageMask right) { return core::Bitfield<UseProgramStageMask>(left) | right;}

enum class VertexArrayPName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    VertexAttribArrayEnabled = 0x8622,
    VertexAttribArrayNormalized = 0x886A,
    VertexAttribArraySize = 0x8623,
    VertexAttribArrayStride = 0x8624,
    VertexAttribArrayType = 0x8625,
#endif
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    VertexAttribArrayInteger = 0x88FD,
#endif
#if defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    VertexAttribArrayDivisor = 0x88FE,
#endif
#if defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    VertexAttribArrayLong = 0x874E,
    VertexAttribRelativeOffset = 0x82D5,
#endif
};

enum class VertexAttribEnum : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    VertexAttribArrayBufferBinding = 0x889F,
#endif
#if defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    CurrentVertexAttrib = 0x8626,
    VertexAttribArrayEnabled = 0x8622,
    VertexAttribArrayNormalized = 0x886A,
    VertexAttribArraySize = 0x8623,
    VertexAttribArrayStride = 0x8624,
    VertexAttribArrayType = 0x8625,
#endif
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    VertexAttribArrayInteger = 0x88FD,
#endif
#if defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    VertexAttribArrayDivisor = 0x88FE,
#endif
};

enum class VertexAttribIType : core::EnumType
{
    Byte = 0x1400,
    Int = 0x1404,
    Short = 0x1402,
    UnsignedByte = 0x1401,
    UnsignedInt = 0x1405,
    UnsignedShort = 0x1403,
};

enum class VertexAttribLType : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    Double = 0x140A,
#endif
};

enum class VertexAttribPointerPropertyARB : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    VertexAttribArrayPointer = 0x8645,
#endif
};

enum class VertexAttribPointerType : core::EnumType
{
    Byte = 0x1400,
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
    UnsignedByte = 0x1401,
    UnsignedInt = 0x1405,
    UnsignedShort = 0x1403,
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    Double = 0x140A,
#endif
#if defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    UnsignedInt2101010Rev = 0x8368,
#endif
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    HalfFloat = 0x140B,
    UnsignedInt10f11f11fRev = 0x8C3B,
#endif
#if defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    Int2101010Rev = 0x8D9F,
#endif
#if defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    Fixed = 0x140C,
#endif
};

enum class VertexAttribPropertyARB : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    VertexAttribArrayBufferBinding = 0x889F,
#endif
#if defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    CurrentVertexAttrib = 0x8626,
    VertexAttribArrayEnabled = 0x8622,
    VertexAttribArrayNormalized = 0x886A,
    VertexAttribArraySize = 0x8623,
    VertexAttribArrayStride = 0x8624,
    VertexAttribArrayType = 0x8625,
#endif
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    VertexAttribArrayInteger = 0x88FD,
#endif
#if defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    VertexAttribArrayDivisor = 0x88FE,
#endif
#if defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    VertexAttribArrayLong = 0x874E,
    VertexAttribBinding = 0x82D4,
    VertexAttribRelativeOffset = 0x82D5,
#endif
};

enum class VertexAttribType : core::EnumType
{
    Byte = 0x1400,
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
    UnsignedByte = 0x1401,
    UnsignedInt = 0x1405,
    UnsignedShort = 0x1403,
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    Double = 0x140A,
#endif
#if defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    UnsignedInt2101010Rev = 0x8368,
#endif
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    HalfFloat = 0x140B,
    UnsignedInt10f11f11fRev = 0x8C3B,
#endif
#if defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    Int2101010Rev = 0x8D9F,
#endif
#if defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    Fixed = 0x140C,
#endif
};

enum class VertexBufferObjectParameter : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    BufferAccess = 0x88BB,
    BufferMapped = 0x88BC,
    BufferSize = 0x8764,
    BufferUsage = 0x8765,
#endif
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    BufferAccessFlags = 0x911F,
    BufferMapLength = 0x9120,
    BufferMapOffset = 0x9121,
#endif
#if defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    BufferImmutableStorage = 0x821F,
    BufferStorageFlags = 0x8220,
#endif
};

enum class VertexBufferObjectUsage : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    DynamicCopy = 0x88EA,
    DynamicDraw = 0x88E8,
    DynamicRead = 0x88E9,
    StaticCopy = 0x88E6,
    StaticDraw = 0x88E4,
    StaticRead = 0x88E5,
    StreamCopy = 0x88E2,
    StreamDraw = 0x88E0,
    StreamRead = 0x88E1,
#endif
};

enum class VertexPointerType : core::EnumType
{
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    Double = 0x140A,
#endif
};

enum class VertexProvokingMode : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
    FirstVertexConvention = 0x8E4D,
    LastVertexConvention = 0x8E4E,
#endif
};

// commands
extern void disable(EnableCap cap);
extern void blendFunc(BlendingFactor sfactor, BlendingFactor dfactor);
extern const uint8_t *getString(StringName name);
extern void depthMask(bool flag);
extern void getTexLevelParameter(TextureTarget target, int32_t level, GetTextureParameter pname, int32_t *params);
extern void getBooleanv(GetPName pname, bool *data);
extern void frontFace(FrontFaceDirection mode);
extern void getTexParameter(TextureTarget target, GetTextureParameter pname, int32_t *params);
extern void texParameter(TextureTarget target, TextureParameterName pname, int32_t param);
extern void clearColor(float red, float green, float blue, float alpha);
extern void getIntegerv(GetPName pname, int32_t *data);
extern void finish();
extern void stencilMask(uint32_t mask);
extern void clear(core::Bitfield<ClearBufferMask> mask);
extern void flush();
extern void depthRange(double n, double f);
extern void readPixel(int32_t x, int32_t y, core::SizeType width, core::SizeType height, PixelFormat format, PixelType type, void *pixels);
extern void logicOp(LogicOp opcode);
extern void texImage2D(TextureTarget target, int32_t level, InternalFormat internalformat, core::SizeType width, core::SizeType height, int32_t border, PixelFormat format, PixelType type, const void *pixels);
extern void getFloatv(GetPName pname, float *data);
extern void cullFace(CullFaceMode mode);
extern bool isEnable(EnableCap cap);
extern void enable(EnableCap cap);
extern void getDoublev(GetPName pname, double *data);
extern void texParameter(TextureTarget target, TextureParameterName pname, float param);
extern void getTexParameter(TextureTarget target, GetTextureParameter pname, float *params);
extern void drawBuffer(DrawBufferMode buf);
extern void pointSize(float size);
extern void lineWidth(float width);
extern void stencilOp(StencilOp fail, StencilOp zfail, StencilOp zpass);
extern void polygonMode(MaterialFace face, PolygonMode mode);
extern void readBuffer(ReadBufferMode src);
extern void hint(HintTarget target, HintMode mode);
extern void texParameter(TextureTarget target, TextureParameterName pname, const float *params);
extern void getTexLevelParameter(TextureTarget target, int32_t level, GetTextureParameter pname, float *params);
extern void scissor(int32_t x, int32_t y, core::SizeType width, core::SizeType height);
extern void clearDepth(double depth);
extern void viewport(int32_t x, int32_t y, core::SizeType width, core::SizeType height);
extern core::EnumType getError();
extern void pixelStore(PixelStoreParameter pname, float param);
extern void depthFunc(DepthFunction func);
extern void clearStencil(int32_t s);
extern void stencilFunc(StencilFunction func, int32_t ref, uint32_t mask);
extern void colorMask(bool red, bool green, bool blue, bool alpha);
extern void texParameter(TextureTarget target, TextureParameterName pname, const int32_t *params);
extern void texImage1D(TextureTarget target, int32_t level, int32_t internalformat, core::SizeType width, int32_t border, PixelFormat format, PixelType type, const void *pixels);
extern void getTexImage(TextureTarget target, int32_t level, PixelFormat format, PixelType type, void *pixels);
extern void pixelStore(PixelStoreParameter pname, int32_t param);
#if defined(API_LEVEL_GL_VERSION_1_0) || defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
extern void rotate(float angle, float x, float y, float z);
extern void material(MaterialFace face, MaterialParameter pname, float param);
extern void texCoord4(const int16_t *v);
extern void color4(int32_t red, int32_t green, int32_t blue, int32_t alpha);
extern void loadIdentity();
extern void texCoord4(const int32_t *v);
extern void evalCoord1(const double *u);
extern void color4(float red, float green, float blue, float alpha);
extern uint32_t genList(core::SizeType range);
extern void getTexGen(TextureCoordName coord, TextureGenParameter pname, float *params);
extern void index(float c);
extern void rasterPos4(int32_t x, int32_t y, int32_t z, int32_t w);
extern void rotate(double angle, double x, double y, double z);
extern void texEnv(TextureEnvTarget target, TextureEnvParameter pname, int32_t param);
extern void texGen(TextureCoordName coord, TextureGenParameter pname, const double *params);
extern void indexMask(uint32_t mask);
extern void rect(double x1, double y1, double x2, double y2);
extern void rasterPos2(const int16_t *v);
extern void getLight(LightName light, LightParameter pname, int32_t *params);
extern void getMaterial(MaterialFace face, MaterialParameter pname, int32_t *params);
extern void texCoord3(double s, double t, double r);
extern void color3(double red, double green, double blue);
extern void color4(double red, double green, double blue, double alpha);
extern void pixelMap(PixelMap map, core::SizeType mapsize, const uint16_t *values);
extern void pushMatrix();
extern void scale(double x, double y, double z);
extern void texCoord4(const float *v);
extern void rect(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
extern void scale(float x, float y, float z);
extern void evalCoord1(const float *u);
extern void rasterPos4(float x, float y, float z, float w);
extern void texCoord1(int32_t s);
extern void evalCoord2(const double *u);
extern void color3(const float *v);
extern void copyPixel(int32_t x, int32_t y, core::SizeType width, core::SizeType height, PixelCopyType type);
extern void edgeFlag(bool flag);
extern void pixelTransfer(PixelTransferParameter pname, int32_t param);
extern void rasterPos2(int16_t x, int16_t y);
extern void texCoord2(const int32_t *v);
extern void en();
extern void bitmap(core::SizeType width, core::SizeType height, float xorig, float yorig, float xmove, float ymove, const uint8_t *bitmap);
extern void vertex3(float x, float y, float z);
extern void index(const double *c);
extern void vertex4(const double *v);
extern void vertex4(const float *v);
extern void deleteList(uint32_t list, core::SizeType range);
extern void mapGrid1(int32_t un, float u1, float u2);
extern void rasterPos4(const int32_t *v);
extern void texCoord2(float s, float t);
extern void clearIndex(float c);
extern void evalCoord1(float u);
extern void index(const float *c);
extern void normal3(int8_t nx, int8_t ny, int8_t nz);
extern void texCoord1(double s);
extern void texCoord3(const double *v);
extern void texCoord1(const int16_t *v);
extern void rasterPos3(float x, float y, float z);
extern void color4(uint32_t red, uint32_t green, uint32_t blue, uint32_t alpha);
extern void color4(uint16_t red, uint16_t green, uint16_t blue, uint16_t alpha);
extern void vertex3(double x, double y, double z);
extern void color4(const uint16_t *v);
extern void light(LightName light, LightParameter pname, const int32_t *params);
extern void color3(int8_t red, int8_t green, int8_t blue);
extern void rasterPos4(const int16_t *v);
extern int32_t renderMode(RenderingMode mode);
extern void normal3(float nx, float ny, float nz);
extern void endList();
extern void rect(const int32_t *v1, const int32_t *v2);
extern void vertex2(const double *v);
extern void vertex4(const int16_t *v);
extern void vertex3(const float *v);
extern void getMap(MapTarget target, GetMapQuery query, double *v);
extern void getClipPlane(ClipPlaneName plane, double *equation);
extern void map1(MapTarget target, double u1, double u2, int32_t stride, int32_t order, const double *points);
extern void rect(const float *v1, const float *v2);
extern void vertex3(int16_t x, int16_t y, int16_t z);
extern void rasterPos2(const double *v);
extern void rasterPos2(float x, float y);
extern void color4(int16_t red, int16_t green, int16_t blue, int16_t alpha);
extern void evalCoord2(float u, float v);
extern void rasterPos3(const int32_t *v);
extern void color4(const int16_t *v);
extern void drawPixel(core::SizeType width, core::SizeType height, PixelFormat format, PixelType type, const void *pixels);
extern void listBase(uint32_t base);
extern void loadMatrix(const float *m);
extern void fog(FogParameter pname, int32_t param);
extern void pixelZoom(float xfactor, float yfactor);
extern void texCoord4(const double *v);
extern void texCoord2(int32_t s, int32_t t);
extern void color4(const int32_t *v);
extern void texGen(TextureCoordName coord, TextureGenParameter pname, const int32_t *params);
extern void fog(FogParameter pname, const float *params);
extern void getMap(MapTarget target, GetMapQuery query, float *v);
extern void vertex4(int16_t x, int16_t y, int16_t z, int16_t w);
extern void color3(const uint8_t *v);
extern void edgeFlagv(const bool *flag);
extern void vertex4(int32_t x, int32_t y, int32_t z, int32_t w);
extern void color4(const double *v);
extern void color3(const int8_t *v);
extern void texCoord1(float s);
extern void texEnv(TextureEnvTarget target, TextureEnvParameter pname, float param);
extern void texCoord3(int16_t s, int16_t t, int16_t r);
extern void texCoord4(double s, double t, double r, double q);
extern void index(int32_t c);
extern void normal3(int16_t nx, int16_t ny, int16_t nz);
extern void rasterPos2(int32_t x, int32_t y);
extern void getMap(MapTarget target, GetMapQuery query, int32_t *v);
extern void texCoord2(int16_t s, int16_t t);
extern void index(const int32_t *c);
extern void color3(const uint32_t *v);
extern void index(int16_t c);
extern void initName();
extern void rasterPos4(double x, double y, double z, double w);
extern void accum(AccumOp op, float value);
extern void getPolygonStipple(uint8_t *mask);
extern void mapGrid1(int32_t un, double u1, double u2);
extern void multMatrix(const double *m);
extern void multMatrix(const float *m);
extern void normal3(double nx, double ny, double nz);
extern void pixelMap(PixelMap map, core::SizeType mapsize, const float *values);
extern void ortho(double left, double right, double bottom, double top, double zNear, double zFar);
extern void texGen(TextureCoordName coord, TextureGenParameter pname, const float *params);
extern void texGen(TextureCoordName coord, TextureGenParameter pname, int32_t param);
extern void texCoord1(const double *v);
extern void vertex3(const int16_t *v);
extern void vertex2(int32_t x, int32_t y);
extern void index(double c);
extern void vertex2(const float *v);
extern void fog(FogParameter pname, float param);
extern void texCoord1(const int32_t *v);
extern void getPixelMap(PixelMap map, uint16_t *values);
extern void popName();
extern void evalCoord1(double u);
extern void vertex3(int32_t x, int32_t y, int32_t z);
extern void map1(MapTarget target, float u1, float u2, int32_t stride, int32_t order, const float *points);
extern void polygonStipple(const uint8_t *mask);
extern void texCoord3(float s, float t, float r);
extern void color4(const uint8_t *v);
extern void rasterPos2(const float *v);
extern void texCoord4(int16_t s, int16_t t, int16_t r, int16_t q);
extern void clearAccum(float red, float green, float blue, float alpha);
extern void color3(uint8_t red, uint8_t green, uint8_t blue);
extern void color4(int8_t red, int8_t green, int8_t blue, int8_t alpha);
extern void vertex4(double x, double y, double z, double w);
extern void normal3(const int16_t *v);
extern void index(const int16_t *c);
extern void texCoord4(int32_t s, int32_t t, int32_t r, int32_t q);
extern void color3(const uint16_t *v);
extern void material(MaterialFace face, MaterialParameter pname, const float *params);
extern void texCoord2(double s, double t);
extern void color3(int32_t red, int32_t green, int32_t blue);
extern void begin(PrimitiveType mode);
extern void pushAttri(core::Bitfield<AttribMask> mask);
extern void light(LightName light, LightParameter pname, int32_t param);
extern void rasterPos3(int32_t x, int32_t y, int32_t z);
extern void texCoord1(int16_t s);
extern void texCoord4(float s, float t, float r, float q);
extern void lightModel(LightModelParameter pname, const int32_t *params);
extern void rasterPos4(const double *v);
extern void texCoord2(const double *v);
extern void pixelTransfer(PixelTransferParameter pname, float param);
extern void rect(const double *v1, const double *v2);
extern void feedbackBuffer(core::SizeType size, FeedbackType type, float *buffer);
extern void texGen(TextureCoordName coord, TextureGenParameter pname, float param);
extern void color4(const float *v);
extern void newList(uint32_t list, ListMode mode);
extern void popAttri();
extern void color3(int16_t red, int16_t green, int16_t blue);
extern void alphaFunc(AlphaFunction func, float ref);
extern void evalMesh2(MeshMode2 mode, int32_t i1, int32_t i2, int32_t j1, int32_t j2);
extern void vertex2(float x, float y);
extern void vertex3(const double *v);
extern void evalCoord2(double u, double v);
extern bool isList(uint32_t list);
extern void rect(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
extern void getMaterial(MaterialFace face, MaterialParameter pname, float *params);
extern void color3(uint32_t red, uint32_t green, uint32_t blue);
extern void mapGrid2(int32_t un, double u1, double u2, int32_t vn, double v1, double v2);
extern void color3(const double *v);
extern void getPixelMap(PixelMap map, uint32_t *values);
extern void texGen(TextureCoordName coord, TextureGenParameter pname, double param);
extern void getPixelMap(PixelMap map, float *values);
extern void pushName(uint32_t name);
extern void rasterPos3(const int16_t *v);
extern void texCoord3(int32_t s, int32_t t, int32_t r);
extern void normal3(const int8_t *v);
extern void vertex2(const int16_t *v);
extern void lightModel(LightModelParameter pname, const float *params);
extern void texCoord1(const float *v);
extern void color4(const int8_t *v);
extern void color3(const int32_t *v);
extern void texCoord3(const int16_t *v);
extern void map2(MapTarget target, double u1, double u2, int32_t ustride, int32_t uorder, double v1, double v2, int32_t vstride, int32_t vorder, const double *points);
extern void translate(double x, double y, double z);
extern void translate(float x, float y, float z);
extern void normal3(int32_t nx, int32_t ny, int32_t nz);
extern void rasterPos2(double x, double y);
extern void material(MaterialFace face, MaterialParameter pname, const int32_t *params);
extern void getTexGen(TextureCoordName coord, TextureGenParameter pname, double *params);
extern void matrixMode(MatrixMode mode);
extern void lightModel(LightModelParameter pname, float param);
extern void vertex4(const int32_t *v);
extern void texEnv(TextureEnvTarget target, TextureEnvParameter pname, const int32_t *params);
extern void color3(float red, float green, float blue);
extern void map2(MapTarget target, float u1, float u2, int32_t ustride, int32_t uorder, float v1, float v2, int32_t vstride, int32_t vorder, const float *points);
extern void rasterPos3(double x, double y, double z);
extern void popMatrix();
extern void rasterPos3(const double *v);
extern void evalMesh1(MeshMode1 mode, int32_t i1, int32_t i2);
extern void frustum(double left, double right, double bottom, double top, double zNear, double zFar);
extern void callList(uint32_t list);
extern void evalPoint2(int32_t i, int32_t j);
extern void color4(const uint32_t *v);
extern void rect(float x1, float y1, float x2, float y2);
extern void getTexGen(TextureCoordName coord, TextureGenParameter pname, int32_t *params);
extern void color3(uint16_t red, uint16_t green, uint16_t blue);
extern void evalCoord2(const float *u);
extern void rasterPos4(const float *v);
extern void texEnv(TextureEnvTarget target, TextureEnvParameter pname, const float *params);
extern void shadeModel(ShadingModel mode);
extern void getTexEnv(TextureEnvTarget target, TextureEnvParameter pname, float *params);
extern void callList(core::SizeType n, ListNameType type, const void *lists);
extern void vertex3(const int32_t *v);
extern void getTexEnv(TextureEnvTarget target, TextureEnvParameter pname, int32_t *params);
extern void color4(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);
extern void normal3(const float *v);
extern void clipPlane(ClipPlaneName plane, const double *equation);
extern void light(LightName light, LightParameter pname, float param);
extern void vertex4(float x, float y, float z, float w);
extern void lineStipple(int32_t factor, uint16_t pattern);
extern void vertex2(const int32_t *v);
extern void rect(const int16_t *v1, const int16_t *v2);
extern void texCoord2(const int16_t *v);
extern void rasterPos4(int16_t x, int16_t y, int16_t z, int16_t w);
extern void getLight(LightName light, LightParameter pname, float *params);
extern void lightModel(LightModelParameter pname, int32_t param);
extern void vertex2(double x, double y);
extern void light(LightName light, LightParameter pname, const float *params);
extern void normal3(const int32_t *v);
extern void material(MaterialFace face, MaterialParameter pname, int32_t param);
extern void colorMaterial(MaterialFace face, ColorMaterialParameter mode);
extern void passThrough(float token);
extern void loadName(uint32_t name);
extern void normal3(const double *v);
extern void pixelMap(PixelMap map, core::SizeType mapsize, const uint32_t *values);
extern void texCoord2(const float *v);
extern void mapGrid2(int32_t un, float u1, float u2, int32_t vn, float v1, float v2);
extern void rasterPos3(int16_t x, int16_t y, int16_t z);
extern void color3(const int16_t *v);
extern void selectBuffer(core::SizeType size, uint32_t *buffer);
extern void fog(FogParameter pname, const int32_t *params);
extern void rasterPos2(const int32_t *v);
extern void loadMatrix(const double *m);
extern void evalPoint1(int32_t i);
extern void texCoord3(const int32_t *v);
extern void rasterPos3(const float *v);
extern void vertex2(int16_t x, int16_t y);
extern void texCoord3(const float *v);
#endif
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
extern void drawElements(PrimitiveType mode, core::SizeType count, DrawElementsType type, const void *indices);
extern void texSubImage1D(TextureTarget target, int32_t level, int32_t xoffset, core::SizeType width, PixelFormat format, PixelType type, const void *pixels);
extern void copyTexImage2D(TextureTarget target, int32_t level, InternalFormat internalformat, int32_t x, int32_t y, core::SizeType width, core::SizeType height, int32_t border);
extern bool isTexture(uint32_t texture);
extern void polygonOffset(float factor, float units);
extern void bindTexture(TextureTarget target, uint32_t texture);
extern void deleteTextures(core::SizeType n, const uint32_t *textures);
extern void texSubImage2D(TextureTarget target, int32_t level, int32_t xoffset, int32_t yoffset, core::SizeType width, core::SizeType height, PixelFormat format, PixelType type, const void *pixels);
extern void drawArrays(PrimitiveType mode, int32_t first, core::SizeType count);
extern void copyTexSubImage2D(TextureTarget target, int32_t level, int32_t xoffset, int32_t yoffset, int32_t x, int32_t y, core::SizeType width, core::SizeType height);
extern void copyTexSubImage1D(TextureTarget target, int32_t level, int32_t xoffset, int32_t x, int32_t y, core::SizeType width);
extern void copyTexImage1D(TextureTarget target, int32_t level, InternalFormat internalformat, int32_t x, int32_t y, core::SizeType width, int32_t border);
extern void genTextures(core::SizeType n, uint32_t *textures);
#endif
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
extern void getPointerv(GetPointervPName pname, void **params);
#endif
#if defined(API_LEVEL_GL_VERSION_1_1) || defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
extern void indexPointer(IndexPointerType type, core::SizeType stride, const void *pointer);
extern void index(uint8_t c);
extern void index(const uint8_t *c);
extern void interleavedArrays(InterleavedArrayFormat format, core::SizeType stride, const void *pointer);
extern void popClientAttri();
extern void texCoordPointer(int32_t size, TexCoordPointerType type, core::SizeType stride, const void *pointer);
extern void enableClientState(EnableCap array);
extern void edgeFlagPointer(core::SizeType stride, const void *pointer);
extern void disableClientState(EnableCap array);
extern void pushClientAttri(core::Bitfield<ClientAttribMask> mask);
extern void colorPointer(int32_t size, ColorPointerType type, core::SizeType stride, const void *pointer);
extern void vertexPointer(int32_t size, VertexPointerType type, core::SizeType stride, const void *pointer);
extern void prioritizeTextures(core::SizeType n, const uint32_t *textures, const float *priorities);
extern bool areTexturesResident(core::SizeType n, const uint32_t *textures, bool *residences);
extern void arrayElement(int32_t i);
extern void normalPointer(NormalPointerType type, core::SizeType stride, const void *pointer);
#endif
#if defined(API_LEVEL_GL_VERSION_1_2) || defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
extern void texImage3D(TextureTarget target, int32_t level, int32_t internalformat, core::SizeType width, core::SizeType height, core::SizeType depth, int32_t border, PixelFormat format, PixelType type, const void *pixels);
extern void texSubImage3D(TextureTarget target, int32_t level, int32_t xoffset, int32_t yoffset, int32_t zoffset, core::SizeType width, core::SizeType height, core::SizeType depth, PixelFormat format, PixelType type, const void *pixels);
extern void drawRangeElements(PrimitiveType mode, uint32_t start, uint32_t end, core::SizeType count, DrawElementsType type, const void *indices);
extern void copyTexSubImage3D(TextureTarget target, int32_t level, int32_t xoffset, int32_t yoffset, int32_t zoffset, int32_t x, int32_t y, core::SizeType width, core::SizeType height);
#endif
#if defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
extern void compressedTexSubImage1D(TextureTarget target, int32_t level, int32_t xoffset, core::SizeType width, PixelFormat format, core::SizeType imageSize, const void *data);
extern void sampleCoverage(float value, bool invert);
extern void compressedTexImage1D(TextureTarget target, int32_t level, InternalFormat internalformat, core::SizeType width, int32_t border, core::SizeType imageSize, const void *data);
extern void activeTexture(TextureUnit texture);
extern void compressedTexImage3D(TextureTarget target, int32_t level, InternalFormat internalformat, core::SizeType width, core::SizeType height, core::SizeType depth, int32_t border, core::SizeType imageSize, const void *data);
extern void compressedTexImage2D(TextureTarget target, int32_t level, InternalFormat internalformat, core::SizeType width, core::SizeType height, int32_t border, core::SizeType imageSize, const void *data);
extern void getCompressedTexImage(TextureTarget target, int32_t level, void *img);
extern void compressedTexSubImage2D(TextureTarget target, int32_t level, int32_t xoffset, int32_t yoffset, core::SizeType width, core::SizeType height, PixelFormat format, core::SizeType imageSize, const void *data);
extern void compressedTexSubImage3D(TextureTarget target, int32_t level, int32_t xoffset, int32_t yoffset, int32_t zoffset, core::SizeType width, core::SizeType height, core::SizeType depth, PixelFormat format, core::SizeType imageSize, const void *data);
#endif
#if defined(API_LEVEL_GL_VERSION_1_3) || defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
extern void multiTexCoord4(TextureUnit target, const double *v);
extern void multiTexCoord3(TextureUnit target, int32_t s, int32_t t, int32_t r);
extern void multiTexCoord3(TextureUnit target, int16_t s, int16_t t, int16_t r);
extern void multiTexCoord1(TextureUnit target, float s);
extern void multiTexCoord1(TextureUnit target, const int16_t *v);
extern void multiTexCoord4(TextureUnit target, int32_t s, int32_t t, int32_t r, int32_t q);
extern void multiTexCoord3(TextureUnit target, const double *v);
extern void multiTexCoord1(TextureUnit target, int32_t s);
extern void multTransposeMatrix(const double *m);
extern void multiTexCoord3(TextureUnit target, const int32_t *v);
extern void multiTexCoord4(TextureUnit target, const int32_t *v);
extern void multiTexCoord4(TextureUnit target, int16_t s, int16_t t, int16_t r, int16_t q);
extern void clientActiveTexture(TextureUnit texture);
extern void multiTexCoord4(TextureUnit target, double s, double t, double r, double q);
extern void multiTexCoord3(TextureUnit target, const int16_t *v);
extern void multiTexCoord2(TextureUnit target, double s, double t);
extern void multiTexCoord4(TextureUnit target, const int16_t *v);
extern void multiTexCoord1(TextureUnit target, const int32_t *v);
extern void multiTexCoord2(TextureUnit target, const double *v);
extern void multiTexCoord3(TextureUnit target, float s, float t, float r);
extern void multiTexCoord4(TextureUnit target, const float *v);
extern void multiTexCoord4(TextureUnit target, float s, float t, float r, float q);
extern void multiTexCoord1(TextureUnit target, const double *v);
extern void multiTexCoord2(TextureUnit target, int32_t s, int32_t t);
extern void multiTexCoord1(TextureUnit target, const float *v);
extern void multiTexCoord3(TextureUnit target, double s, double t, double r);
extern void multiTexCoord1(TextureUnit target, double s);
extern void multiTexCoord2(TextureUnit target, const float *v);
extern void multiTexCoord2(TextureUnit target, const int32_t *v);
extern void multiTexCoord2(TextureUnit target, const int16_t *v);
extern void loadTransposeMatrix(const double *m);
extern void multTransposeMatrix(const float *m);
extern void multiTexCoord2(TextureUnit target, int16_t s, int16_t t);
extern void multiTexCoord1(TextureUnit target, int16_t s);
extern void multiTexCoord2(TextureUnit target, float s, float t);
extern void loadTransposeMatrix(const float *m);
extern void multiTexCoord3(TextureUnit target, const float *v);
#endif
#if defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
extern void blendEquation(BlendEquationModeEXT mode);
extern void pointParameter(PointParameterNameARB pname, const int32_t *params);
extern void blendFuncSeparate(BlendingFactor sfactorRGB, BlendingFactor dfactorRGB, BlendingFactor sfactorAlpha, BlendingFactor dfactorAlpha);
extern void pointParameter(PointParameterNameARB pname, float param);
extern void blendColor(float red, float green, float blue, float alpha);
extern void multiDrawElements(PrimitiveType mode, const core::SizeType *count, DrawElementsType type, const void *const*indices, core::SizeType drawcount);
extern void multiDrawArrays(PrimitiveType mode, const int32_t *first, const core::SizeType *count, core::SizeType drawcount);
extern void pointParameter(PointParameterNameARB pname, const float *params);
extern void pointParameter(PointParameterNameARB pname, int32_t param);
#endif
#if defined(API_LEVEL_GL_VERSION_1_4) || defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
extern void secondaryColor3(int8_t red, int8_t green, int8_t blue);
extern void windowPos3(int32_t x, int32_t y, int32_t z);
extern void secondaryColor3(float red, float green, float blue);
extern void secondaryColor3(double red, double green, double blue);
extern void windowPos2(const double *v);
extern void secondaryColor3(const uint32_t *v);
extern void windowPos2(int16_t x, int16_t y);
extern void windowPos2(const int32_t *v);
extern void fogCoordPointer(FogPointerTypeEXT type, core::SizeType stride, const void *pointer);
extern void windowPos3(double x, double y, double z);
extern void windowPos3(const float *v);
extern void secondaryColor3(const double *v);
extern void windowPos3(float x, float y, float z);
extern void fogCoord(float coord);
extern void windowPos3(const double *v);
extern void fogCoord(double coord);
extern void windowPos2(float x, float y);
extern void secondaryColor3(int16_t red, int16_t green, int16_t blue);
extern void fogCoord(const double *coord);
extern void secondaryColor3(const float *v);
extern void secondaryColor3(const uint16_t *v);
extern void secondaryColor3(uint32_t red, uint32_t green, uint32_t blue);
extern void windowPos2(const int16_t *v);
extern void fogCoord(const float *coord);
extern void secondaryColor3(const int8_t *v);
extern void secondaryColor3(const int16_t *v);
extern void windowPos2(double x, double y);
extern void windowPos3(const int32_t *v);
extern void secondaryColor3(const int32_t *v);
extern void secondaryColor3(int32_t red, int32_t green, int32_t blue);
extern void secondaryColor3(uint8_t red, uint8_t green, uint8_t blue);
extern void secondaryColor3(const uint8_t *v);
extern void secondaryColor3(uint16_t red, uint16_t green, uint16_t blue);
extern void windowPos3(int16_t x, int16_t y, int16_t z);
extern void windowPos3(const int16_t *v);
extern void windowPos2(const float *v);
extern void secondaryColorPointer(int32_t size, ColorPointerType type, core::SizeType stride, const void *pointer);
extern void windowPos2(int32_t x, int32_t y);
#endif
#if defined(API_LEVEL_GL_VERSION_1_5) || defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
extern void beginQuery(QueryTarget target, uint32_t id);
extern void endQuery(QueryTarget target);
extern void genBuffers(core::SizeType n, uint32_t *buffers);
extern void getBufferPointerv(BufferTargetARB target, BufferPointerNameARB pname, void **params);
extern void deleteQuerie(core::SizeType n, const uint32_t *ids);
extern void deleteBuffers(core::SizeType n, const uint32_t *buffers);
extern void bufferData(BufferTargetARB target, std::size_t size, const void *data, BufferUsageARB usage);
extern void getBufferSubData(BufferTargetARB target, std::intptr_t offset, std::size_t size, void *data);
extern void genQuerie(core::SizeType n, uint32_t *ids);
extern void getQueryObject(uint32_t id, QueryObjectParameterName pname, int32_t *params);
extern bool isBuffer(uint32_t buffer);
extern void getQueryObject(uint32_t id, QueryObjectParameterName pname, uint32_t *params);
extern void getBufferParameter(BufferTargetARB target, BufferPNameARB pname, int32_t *params);
extern bool isQuery(uint32_t id);
extern void *mapBuffer(BufferTargetARB target, BufferAccessARB access);
extern void getQuery(QueryTarget target, QueryParameterName pname, int32_t *params);
extern void bindBuffer(BufferTargetARB target, uint32_t buffer);
extern bool unmapBuffer(BufferTargetARB target);
extern void bufferSubData(BufferTargetARB target, std::intptr_t offset, std::size_t size, const void *data);
#endif
#if defined(API_LEVEL_GL_VERSION_2_0) || defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
extern void vertexAttrib4N(uint32_t index, const uint32_t *v);
extern void vertexAttrib3(uint32_t index, const float *v);
extern void linkProgram(uint32_t program);
extern void vertexAttrib4(uint32_t index, float x, float y, float z, float w);
extern void validateProgram(uint32_t program);
extern void vertexAttrib4N(uint32_t index, const int32_t *v);
extern void vertexAttrib4(uint32_t index, const uint16_t *v);
extern void uniform1(int32_t location, float v0);
extern void uniform3(int32_t location, float v0, float v1, float v2);
extern void deleteShader(uint32_t shader);
extern void vertexAttrib4N(uint32_t index, const int8_t *v);
extern void vertexAttribPointer(uint32_t index, int32_t size, VertexAttribPointerType type, bool normalized, core::SizeType stride, const void *pointer);
extern void vertexAttrib4(uint32_t index, const uint8_t *v);
extern void getProgramInfoLog(uint32_t program, core::SizeType bufSize, core::SizeType *length, char *infoLog);
extern void vertexAttrib2(uint32_t index, const float *v);
extern void vertexAttrib3(uint32_t index, const int16_t *v);
extern void vertexAttrib2(uint32_t index, const double *v);
extern void uniform2(int32_t location, core::SizeType count, const int32_t *value);
extern void compileShader(uint32_t shader);
extern void getVertexAttrib(uint32_t index, VertexAttribPropertyARB pname, float *params);
extern bool isShader(uint32_t shader);
extern void vertexAttrib4(uint32_t index, const int8_t *v);
extern void drawBuffers(core::SizeType n, const DrawBufferMode *bufs);
extern void vertexAttrib3(uint32_t index, float x, float y, float z);
extern void vertexAttrib1(uint32_t index, const float *v);
extern void vertexAttrib2(uint32_t index, double x, double y);
extern void getAttachedShaders(uint32_t program, core::SizeType maxCount, core::SizeType *count, uint32_t *shaders);
extern void enableVertexAttribArray(uint32_t index);
extern void vertexAttrib4N(uint32_t index, const int16_t *v);
extern void vertexAttrib1(uint32_t index, const int16_t *v);
extern void getVertexAttrib(uint32_t index, VertexAttribPropertyARB pname, double *params);
extern void vertexAttrib2(uint32_t index, const int16_t *v);
extern void uniform4(int32_t location, core::SizeType count, const float *value);
extern void getActiveAttri(uint32_t program, uint32_t index, core::SizeType bufSize, core::SizeType *length, int32_t *size, AttributeType *type, char *name);
extern int32_t getAttribLocation(uint32_t program, const char *name);
extern void vertexAttrib2(uint32_t index, int16_t x, int16_t y);
extern void shaderSource(uint32_t shader, core::SizeType count, const char *const*string, const int32_t *length);
extern void vertexAttrib3(uint32_t index, double x, double y, double z);
extern void vertexAttrib1(uint32_t index, const double *v);
extern void vertexAttrib3(uint32_t index, int16_t x, int16_t y, int16_t z);
extern void stencilMaskSeparate(StencilFaceDirection face, uint32_t mask);
extern void getUniform(uint32_t program, int32_t location, float *params);
extern void vertexAttrib4N(uint32_t index, const uint8_t *v);
extern void vertexAttrib4(uint32_t index, double x, double y, double z, double w);
extern void vertexAttrib4(uint32_t index, const int32_t *v);
extern void vertexAttrib1(uint32_t index, float x);
extern void vertexAttrib4N(uint32_t index, const uint16_t *v);
extern void vertexAttrib4(uint32_t index, const uint32_t *v);
extern void getProgram(uint32_t program, ProgramPropertyARB pname, int32_t *params);
extern void uniform1(int32_t location, core::SizeType count, const int32_t *value);
extern void vertexAttrib4N(uint32_t index, uint8_t x, uint8_t y, uint8_t z, uint8_t w);
extern void uniformMatrix4(int32_t location, core::SizeType count, bool transpose, const float *value);
extern void uniform4(int32_t location, float v0, float v1, float v2, float v3);
extern void vertexAttrib4(uint32_t index, int16_t x, int16_t y, int16_t z, int16_t w);
extern void vertexAttrib1(uint32_t index, int16_t x);
extern void uniform4(int32_t location, int32_t v0, int32_t v1, int32_t v2, int32_t v3);
extern void detachShader(uint32_t program, uint32_t shader);
extern void uniform4(int32_t location, core::SizeType count, const int32_t *value);
extern void getActiveUniform(uint32_t program, uint32_t index, core::SizeType bufSize, core::SizeType *length, int32_t *size, UniformType *type, char *name);
extern void stencilOpSeparate(StencilFaceDirection face, StencilOp sfail, StencilOp dpfail, StencilOp dppass);
extern uint32_t createShader(ShaderType type);
extern int32_t getUniformLocation(uint32_t program, const char *name);
extern void attachShader(uint32_t program, uint32_t shader);
extern void bindAttribLocation(uint32_t program, uint32_t index, const char *name);
extern void getShaderInfoLog(uint32_t shader, core::SizeType bufSize, core::SizeType *length, char *infoLog);
extern void uniformMatrix2(int32_t location, core::SizeType count, bool transpose, const float *value);
extern uint32_t createProgram();
extern void getShaderSource(uint32_t shader, core::SizeType bufSize, core::SizeType *length, char *source);
extern void vertexAttrib1(uint32_t index, double x);
extern void stencilFuncSeparate(StencilFaceDirection face, StencilFunction func, int32_t ref, uint32_t mask);
extern void vertexAttrib2(uint32_t index, float x, float y);
extern void getVertexAttrib(uint32_t index, VertexAttribPropertyARB pname, int32_t *params);
extern void getShader(uint32_t shader, ShaderParameterName pname, int32_t *params);
extern void vertexAttrib4(uint32_t index, const int16_t *v);
extern void uniform2(int32_t location, float v0, float v1);
extern void uniform2(int32_t location, core::SizeType count, const float *value);
extern void uniform2(int32_t location, int32_t v0, int32_t v1);
extern void uniform3(int32_t location, core::SizeType count, const float *value);
extern void uniform3(int32_t location, core::SizeType count, const int32_t *value);
extern void vertexAttrib4(uint32_t index, const float *v);
extern void uniform1(int32_t location, core::SizeType count, const float *value);
extern void vertexAttrib3(uint32_t index, const double *v);
extern void vertexAttrib4(uint32_t index, const double *v);
extern void uniformMatrix3(int32_t location, core::SizeType count, bool transpose, const float *value);
extern void disableVertexAttribArray(uint32_t index);
extern void getUniform(uint32_t program, int32_t location, int32_t *params);
extern bool isProgram(uint32_t program);
extern void uniform3(int32_t location, int32_t v0, int32_t v1, int32_t v2);
extern void uniform1(int32_t location, int32_t v0);
extern void getVertexAttribPointerv(uint32_t index, VertexAttribPointerPropertyARB pname, void **pointer);
extern void useProgram(uint32_t program);
extern void deleteProgram(uint32_t program);
extern void blendEquationSeparate(BlendEquationModeEXT modeRGB, BlendEquationModeEXT modeAlpha);
#endif
#if defined(API_LEVEL_GL_VERSION_2_1) || defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
extern void uniformMatrix2x4(int32_t location, core::SizeType count, bool transpose, const float *value);
extern void uniformMatrix4x3(int32_t location, core::SizeType count, bool transpose, const float *value);
extern void uniformMatrix2x3(int32_t location, core::SizeType count, bool transpose, const float *value);
extern void uniformMatrix4x2(int32_t location, core::SizeType count, bool transpose, const float *value);
extern void uniformMatrix3x2(int32_t location, core::SizeType count, bool transpose, const float *value);
extern void uniformMatrix3x4(int32_t location, core::SizeType count, bool transpose, const float *value);
#endif
#if defined(API_LEVEL_GL_VERSION_3_0) || defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
extern void generateMipmap(TextureTarget target);
extern void getTexParameterI(TextureTarget target, GetTextureParameter pname, int32_t *params);
extern void enable(EnableCap target, uint32_t index);
extern void getBooleani_v(BufferTargetARB target, uint32_t index, bool *data);
extern bool isRenderbuffer(uint32_t renderbuffer);
extern bool isEnabled(EnableCap target, uint32_t index);
extern void vertexAttribI4(uint32_t index, const uint16_t *v);
extern void colorMask(uint32_t index, bool r, bool g, bool b, bool a);
extern void clearBuffer(Buffer buffer, int32_t drawbuffer, const int32_t *value);
extern void uniform3(int32_t location, uint32_t v0, uint32_t v1, uint32_t v2);
extern void uniform4(int32_t location, core::SizeType count, const uint32_t *value);
extern void getIntegeri_v(core::EnumType target, uint32_t index, int32_t *data);
extern void vertexAttribI2(uint32_t index, const int32_t *v);
extern void getTransformFeedbackVarying(uint32_t program, uint32_t index, core::SizeType bufSize, core::SizeType *length, core::SizeType *size, GlslTypeToken *type, char *name);
extern void beginTransformFeedback(PrimitiveType primitiveMode);
extern void vertexAttribI1(uint32_t index, const uint32_t *v);
extern void genVertexArrays(core::SizeType n, uint32_t *arrays);
extern void genFramebuffers(core::SizeType n, uint32_t *framebuffers);
extern void clearBuffer(Buffer buffer, int32_t drawbuffer, const float *value);
extern void uniform1(int32_t location, uint32_t v0);
extern void transformFeedbackVarying(uint32_t program, core::SizeType count, const char *const*varyings, TransformFeedbackBufferMode bufferMode);
extern void bindBufferRange(BufferTargetARB target, uint32_t index, uint32_t buffer, std::intptr_t offset, std::size_t size);
extern void uniform3(int32_t location, core::SizeType count, const uint32_t *value);
extern void getFramebufferAttachmentParameter(FramebufferTarget target, FramebufferAttachment attachment, FramebufferAttachmentParameterName pname, int32_t *params);
extern void getRenderbufferParameter(RenderbufferTarget target, RenderbufferParameterName pname, int32_t *params);
extern void vertexAttribI2(uint32_t index, int32_t x, int32_t y);
extern void framebufferTexture3D(FramebufferTarget target, FramebufferAttachment attachment, TextureTarget textarget, uint32_t texture, int32_t level, int32_t zoffset);
extern void uniform4(int32_t location, uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3);
extern void bindVertexArray(uint32_t array);
extern void deleteRenderbuffers(core::SizeType n, const uint32_t *renderbuffers);
extern void *mapBufferRange(BufferTargetARB target, std::intptr_t offset, std::size_t length, core::Bitfield<MapBufferAccessMask> access);
extern void vertexAttribI3(uint32_t index, const int32_t *v);
extern void vertexAttribI4(uint32_t index, const int32_t *v);
extern void vertexAttribI4(uint32_t index, const uint8_t *v);
extern void framebufferRenderbuffer(FramebufferTarget target, FramebufferAttachment attachment, RenderbufferTarget renderbuffertarget, uint32_t renderbuffer);
extern void uniform2(int32_t location, core::SizeType count, const uint32_t *value);
extern void getVertexAttribI(uint32_t index, VertexAttribEnum pname, uint32_t *params);
extern void vertexAttribI3(uint32_t index, int32_t x, int32_t y, int32_t z);
extern int32_t getFragDataLocation(uint32_t program, const char *name);
extern void framebufferTexture1D(FramebufferTarget target, FramebufferAttachment attachment, TextureTarget textarget, uint32_t texture, int32_t level);
extern void beginConditionalRender(uint32_t id, ConditionalRenderMode mode);
extern void vertexAttribI4(uint32_t index, const int8_t *v);
extern void genRenderbuffers(core::SizeType n, uint32_t *renderbuffers);
extern void vertexAttribI1(uint32_t index, int32_t x);
extern const uint8_t *getString(StringName name, uint32_t index);
extern void uniform2(int32_t location, uint32_t v0, uint32_t v1);
extern void vertexAttribI4(uint32_t index, const uint32_t *v);
extern void vertexAttribI1(uint32_t index, uint32_t x);
extern void endConditionalRender();
extern void getTexParameterI(TextureTarget target, GetTextureParameter pname, uint32_t *params);
extern void deleteVertexArrays(core::SizeType n, const uint32_t *arrays);
extern void framebufferTexture2D(FramebufferTarget target, FramebufferAttachment attachment, TextureTarget textarget, uint32_t texture, int32_t level);
extern void flushMappedBufferRange(BufferTargetARB target, std::intptr_t offset, std::size_t length);
extern void blitFramebuffer(int32_t srcX0, int32_t srcY0, int32_t srcX1, int32_t srcY1, int32_t dstX0, int32_t dstY0, int32_t dstX1, int32_t dstY1, core::Bitfield<ClearBufferMask> mask, BlitFramebufferFilter filter);
extern void deleteFramebuffers(core::SizeType n, const uint32_t *framebuffers);
extern void getVertexAttribI(uint32_t index, VertexAttribEnum pname, int32_t *params);
extern void texParameterI(TextureTarget target, TextureParameterName pname, const int32_t *params);
extern void vertexAttribI2(uint32_t index, uint32_t x, uint32_t y);
extern void vertexAttribI3(uint32_t index, const uint32_t *v);
extern void getUniform(uint32_t program, int32_t location, uint32_t *params);
extern void bindFramebuffer(FramebufferTarget target, uint32_t framebuffer);
extern void renderbufferStorageMultisample(RenderbufferTarget target, core::SizeType samples, InternalFormat internalformat, core::SizeType width, core::SizeType height);
extern void vertexAttribI4(uint32_t index, uint32_t x, uint32_t y, uint32_t z, uint32_t w);
extern void vertexAttribI1(uint32_t index, const int32_t *v);
extern void bindFragDataLocation(uint32_t program, uint32_t color, const char *name);
extern void vertexAttribI2(uint32_t index, const uint32_t *v);
extern void vertexAttribI3(uint32_t index, uint32_t x, uint32_t y, uint32_t z);
extern void renderbufferStorage(RenderbufferTarget target, InternalFormat internalformat, core::SizeType width, core::SizeType height);
extern void clampColor(ClampColorTargetARB target, ClampColorModeARB clamp);
extern void uniform1(int32_t location, core::SizeType count, const uint32_t *value);
extern void endTransformFeedback();
extern bool isFramebuffer(uint32_t framebuffer);
extern void clearBufferf(Buffer buffer, int32_t drawbuffer, float depth, int32_t stencil);
extern core::EnumType checkFramebufferStatus(FramebufferTarget target);
extern void vertexAttribI4(uint32_t index, int32_t x, int32_t y, int32_t z, int32_t w);
extern void vertexAttribI4(uint32_t index, const int16_t *v);
extern void texParameterI(TextureTarget target, TextureParameterName pname, const uint32_t *params);
extern void bindBufferBase(BufferTargetARB target, uint32_t index, uint32_t buffer);
extern void vertexAttribIPointer(uint32_t index, int32_t size, VertexAttribPointerType type, core::SizeType stride, const void *pointer);
extern bool isVertexArray(uint32_t array);
extern void disable(EnableCap target, uint32_t index);
extern void clearBuffer(Buffer buffer, int32_t drawbuffer, const uint32_t *value);
extern void bindRenderbuffer(RenderbufferTarget target, uint32_t renderbuffer);
extern void framebufferTextureLayer(FramebufferTarget target, FramebufferAttachment attachment, uint32_t texture, int32_t level, int32_t layer);
#endif
#if defined(API_LEVEL_GL_VERSION_3_1) || defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
extern uint32_t getUniformBlockIndex(uint32_t program, const char *uniformBlockName);
extern void primitiveRestartIndex(uint32_t index);
extern void getActiveUniformName(uint32_t program, uint32_t uniformIndex, core::SizeType bufSize, core::SizeType *length, char *uniformName);
extern void getActiveUniforms(uint32_t program, core::SizeType uniformCount, const uint32_t *uniformIndices, UniformPName pname, int32_t *params);
extern void texBuffer(TextureTarget target, InternalFormat internalformat, uint32_t buffer);
extern void getActiveUniformBlock(uint32_t program, uint32_t uniformBlockIndex, UniformBlockPName pname, int32_t *params);
extern void uniformBlockBinding(uint32_t program, uint32_t uniformBlockIndex, uint32_t uniformBlockBinding);
extern void copyBufferSubData(CopyBufferSubDataTarget readTarget, CopyBufferSubDataTarget writeTarget, std::intptr_t readOffset, std::intptr_t writeOffset, std::size_t size);
extern void getActiveUniformBlockName(uint32_t program, uint32_t uniformBlockIndex, core::SizeType bufSize, core::SizeType *length, char *uniformBlockName);
extern void drawArraysInstance(PrimitiveType mode, int32_t first, core::SizeType count, core::SizeType instancecount);
extern void getUniformIndice(uint32_t program, core::SizeType uniformCount, const char *const*uniformNames, uint32_t *uniformIndices);
extern void drawElementsInstance(PrimitiveType mode, core::SizeType count, DrawElementsType type, const void *indices, core::SizeType instancecount);
#endif
#if defined(API_LEVEL_GL_VERSION_3_2_core) || defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
extern void drawRangeElementsBaseVertex(PrimitiveType mode, uint32_t start, uint32_t end, core::SizeType count, DrawElementsType type, const void *indices, int32_t basevertex);
extern core::Sync fenceSync(SyncCondition condition, uint32_t flags);
extern void waitSync(core::Sync sync, uint32_t flags, uint64_t timeout);
extern void provokingVertex(VertexProvokingMode mode);
extern void drawElementsInstancedBaseVertex(PrimitiveType mode, core::SizeType count, DrawElementsType type, const void *indices, core::SizeType instancecount, int32_t basevertex);
extern bool isSync(core::Sync sync);
extern void drawElementsBaseVertex(PrimitiveType mode, core::SizeType count, DrawElementsType type, const void *indices, int32_t basevertex);
extern void framebufferTexture(FramebufferTarget target, FramebufferAttachment attachment, uint32_t texture, int32_t level);
extern core::EnumType clientWaitSync(core::Sync sync, core::Bitfield<SyncObjectMask> flags, uint64_t timeout);
extern void sampleMask(uint32_t maskNumber, uint32_t mask);
extern void texImage3DMultisample(TextureTarget target, core::SizeType samples, InternalFormat internalformat, core::SizeType width, core::SizeType height, core::SizeType depth, bool fixedsamplelocations);
extern void getMultisample(GetMultisamplePNameNV pname, uint32_t index, float *val);
extern void texImage2DMultisample(TextureTarget target, core::SizeType samples, InternalFormat internalformat, core::SizeType width, core::SizeType height, bool fixedsamplelocations);
extern void getSync(core::Sync sync, SyncParameterName pname, core::SizeType bufSize, core::SizeType *length, int32_t *values);
extern void multiDrawElementsBaseVertex(PrimitiveType mode, const core::SizeType *count, DrawElementsType type, const void *const*indices, core::SizeType drawcount, const int32_t *basevertex);
extern void deleteSync(core::Sync sync);
extern void getInteger64v(GetPName pname, int64_t *data);
extern void getInteger64i_v(core::EnumType target, uint32_t index, int64_t *data);
extern void getBufferParameter(BufferTargetARB target, BufferPNameARB pname, int64_t *params);
#endif
#if defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_3_3_core) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
extern void vertexAttribP4(uint32_t index, VertexAttribPointerType type, bool normalized, uint32_t value);
extern void bindSampler(uint32_t unit, uint32_t sampler);
extern void samplerParameterI(uint32_t sampler, SamplerParameterI pname, const uint32_t *param);
extern void getSamplerParameter(uint32_t sampler, SamplerParameterF pname, float *params);
extern void vertexAttribP1(uint32_t index, VertexAttribPointerType type, bool normalized, const uint32_t *value);
extern void getSamplerParameter(uint32_t sampler, SamplerParameterI pname, int32_t *params);
extern void vertexAttribP1(uint32_t index, VertexAttribPointerType type, bool normalized, uint32_t value);
extern void getSamplerParameterI(uint32_t sampler, SamplerParameterI pname, int32_t *params);
extern void getSamplerParameterI(uint32_t sampler, SamplerParameterI pname, uint32_t *params);
extern void genSampler(core::SizeType count, uint32_t *samplers);
extern void vertexAttribP4(uint32_t index, VertexAttribPointerType type, bool normalized, const uint32_t *value);
extern void samplerParameter(uint32_t sampler, SamplerParameterI pname, const int32_t *param);
extern void vertexAttribP3(uint32_t index, VertexAttribPointerType type, bool normalized, const uint32_t *value);
extern void deleteSampler(core::SizeType count, const uint32_t *samplers);
extern void samplerParameter(uint32_t sampler, SamplerParameterF pname, float param);
extern void vertexAttribDivisor(uint32_t index, uint32_t divisor);
extern void samplerParameterI(uint32_t sampler, SamplerParameterI pname, const int32_t *param);
extern bool isSampler(uint32_t sampler);
extern void getQueryObject(uint32_t id, QueryObjectParameterName pname, int64_t *params);
extern int32_t getFragDataIndex(uint32_t program, const char *name);
extern void bindFragDataLocationIndexe(uint32_t program, uint32_t colorNumber, uint32_t index, const char *name);
extern void getQueryObject(uint32_t id, QueryObjectParameterName pname, uint64_t *params);
extern void samplerParameter(uint32_t sampler, SamplerParameterF pname, const float *param);
extern void samplerParameter(uint32_t sampler, SamplerParameterI pname, int32_t param);
extern void vertexAttribP2(uint32_t index, VertexAttribPointerType type, bool normalized, uint32_t value);
extern void queryCounter(uint32_t id, QueryCounterTarget target);
extern void vertexAttribP2(uint32_t index, VertexAttribPointerType type, bool normalized, const uint32_t *value);
extern void vertexAttribP3(uint32_t index, VertexAttribPointerType type, bool normalized, uint32_t value);
#endif
#if defined(API_LEVEL_GL_VERSION_3_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
extern void colorP3(ColorPointerType type, const uint32_t *color);
extern void multiTexCoordP3(TextureUnit texture, TexCoordPointerType type, uint32_t coords);
extern void texCoordP2(TexCoordPointerType type, const uint32_t *coords);
extern void multiTexCoordP3(TextureUnit texture, TexCoordPointerType type, const uint32_t *coords);
extern void texCoordP1(TexCoordPointerType type, const uint32_t *coords);
extern void texCoordP3(TexCoordPointerType type, const uint32_t *coords);
extern void texCoordP2(TexCoordPointerType type, uint32_t coords);
extern void normalP3(NormalPointerType type, uint32_t coords);
extern void secondaryColorP3(ColorPointerType type, uint32_t color);
extern void vertexP3(VertexPointerType type, const uint32_t *value);
extern void multiTexCoordP4(TextureUnit texture, TexCoordPointerType type, uint32_t coords);
extern void colorP3(ColorPointerType type, uint32_t color);
extern void texCoordP3(TexCoordPointerType type, uint32_t coords);
extern void multiTexCoordP2(TextureUnit texture, TexCoordPointerType type, const uint32_t *coords);
extern void texCoordP4(TexCoordPointerType type, const uint32_t *coords);
extern void multiTexCoordP1(TextureUnit texture, TexCoordPointerType type, uint32_t coords);
extern void colorP4(ColorPointerType type, const uint32_t *color);
extern void vertexP2(VertexPointerType type, const uint32_t *value);
extern void colorP4(ColorPointerType type, uint32_t color);
extern void vertexP4(VertexPointerType type, const uint32_t *value);
extern void texCoordP1(TexCoordPointerType type, uint32_t coords);
extern void vertexP4(VertexPointerType type, uint32_t value);
extern void texCoordP4(TexCoordPointerType type, uint32_t coords);
extern void normalP3(NormalPointerType type, const uint32_t *coords);
extern void secondaryColorP3(ColorPointerType type, const uint32_t *color);
extern void multiTexCoordP2(TextureUnit texture, TexCoordPointerType type, uint32_t coords);
extern void multiTexCoordP1(TextureUnit texture, TexCoordPointerType type, const uint32_t *coords);
extern void vertexP2(VertexPointerType type, uint32_t value);
extern void vertexP3(VertexPointerType type, uint32_t value);
extern void multiTexCoordP4(TextureUnit texture, TexCoordPointerType type, const uint32_t *coords);
#endif
#if defined(API_LEVEL_GL_VERSION_4_0_compatibility) || defined(API_LEVEL_GL_VERSION_4_0_core) || defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
extern void getUniform(uint32_t program, int32_t location, double *params);
extern void uniform3(int32_t location, double x, double y, double z);
extern void getQueryIndexed(QueryTarget target, uint32_t index, QueryParameterName pname, int32_t *params);
extern void drawTransformFeedbackStream(PrimitiveType mode, uint32_t id, uint32_t stream);
extern void blendEquation(uint32_t buf, BlendEquationModeEXT mode);
extern void uniformSubroutines(ShaderType shadertype, core::SizeType count, const uint32_t *indices);
extern void uniformMatrix2(int32_t location, core::SizeType count, bool transpose, const double *value);
extern void bindTransformFeedback(BindTransformFeedbackTarget target, uint32_t id);
extern void uniform3(int32_t location, core::SizeType count, const double *value);
extern void uniform1(int32_t location, core::SizeType count, const double *value);
extern void getActiveSubroutineUniform(uint32_t program, ShaderType shadertype, uint32_t index, SubroutineParameterName pname, int32_t *values);
extern void resumeTransformFeedback();
extern int32_t getSubroutineUniformLocation(uint32_t program, ShaderType shadertype, const char *name);
extern void endQueryIndexe(QueryTarget target, uint32_t index);
extern void drawArraysIndirect(PrimitiveType mode, const void *indirect);
extern void uniformMatrix4x3(int32_t location, core::SizeType count, bool transpose, const double *value);
extern void beginQueryIndexe(QueryTarget target, uint32_t index, uint32_t id);
extern void uniformMatrix2x4(int32_t location, core::SizeType count, bool transpose, const double *value);
extern void blendFunc(uint32_t buf, BlendingFactor src, BlendingFactor dst);
extern void uniformMatrix3x4(int32_t location, core::SizeType count, bool transpose, const double *value);
extern uint32_t getSubroutineIndex(uint32_t program, ShaderType shadertype, const char *name);
extern void getProgramStage(uint32_t program, ShaderType shadertype, ProgramStagePName pname, int32_t *values);
extern void uniformMatrix4(int32_t location, core::SizeType count, bool transpose, const double *value);
extern void getActiveSubroutineName(uint32_t program, ShaderType shadertype, uint32_t index, core::SizeType bufsize, core::SizeType *length, char *name);
extern void minSampleShading(float value);
extern void drawElementsIndirect(PrimitiveType mode, DrawElementsType type, const void *indirect);
extern void uniform1(int32_t location, double x);
extern void uniformMatrix2x3(int32_t location, core::SizeType count, bool transpose, const double *value);
extern void drawTransformFeedback(PrimitiveType mode, uint32_t id);
extern void patchParameter(PatchParameterName pname, const float *values);
extern void getUniformSubroutine(ShaderType shadertype, int32_t location, uint32_t *params);
extern void blendFuncSeparate(uint32_t buf, BlendingFactor srcRGB, BlendingFactor dstRGB, BlendingFactor srcAlpha, BlendingFactor dstAlpha);
extern void pauseTransformFeedback();
extern void uniform2(int32_t location, double x, double y);
extern void uniform2(int32_t location, core::SizeType count, const double *value);
extern void uniformMatrix3x2(int32_t location, core::SizeType count, bool transpose, const double *value);
extern void getActiveSubroutineUniformName(uint32_t program, ShaderType shadertype, uint32_t index, core::SizeType bufsize, core::SizeType *length, char *name);
extern bool isTransformFeedback(uint32_t id);
extern void uniformMatrix3(int32_t location, core::SizeType count, bool transpose, const double *value);
extern void deleteTransformFeedback(core::SizeType n, const uint32_t *ids);
extern void uniform4(int32_t location, core::SizeType count, const double *value);
extern void uniform4(int32_t location, double x, double y, double z, double w);
extern void patchParameter(PatchParameterName pname, int32_t value);
extern void uniformMatrix4x2(int32_t location, core::SizeType count, bool transpose, const double *value);
extern void blendEquationSeparate(uint32_t buf, BlendEquationModeEXT modeRGB, BlendEquationModeEXT modeAlpha);
extern void genTransformFeedback(core::SizeType n, uint32_t *ids);
#endif
#if defined(API_LEVEL_GL_VERSION_4_1_compatibility) || defined(API_LEVEL_GL_VERSION_4_1_core) || defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
extern void programUniform1(uint32_t program, int32_t location, uint32_t v0);
extern void programUniform3(uint32_t program, int32_t location, double v0, double v1, double v2);
extern void getProgramPipeline(uint32_t pipeline, PipelineParameterName pname, int32_t *params);
extern void programUniform3(uint32_t program, int32_t location, core::SizeType count, const uint32_t *value);
extern void programUniform4(uint32_t program, int32_t location, core::SizeType count, const double *value);
extern void programUniform2(uint32_t program, int32_t location, core::SizeType count, const int32_t *value);
extern void genProgramPipeline(core::SizeType n, uint32_t *pipelines);
extern void vertexAttribL4(uint32_t index, double x, double y, double z, double w);
extern void programUniformMatrix4(uint32_t program, int32_t location, core::SizeType count, bool transpose, const double *value);
extern void programUniform3(uint32_t program, int32_t location, uint32_t v0, uint32_t v1, uint32_t v2);
extern void vertexAttribL1(uint32_t index, double x);
extern void programUniform2(uint32_t program, int32_t location, core::SizeType count, const float *value);
extern void viewportArrayv(uint32_t first, core::SizeType count, const float *v);
extern void programUniform2(uint32_t program, int32_t location, float v0, float v1);
extern void programUniformMatrix2x3(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float *value);
extern void programUniformMatrix3(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float *value);
extern void programUniformMatrix2(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float *value);
extern void getProgramBinary(uint32_t program, core::SizeType bufSize, core::SizeType *length, core::EnumType *binaryFormat, void *binary);
extern void programUniformMatrix3x2(uint32_t program, int32_t location, core::SizeType count, bool transpose, const double *value);
extern void activeShaderProgram(uint32_t pipeline, uint32_t program);
extern void getShaderPrecisionFormat(ShaderType shadertype, PrecisionType precisiontype, int32_t *range, int32_t *precision);
extern void useProgramStage(uint32_t pipeline, core::Bitfield<UseProgramStageMask> stages, uint32_t program);
extern void programUniformMatrix4x2(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float *value);
extern void vertexAttribL2(uint32_t index, double x, double y);
extern void vertexAttribL3(uint32_t index, double x, double y, double z);
extern void vertexAttribLPointer(uint32_t index, int32_t size, VertexAttribPointerType type, core::SizeType stride, const void *pointer);
extern void programUniform1(uint32_t program, int32_t location, core::SizeType count, const double *value);
extern void programUniform1(uint32_t program, int32_t location, core::SizeType count, const int32_t *value);
extern void getFloati_v(core::EnumType target, uint32_t index, float *data);
extern void programUniform4(uint32_t program, int32_t location, double v0, double v1, double v2, double v3);
extern void programUniformMatrix2x4(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float *value);
extern void depthRange(float n, float f);
extern void programUniformMatrix4(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float *value);
extern void programUniformMatrix2x3(uint32_t program, int32_t location, core::SizeType count, bool transpose, const double *value);
extern uint32_t createShaderProgramv(ShaderType type, core::SizeType count, const char *const*strings);
extern void programUniform1(uint32_t program, int32_t location, int32_t v0);
extern void programUniform2(uint32_t program, int32_t location, core::SizeType count, const double *value);
extern void programUniformMatrix4x3(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float *value);
extern void programUniform1(uint32_t program, int32_t location, core::SizeType count, const float *value);
extern void programUniform2(uint32_t program, int32_t location, uint32_t v0, uint32_t v1);
extern void programUniform4(uint32_t program, int32_t location, float v0, float v1, float v2, float v3);
extern void programUniform1(uint32_t program, int32_t location, core::SizeType count, const uint32_t *value);
extern void programUniform4(uint32_t program, int32_t location, int32_t v0, int32_t v1, int32_t v2, int32_t v3);
extern void programUniform4(uint32_t program, int32_t location, uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3);
extern void scissorIndexe(uint32_t index, int32_t left, int32_t bottom, core::SizeType width, core::SizeType height);
extern void programUniform3(uint32_t program, int32_t location, core::SizeType count, const float *value);
extern bool isProgramPipeline(uint32_t pipeline);
extern void programUniformMatrix3x4(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float *value);
extern void programUniform1(uint32_t program, int32_t location, double v0);
extern void programUniformMatrix4x3(uint32_t program, int32_t location, core::SizeType count, bool transpose, const double *value);
extern void scissorArrayv(uint32_t first, core::SizeType count, const int32_t *v);
extern void programUniformMatrix2x4(uint32_t program, int32_t location, core::SizeType count, bool transpose, const double *value);
extern void programUniform4(uint32_t program, int32_t location, core::SizeType count, const float *value);
extern void releaseShaderCompiler();
extern void programUniform3(uint32_t program, int32_t location, core::SizeType count, const double *value);
extern void programUniformMatrix4x2(uint32_t program, int32_t location, core::SizeType count, bool transpose, const double *value);
extern void getDoublei_v(core::EnumType target, uint32_t index, double *data);
extern void programParameter(uint32_t program, ProgramParameterPName pname, int32_t value);
extern void validateProgramPipeline(uint32_t pipeline);
extern void programUniform3(uint32_t program, int32_t location, core::SizeType count, const int32_t *value);
extern void programUniformMatrix3(uint32_t program, int32_t location, core::SizeType count, bool transpose, const double *value);
extern void deleteProgramPipeline(core::SizeType n, const uint32_t *pipelines);
extern void programUniform4(uint32_t program, int32_t location, core::SizeType count, const int32_t *value);
extern void programUniform2(uint32_t program, int32_t location, int32_t v0, int32_t v1);
extern void shaderBinary(core::SizeType count, const uint32_t *shaders, core::EnumType binaryformat, const void *binary, core::SizeType length);
extern void scissorIndexe(uint32_t index, const int32_t *v);
extern void viewportIndexed(uint32_t index, float x, float y, float w, float h);
extern void programUniform1(uint32_t program, int32_t location, float v0);
extern void getVertexAttribL(uint32_t index, VertexAttribEnum pname, double *params);
extern void programBinary(uint32_t program, core::EnumType binaryFormat, const void *binary, core::SizeType length);
extern void programUniformMatrix2(uint32_t program, int32_t location, core::SizeType count, bool transpose, const double *value);
extern void programUniformMatrix3x4(uint32_t program, int32_t location, core::SizeType count, bool transpose, const double *value);
extern void programUniformMatrix3x2(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float *value);
extern void viewportIndexed(uint32_t index, const float *v);
extern void programUniform2(uint32_t program, int32_t location, core::SizeType count, const uint32_t *value);
extern void programUniform3(uint32_t program, int32_t location, int32_t v0, int32_t v1, int32_t v2);
extern void vertexAttribL4(uint32_t index, const double *v);
extern void vertexAttribL3(uint32_t index, const double *v);
extern void programUniform3(uint32_t program, int32_t location, float v0, float v1, float v2);
extern void depthRangeIndexe(uint32_t index, double n, double f);
extern void bindProgramPipeline(uint32_t pipeline);
extern void depthRangeArrayv(uint32_t first, core::SizeType count, const double *v);
extern void vertexAttribL1(uint32_t index, const double *v);
extern void vertexAttribL2(uint32_t index, const double *v);
extern void getProgramPipelineInfoLog(uint32_t pipeline, core::SizeType bufSize, core::SizeType *length, char *infoLog);
extern void programUniform4(uint32_t program, int32_t location, core::SizeType count, const uint32_t *value);
extern void clearDepth(float d);
extern void programUniform2(uint32_t program, int32_t location, double v0, double v1);
#endif
#if defined(API_LEVEL_GL_VERSION_4_2_compatibility) || defined(API_LEVEL_GL_VERSION_4_2_core) || defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
extern void drawElementsInstancedBaseVertexBaseInstance(PrimitiveType mode, core::SizeType count, DrawElementsType type, const void *indices, core::SizeType instancecount, int32_t basevertex, uint32_t baseinstance);
extern void drawElementsInstancedBaseInstance(PrimitiveType mode, core::SizeType count, PrimitiveType type, const void *indices, core::SizeType instancecount, uint32_t baseinstance);
extern void drawTransformFeedbackStreamInstance(PrimitiveType mode, uint32_t id, uint32_t stream, core::SizeType instancecount);
extern void memoryBarrier(core::Bitfield<MemoryBarrierMask> barriers);
extern void texStorage3D(TextureTarget target, core::SizeType levels, InternalFormat internalformat, core::SizeType width, core::SizeType height, core::SizeType depth);
extern void texStorage1D(TextureTarget target, core::SizeType levels, InternalFormat internalformat, core::SizeType width);
extern void texStorage2D(TextureTarget target, core::SizeType levels, InternalFormat internalformat, core::SizeType width, core::SizeType height);
extern void drawTransformFeedbackInstance(PrimitiveType mode, uint32_t id, core::SizeType instancecount);
extern void drawArraysInstancedBaseInstance(PrimitiveType mode, int32_t first, core::SizeType count, core::SizeType instancecount, uint32_t baseinstance);
extern void getInternalformat(TextureTarget target, InternalFormat internalformat, InternalFormatPName pname, core::SizeType bufSize, int32_t *params);
extern void getActiveAtomicCounterBuffer(uint32_t program, uint32_t bufferIndex, AtomicCounterBufferPName pname, int32_t *params);
extern void bindImageTexture(uint32_t unit, uint32_t texture, int32_t level, bool layered, int32_t layer, BufferAccessARB access, InternalFormat format);
#endif
#if defined(API_LEVEL_GL_VERSION_4_3_compatibility) || defined(API_LEVEL_GL_VERSION_4_3_core) || defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
extern void copyImageSubData(uint32_t srcName, TextureTarget srcTarget, int32_t srcLevel, int32_t srcX, int32_t srcY, int32_t srcZ, uint32_t dstName, TextureTarget dstTarget, int32_t dstLevel, int32_t dstX, int32_t dstY, int32_t dstZ, core::SizeType srcWidth, core::SizeType srcHeight, core::SizeType srcDepth);
extern void invalidateSubFramebuffer(FramebufferTarget target, core::SizeType numAttachments, const FramebufferAttachment *attachments, int32_t x, int32_t y, core::SizeType width, core::SizeType height);
extern int32_t getProgramResourceLocationIndex(uint32_t program, ProgramInterface programInterface, const char *name);
extern void getProgramInterface(uint32_t program, ProgramInterface programInterface, ProgramInterfacePName pname, int32_t *params);
extern void dispatchComputeIndirect(std::intptr_t indirect);
extern void invalidateTexSubImage(uint32_t texture, int32_t level, int32_t xoffset, int32_t yoffset, int32_t zoffset, core::SizeType width, core::SizeType height, core::SizeType depth);
extern void shaderStorageBlockBinding(uint32_t program, uint32_t storageBlockIndex, uint32_t storageBlockBinding);
extern void debugMessageCallback(core::DebugProc callback, const void *userParam);
extern void getProgramResource(uint32_t program, ProgramInterface programInterface, uint32_t index, core::SizeType propCount, const ProgramResourceProperty *props, core::SizeType bufSize, core::SizeType *length, int32_t *params);
extern void texStorage2DMultisample(TextureTarget target, core::SizeType samples, InternalFormat internalformat, core::SizeType width, core::SizeType height, bool fixedsamplelocations);
extern void getObjectPtrLabel(const void *ptr, core::SizeType bufSize, core::SizeType *length, char *label);
extern void objectPtrLabel(const void *ptr, core::SizeType length, const char *label);
extern void vertexBindingDivisor(uint32_t bindingindex, uint32_t divisor);
extern void objectLabel(ObjectIdentifier identifier, uint32_t name, core::SizeType length, const char *label);
extern void invalidateTexImage(uint32_t texture, int32_t level);
extern void debugMessageInsert(DebugSource source, DebugType type, uint32_t id, DebugSeverity severity, core::SizeType length, const char *buf);
extern void popDebugGroup();
extern void getObjectLabel(ObjectIdentifier identifier, uint32_t name, core::SizeType bufSize, core::SizeType *length, char *label);
extern void debugMessageControl(DebugSource source, DebugType type, DebugSeverity severity, core::SizeType count, const uint32_t *ids, bool enabled);
extern int32_t getProgramResourceLocation(uint32_t program, ProgramInterface programInterface, const char *name);
extern void getProgramResourceName(uint32_t program, ProgramInterface programInterface, uint32_t index, core::SizeType bufSize, core::SizeType *length, char *name);
extern void multiDrawElementsIndirect(PrimitiveType mode, DrawElementsType type, const void *indirect, core::SizeType drawcount, core::SizeType stride);
extern void textureView(uint32_t texture, TextureTarget target, uint32_t origtexture, InternalFormat internalformat, uint32_t minlevel, uint32_t numlevels, uint32_t minlayer, uint32_t numlayers);
extern void dispatchCompute(uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z);
extern void invalidateFramebuffer(FramebufferTarget target, core::SizeType numAttachments, const FramebufferAttachment *attachments);
extern uint32_t getProgramResourceIndex(uint32_t program, ProgramInterface programInterface, const char *name);
extern void invalidateBufferData(uint32_t buffer);
extern void pushDebugGroup(DebugSource source, uint32_t id, core::SizeType length, const char *message);
extern void vertexAttribLFormat(uint32_t attribindex, int32_t size, VertexAttribLType type, uint32_t relativeoffset);
extern void clearBufferData(BufferStorageTarget target, InternalFormat internalformat, PixelFormat format, PixelType type, const void *data);
extern void getInternalformat(TextureTarget target, InternalFormat internalformat, InternalFormatPName pname, core::SizeType bufSize, int64_t *params);
extern void vertexAttribFormat(uint32_t attribindex, int32_t size, VertexAttribType type, bool normalized, uint32_t relativeoffset);
extern void vertexAttribIFormat(uint32_t attribindex, int32_t size, VertexAttribIType type, uint32_t relativeoffset);
extern void getFramebufferParameter(FramebufferTarget target, FramebufferAttachmentParameterName pname, int32_t *params);
extern void texBufferRange(TextureTarget target, InternalFormat internalformat, uint32_t buffer, std::intptr_t offset, std::size_t size);
extern void clearBufferSubData(BufferTargetARB target, InternalFormat internalformat, std::intptr_t offset, std::size_t size, PixelFormat format, PixelType type, const void *data);
extern uint32_t getDebugMessageLog(uint32_t count, core::SizeType bufSize, DebugSource *sources, DebugType *types, uint32_t *ids, DebugSeverity *severities, core::SizeType *lengths, char *messageLog);
extern void texStorage3DMultisample(TextureTarget target, core::SizeType samples, InternalFormat internalformat, core::SizeType width, core::SizeType height, core::SizeType depth, bool fixedsamplelocations);
extern void framebufferParameter(FramebufferTarget target, FramebufferParameterName pname, int32_t param);
extern void vertexAttribBinding(uint32_t attribindex, uint32_t bindingindex);
extern void invalidateBufferSubData(uint32_t buffer, std::intptr_t offset, std::size_t length);
extern void multiDrawArraysIndirect(PrimitiveType mode, const void *indirect, core::SizeType drawcount, core::SizeType stride);
extern void bindVertexBuffer(uint32_t bindingindex, uint32_t buffer, std::intptr_t offset, core::SizeType stride);
#endif
#if defined(API_LEVEL_GL_VERSION_4_4_compatibility) || defined(API_LEVEL_GL_VERSION_4_4_core) || defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
extern void bindBuffersRange(BufferTargetARB target, uint32_t first, core::SizeType count, const uint32_t *buffers, const std::intptr_t *offsets, const std::size_t *sizes);
extern void bindImageTextures(uint32_t first, core::SizeType count, const uint32_t *textures);
extern void bindTextures(uint32_t first, core::SizeType count, const uint32_t *textures);
extern void clearTexImage(uint32_t texture, int32_t level, PixelFormat format, PixelType type, const void *data);
extern void bindSampler(uint32_t first, core::SizeType count, const uint32_t *samplers);
extern void clearTexSubImage(uint32_t texture, int32_t level, int32_t xoffset, int32_t yoffset, int32_t zoffset, core::SizeType width, core::SizeType height, core::SizeType depth, PixelFormat format, PixelType type, const void *data);
extern void bindBuffersBase(BufferTargetARB target, uint32_t first, core::SizeType count, const uint32_t *buffers);
extern void bufferStorage(BufferStorageTarget target, std::size_t size, const void *data, core::Bitfield<BufferStorageMask> flags);
extern void bindVertexBuffers(uint32_t first, core::SizeType count, const uint32_t *buffers, const std::intptr_t *offsets, const core::SizeType *strides);
#endif
#if defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_5_core) || defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
extern void disableVertexArrayAttri(uint32_t vaobj, uint32_t index);
extern core::EnumType getGraphicsResetStatus();
extern void copyTextureSubImage1D(uint32_t texture, int32_t level, int32_t xoffset, int32_t x, int32_t y, core::SizeType width);
extern void getVertexArrayIndexed(uint32_t vaobj, uint32_t index, VertexArrayPName pname, int32_t *param);
extern void copyTextureSubImage2D(uint32_t texture, int32_t level, int32_t xoffset, int32_t yoffset, int32_t x, int32_t y, core::SizeType width, core::SizeType height);
extern bool unmapNamedBuffer(uint32_t buffer);
extern void textureStorage3DMultisample(uint32_t texture, core::SizeType samples, InternalFormat internalformat, core::SizeType width, core::SizeType height, core::SizeType depth, bool fixedsamplelocations);
extern void getTransformFeedbacki_v(uint32_t xfb, TransformFeedbackPName pname, uint32_t index, int32_t *param);
extern void getNamedBufferParameter(uint32_t buffer, VertexBufferObjectParameter pname, int64_t *params);
extern void *mapNamedBufferRange(uint32_t buffer, std::intptr_t offset, std::size_t length, core::Bitfield<MapBufferAccessMask> access);
extern void textureParameter(uint32_t texture, TextureParameterName pname, const float *param);
extern void clipControl(ClipControlOrigin origin, ClipControlDepth depth);
extern void namedRenderbufferStorage(uint32_t renderbuffer, InternalFormat internalformat, core::SizeType width, core::SizeType height);
extern void clearNamedFramebufferf(uint32_t framebuffer, Buffer buffer, int32_t drawbuffer, float depth, int32_t stencil);
extern void getnUniform(uint32_t program, int32_t location, core::SizeType bufSize, double *params);
extern void textureParameterI(uint32_t texture, TextureParameterName pname, const int32_t *params);
extern void vertexArrayAttribBinding(uint32_t vaobj, uint32_t attribindex, uint32_t bindingindex);
extern void createQuerie(QueryTarget target, core::SizeType n, uint32_t *ids);
extern void textureSubImage2D(uint32_t texture, int32_t level, int32_t xoffset, int32_t yoffset, core::SizeType width, core::SizeType height, PixelFormat format, PixelType type, const void *pixels);
extern void getNamedFramebufferAttachmentParameter(uint32_t framebuffer, FramebufferAttachment attachment, FramebufferAttachmentParameterName pname, int32_t *params);
extern void namedBufferSubData(uint32_t buffer, std::intptr_t offset, std::size_t size, const void *data);
extern void getNamedFramebufferParameter(uint32_t framebuffer, GetFramebufferParameter pname, int32_t *param);
extern void textureStorage2D(uint32_t texture, core::SizeType levels, InternalFormat internalformat, core::SizeType width, core::SizeType height);
extern void createProgramPipeline(core::SizeType n, uint32_t *pipelines);
extern void invalidateNamedFramebufferSubData(uint32_t framebuffer, core::SizeType numAttachments, const FramebufferAttachment *attachments, int32_t x, int32_t y, core::SizeType width, core::SizeType height);
extern void vertexArrayAttribFormat(uint32_t vaobj, uint32_t attribindex, int32_t size, VertexAttribType type, bool normalized, uint32_t relativeoffset);
extern void textureBuffer(uint32_t texture, InternalFormat internalformat, uint32_t buffer);
extern void namedBufferData(uint32_t buffer, std::size_t size, const void *data, VertexBufferObjectUsage usage);
extern void copyTextureSubImage3D(uint32_t texture, int32_t level, int32_t xoffset, int32_t yoffset, int32_t zoffset, int32_t x, int32_t y, core::SizeType width, core::SizeType height);
extern void generateTextureMipmap(uint32_t texture);
extern void getNamedBufferPointerv(uint32_t buffer, VertexBufferObjectParameter pname, void **params);
extern void getTextureLevelParameter(uint32_t texture, int32_t level, GetTextureParameter pname, float *params);
extern void vertexArrayElementBuffer(uint32_t vaobj, uint32_t buffer);
extern core::EnumType checkNamedFramebufferStatus(uint32_t framebuffer, FramebufferTarget target);
extern void textureStorage2DMultisample(uint32_t texture, core::SizeType samples, InternalFormat internalformat, core::SizeType width, core::SizeType height, bool fixedsamplelocations);
extern void createSampler(core::SizeType n, uint32_t *samplers);
extern void getnUniform(uint32_t program, int32_t location, core::SizeType bufSize, int32_t *params);
extern void *mapNamedBuffer(uint32_t buffer, BufferAccessARB access);
extern void clearNamedFramebuffer(uint32_t framebuffer, Buffer buffer, int32_t drawbuffer, const float *value);
extern void clearNamedFramebuffer(uint32_t framebuffer, Buffer buffer, int32_t drawbuffer, const uint32_t *value);
extern void createRenderbuffers(core::SizeType n, uint32_t *renderbuffers);
extern void getTextureParameter(uint32_t texture, GetTextureParameter pname, float *params);
extern void vertexArrayAttribIFormat(uint32_t vaobj, uint32_t attribindex, int32_t size, VertexAttribIType type, uint32_t relativeoffset);
extern void getTransformFeedbacki64_v(uint32_t xfb, TransformFeedbackPName pname, uint32_t index, int64_t *param);
extern void textureParameter(uint32_t texture, TextureParameterName pname, int32_t param);
extern void getNamedBufferParameter(uint32_t buffer, VertexBufferObjectParameter pname, int32_t *params);
extern void flushMappedNamedBufferRange(uint32_t buffer, std::intptr_t offset, std::size_t length);
extern void getTextureSubImage(uint32_t texture, int32_t level, int32_t xoffset, int32_t yoffset, int32_t zoffset, core::SizeType width, core::SizeType height, core::SizeType depth, PixelFormat format, PixelType type, core::SizeType bufSize, void *pixels);
extern void readnPixel(int32_t x, int32_t y, core::SizeType width, core::SizeType height, PixelFormat format, PixelType type, core::SizeType bufSize, void *data);
extern void vertexArrayBindingDivisor(uint32_t vaobj, uint32_t bindingindex, uint32_t divisor);
extern void transformFeedbackBufferBase(uint32_t xfb, uint32_t index, uint32_t buffer);
extern void getnTexImage(TextureTarget target, int32_t level, PixelFormat format, PixelType type, core::SizeType bufSize, void *pixels);
extern void getTextureImage(uint32_t texture, int32_t level, PixelFormat format, PixelType type, core::SizeType bufSize, void *pixels);
extern void namedFramebufferTextureLayer(uint32_t framebuffer, FramebufferAttachment attachment, uint32_t texture, int32_t level, int32_t layer);
extern void compressedTextureSubImage1D(uint32_t texture, int32_t level, int32_t xoffset, core::SizeType width, PixelFormat format, core::SizeType imageSize, const void *data);
extern void getCompressedTextureSubImage(uint32_t texture, int32_t level, int32_t xoffset, int32_t yoffset, int32_t zoffset, core::SizeType width, core::SizeType height, core::SizeType depth, core::SizeType bufSize, void *pixels);
extern void getTextureLevelParameter(uint32_t texture, int32_t level, GetTextureParameter pname, int32_t *params);
extern void namedFramebufferReadBuffer(uint32_t framebuffer, ColorBuffer src);
extern void createFramebuffers(core::SizeType n, uint32_t *framebuffers);
extern void vertexArrayAttribLFormat(uint32_t vaobj, uint32_t attribindex, int32_t size, VertexAttribLType type, uint32_t relativeoffset);
extern void getQueryBufferObject(uint32_t id, uint32_t buffer, QueryObjectParameterName pname, std::intptr_t offset);
extern void getTextureParameterI(uint32_t texture, GetTextureParameter pname, uint32_t *params);
extern void clearNamedBufferSubData(uint32_t buffer, InternalFormat internalformat, std::intptr_t offset, std::size_t size, PixelFormat format, PixelType type, const void *data);
extern void getTextureParameterI(uint32_t texture, GetTextureParameter pname, int32_t *params);
extern void enableVertexArrayAttri(uint32_t vaobj, uint32_t index);
extern void textureParameterI(uint32_t texture, TextureParameterName pname, const uint32_t *params);
extern void invalidateNamedFramebufferData(uint32_t framebuffer, core::SizeType numAttachments, const FramebufferAttachment *attachments);
extern void getQueryBufferObject(uint32_t id, uint32_t buffer, QueryObjectParameterName pname, std::intptr_t offset);
extern void getnUniform(uint32_t program, int32_t location, core::SizeType bufSize, float *params);
extern void transformFeedbackBufferRange(uint32_t xfb, uint32_t index, uint32_t buffer, std::intptr_t offset, std::size_t size);
extern void getVertexArrayIndexed64(uint32_t vaobj, uint32_t index, VertexArrayPName pname, int64_t *param);
extern void namedFramebufferDrawBuffer(uint32_t framebuffer, ColorBuffer buf);
extern void getCompressedTextureImage(uint32_t texture, int32_t level, core::SizeType bufSize, void *pixels);
extern void textureStorage1D(uint32_t texture, core::SizeType levels, InternalFormat internalformat, core::SizeType width);
extern void createVertexArrays(core::SizeType n, uint32_t *arrays);
extern void blitNamedFramebuffer(uint32_t readFramebuffer, uint32_t drawFramebuffer, int32_t srcX0, int32_t srcY0, int32_t srcX1, int32_t srcY1, int32_t dstX0, int32_t dstY0, int32_t dstX1, int32_t dstY1, core::Bitfield<ClearBufferMask> mask, BlitFramebufferFilter filter);
extern void textureSubImage1D(uint32_t texture, int32_t level, int32_t xoffset, core::SizeType width, PixelFormat format, PixelType type, const void *pixels);
extern void namedFramebufferDrawBuffers(uint32_t framebuffer, core::SizeType n, const ColorBuffer *bufs);
extern void namedRenderbufferStorageMultisample(uint32_t renderbuffer, core::SizeType samples, InternalFormat internalformat, core::SizeType width, core::SizeType height);
extern void getTransformFeedback(uint32_t xfb, TransformFeedbackPName pname, int32_t *param);
extern void memoryBarrierByRegion(core::Bitfield<MemoryBarrierMask> barriers);
extern void textureParameter(uint32_t texture, TextureParameterName pname, float param);
extern void namedFramebufferTexture(uint32_t framebuffer, FramebufferAttachment attachment, uint32_t texture, int32_t level);
extern void getVertexArray(uint32_t vaobj, VertexArrayPName pname, int32_t *param);
extern void compressedTextureSubImage3D(uint32_t texture, int32_t level, int32_t xoffset, int32_t yoffset, int32_t zoffset, core::SizeType width, core::SizeType height, core::SizeType depth, PixelFormat format, core::SizeType imageSize, const void *data);
extern void textureBarrier();
extern void textureParameter(uint32_t texture, TextureParameterName pname, const int32_t *param);
extern void createTextures(TextureTarget target, core::SizeType n, uint32_t *textures);
extern void getnUniform(uint32_t program, int32_t location, core::SizeType bufSize, uint32_t *params);
extern void vertexArrayVertexBuffers(uint32_t vaobj, uint32_t first, core::SizeType count, const uint32_t *buffers, const std::intptr_t *offsets, const core::SizeType *strides);
extern void createTransformFeedback(core::SizeType n, uint32_t *ids);
extern void vertexArrayVertexBuffer(uint32_t vaobj, uint32_t bindingindex, uint32_t buffer, std::intptr_t offset, core::SizeType stride);
extern void clearNamedBufferData(uint32_t buffer, InternalFormat internalformat, PixelFormat format, PixelType type, const void *data);
extern void getQueryBufferObject(uint32_t id, uint32_t buffer, QueryObjectParameterName pname, std::intptr_t offset);
extern void textureStorage3D(uint32_t texture, core::SizeType levels, InternalFormat internalformat, core::SizeType width, core::SizeType height, core::SizeType depth);
extern void getnCompressedTexImage(TextureTarget target, int32_t lod, core::SizeType bufSize, void *pixels);
extern void textureSubImage3D(uint32_t texture, int32_t level, int32_t xoffset, int32_t yoffset, int32_t zoffset, core::SizeType width, core::SizeType height, core::SizeType depth, PixelFormat format, PixelType type, const void *pixels);
extern void namedFramebufferRenderbuffer(uint32_t framebuffer, FramebufferAttachment attachment, RenderbufferTarget renderbuffertarget, uint32_t renderbuffer);
extern void compressedTextureSubImage2D(uint32_t texture, int32_t level, int32_t xoffset, int32_t yoffset, core::SizeType width, core::SizeType height, PixelFormat format, core::SizeType imageSize, const void *data);
extern void copyNamedBufferSubData(uint32_t readBuffer, uint32_t writeBuffer, std::intptr_t readOffset, std::intptr_t writeOffset, std::size_t size);
extern void getQueryBufferObject(uint32_t id, uint32_t buffer, QueryObjectParameterName pname, std::intptr_t offset);
extern void namedBufferStorage(uint32_t buffer, std::size_t size, const void *data, core::Bitfield<BufferStorageMask> flags);
extern void bindTextureUnit(uint32_t unit, uint32_t texture);
extern void createBuffers(core::SizeType n, uint32_t *buffers);
extern void getNamedBufferSubData(uint32_t buffer, std::intptr_t offset, std::size_t size, void *data);
extern void getTextureParameter(uint32_t texture, GetTextureParameter pname, int32_t *params);
extern void getNamedRenderbufferParameter(uint32_t renderbuffer, RenderbufferParameterName pname, int32_t *params);
extern void clearNamedFramebuffer(uint32_t framebuffer, Buffer buffer, int32_t drawbuffer, const int32_t *value);
extern void textureBufferRange(uint32_t texture, InternalFormat internalformat, uint32_t buffer, std::intptr_t offset, std::size_t size);
extern void namedFramebufferParameter(uint32_t framebuffer, FramebufferParameterName pname, int32_t param);
#endif
#if defined(API_LEVEL_GL_VERSION_4_5_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_compatibility)
extern void getnMinmax(MinmaxTargetEXT target, bool reset, PixelFormat format, PixelType type, core::SizeType bufSize, void *values);
extern void getnMap(MapTarget target, MapQuery query, core::SizeType bufSize, float *v);
extern void getnPixelMap(PixelMap map, core::SizeType bufSize, uint32_t *values);
extern void getnMap(MapTarget target, MapQuery query, core::SizeType bufSize, double *v);
extern void getnPolygonStipple(core::SizeType bufSize, uint8_t *pattern);
extern void getnMap(MapTarget target, MapQuery query, core::SizeType bufSize, int32_t *v);
extern void getnColorTable(ColorTableTarget target, PixelFormat format, PixelType type, core::SizeType bufSize, void *table);
extern void getnPixelMap(PixelMap map, core::SizeType bufSize, float *values);
extern void getnConvolutionFilter(ConvolutionTarget target, PixelFormat format, PixelType type, core::SizeType bufSize, void *image);
extern void getnSeparableFilter(SeparableTargetEXT target, PixelFormat format, PixelType type, core::SizeType rowBufSize, void *row, core::SizeType columnBufSize, void *column, void *span);
extern void getnHistogram(HistogramTargetEXT target, bool reset, PixelFormat format, PixelType type, core::SizeType bufSize, void *values);
extern void getnPixelMap(PixelMap map, core::SizeType bufSize, uint16_t *values);
#endif
#if defined(API_LEVEL_GL_VERSION_4_6_compatibility) || defined(API_LEVEL_GL_VERSION_4_6_core)
extern void multiDrawArraysIndirectCount(PrimitiveType mode, const void *indirect, std::intptr_t drawcount, core::SizeType maxdrawcount, core::SizeType stride);
extern void polygonOffsetClamp(float factor, float units, float clamp);
extern void multiDrawElementsIndirectCount(PrimitiveType mode, DrawElementsType type, const void *indirect, std::intptr_t drawcount, core::SizeType maxdrawcount, core::SizeType stride);
extern void specializeShader(uint32_t shader, const char *pEntryPoint, uint32_t numSpecializationConstants, const uint32_t *pConstantIndex, const uint32_t *pConstantValue);
#endif

}
