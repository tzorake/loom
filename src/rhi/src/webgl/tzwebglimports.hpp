#pragma once

// WASM_IMPORT declarations for the WebGL 2.0 surface.
//
// JavaScript provides these functions in the "webgl" import object.
// All GL object handles are integer indices into JS-side arrays.

#if defined(__wasm__) || defined(__EMSCRIPTEN__)
#  define WASM_IMPORT_AS(name) __attribute__((import_module("webgl"), import_name(name)))
#else
#  define WASM_IMPORT_AS(name)
#endif

extern "C" {

// ── Buffer ────────────────────────────────────────────────────────────────────
WASM_IMPORT_AS("gl_create_buffer")       int  gl_create_buffer();
WASM_IMPORT_AS("gl_bind_buffer")         void gl_bind_buffer(int target, int handle);
WASM_IMPORT_AS("gl_buffer_data_static")  void gl_buffer_data_static(int target, const void *ptr, int byteSize);
WASM_IMPORT_AS("gl_buffer_sub_data")     void gl_buffer_sub_data(int target, int offset, const void *ptr, int byteSize);
WASM_IMPORT_AS("gl_delete_buffer")       void gl_delete_buffer(int handle);

// ── Texture ───────────────────────────────────────────────────────────────────
WASM_IMPORT_AS("gl_create_texture")      int  gl_create_texture();
WASM_IMPORT_AS("gl_bind_texture")        void gl_bind_texture(int target, int handle);
WASM_IMPORT_AS("gl_tex_image_2d")        void gl_tex_image_2d(int target, int level, int internalFormat,
                                                               int width, int height, int border,
                                                               int format, int type, const void *ptr, int byteSize);
WASM_IMPORT_AS("gl_tex_sub_image_2d")    void gl_tex_sub_image_2d(int target, int level,
                                                                    int xoffset, int yoffset,
                                                                    int width, int height,
                                                                    int format, int type,
                                                                    const void *ptr, int byteSize);
WASM_IMPORT_AS("gl_tex_parameteri")      void gl_tex_parameteri(int target, int pname, int param);
WASM_IMPORT_AS("gl_active_texture")      void gl_active_texture(int texture);
WASM_IMPORT_AS("gl_delete_texture")      void gl_delete_texture(int handle);

// ── Sampler ───────────────────────────────────────────────────────────────────
WASM_IMPORT_AS("gl_create_sampler")      int  gl_create_sampler();
WASM_IMPORT_AS("gl_bind_sampler")        void gl_bind_sampler(int unit, int handle);
WASM_IMPORT_AS("gl_sampler_parameteri")  void gl_sampler_parameteri(int handle, int pname, int param);
WASM_IMPORT_AS("gl_delete_sampler")      void gl_delete_sampler(int handle);

// ── Shader / Program ─────────────────────────────────────────────────────────
WASM_IMPORT_AS("gl_create_shader")            int  gl_create_shader(int type);
WASM_IMPORT_AS("gl_shader_source")            void gl_shader_source(int handle, const char *ptr, int len);
WASM_IMPORT_AS("gl_compile_shader")           void gl_compile_shader(int handle);
WASM_IMPORT_AS("gl_get_shader_compile_status") int gl_get_shader_compile_status(int handle);
WASM_IMPORT_AS("gl_delete_shader")            void gl_delete_shader(int handle);
WASM_IMPORT_AS("gl_create_program")           int  gl_create_program();
WASM_IMPORT_AS("gl_attach_shader")            void gl_attach_shader(int program, int shader);
WASM_IMPORT_AS("gl_link_program")             void gl_link_program(int program);
WASM_IMPORT_AS("gl_get_program_link_status")  int  gl_get_program_link_status(int program);
WASM_IMPORT_AS("gl_use_program")              void gl_use_program(int program);
WASM_IMPORT_AS("gl_delete_program")           void gl_delete_program(int program);
WASM_IMPORT_AS("gl_get_uniform_block_index")        int  gl_get_uniform_block_index(int program,
                                                                                        const char *namePtr, int nameLen);
WASM_IMPORT_AS("gl_uniform_block_binding")          void gl_uniform_block_binding(int program,
                                                                                    int blockIndex, int binding);
WASM_IMPORT_AS("gl_get_active_uniform_block_count") int  gl_get_active_uniform_block_count(int program);
WASM_IMPORT_AS("gl_get_active_uniform_count")       int  gl_get_active_uniform_count(int program);
WASM_IMPORT_AS("gl_get_active_uniform_type")        int  gl_get_active_uniform_type(int program, int index);
WASM_IMPORT_AS("gl_get_active_uniform_name")        void gl_get_active_uniform_name(int program, int index,
                                                                                     char *namePtr, int maxLen);
WASM_IMPORT_AS("gl_get_uniform_location")           int  gl_get_uniform_location(int program,
                                                                                   const char *namePtr, int nameLen);
WASM_IMPORT_AS("gl_uniform1i")                      void gl_uniform1i(int location, int value);

// ── Vertex Array Object ───────────────────────────────────────────────────────
WASM_IMPORT_AS("gl_create_vertex_array")        int  gl_create_vertex_array();
WASM_IMPORT_AS("gl_bind_vertex_array")          void gl_bind_vertex_array(int handle);
WASM_IMPORT_AS("gl_enable_vertex_attrib_array") void gl_enable_vertex_attrib_array(int index);
WASM_IMPORT_AS("gl_vertex_attrib_pointer")      void gl_vertex_attrib_pointer(int index, int size,
                                                                                int type, int normalized,
                                                                                int stride, int offset);
WASM_IMPORT_AS("gl_delete_vertex_array")        void gl_delete_vertex_array(int handle);

// ── Draw ──────────────────────────────────────────────────────────────────────
WASM_IMPORT_AS("gl_draw_arrays")    void gl_draw_arrays(int mode, int first, int count);
WASM_IMPORT_AS("gl_draw_elements")  void gl_draw_elements(int mode, int count, int type, int offset);

// ── State ─────────────────────────────────────────────────────────────────────
WASM_IMPORT_AS("gl_enable")                  void gl_enable(int cap);
WASM_IMPORT_AS("gl_disable")                 void gl_disable(int cap);
WASM_IMPORT_AS("gl_blend_func_separate")     void gl_blend_func_separate(int srcRGB, int dstRGB,
                                                                           int srcAlpha, int dstAlpha);
WASM_IMPORT_AS("gl_blend_equation_separate") void gl_blend_equation_separate(int modeRGB, int modeAlpha);
WASM_IMPORT_AS("gl_depth_func")              void gl_depth_func(int func);
WASM_IMPORT_AS("gl_depth_mask")              void gl_depth_mask(int flag);
WASM_IMPORT_AS("gl_cull_face")               void gl_cull_face(int mode);
WASM_IMPORT_AS("gl_front_face")              void gl_front_face(int mode);
WASM_IMPORT_AS("gl_viewport")                void gl_viewport(int x, int y, int width, int height);
WASM_IMPORT_AS("gl_clear_color")             void gl_clear_color(float r, float g, float b, float a);
WASM_IMPORT_AS("gl_clear")                   void gl_clear(int mask);
WASM_IMPORT_AS("gl_scissor")                 void gl_scissor(int x, int y, int width, int height);
WASM_IMPORT_AS("gl_line_width")              void gl_line_width(float width);

// ── Framebuffer ───────────────────────────────────────────────────────────────
WASM_IMPORT_AS("gl_create_framebuffer")       int  gl_create_framebuffer();
WASM_IMPORT_AS("gl_bind_framebuffer")         void gl_bind_framebuffer(int target, int handle);
WASM_IMPORT_AS("gl_framebuffer_texture_2d")   void gl_framebuffer_texture_2d(int target, int attachment,
                                                                               int textarget, int texture, int level);
WASM_IMPORT_AS("gl_delete_framebuffer")       void gl_delete_framebuffer(int handle);

// ── UBO ───────────────────────────────────────────────────────────────────────
WASM_IMPORT_AS("gl_bind_buffer_range")  void gl_bind_buffer_range(int target, int index, int buffer,
                                                                    int offset, int size);

} // extern "C"

// ── WebGL 2.0 enum constants (mirrors GL ES 3.0 / WebGL2 values) ─────────────
// These match the integer values in the WebGL spec and are safe to pass to the
// JS import functions above.

enum : int {
    WEBGL_ARRAY_BUFFER              = 0x8892,
    WEBGL_ELEMENT_ARRAY_BUFFER      = 0x8893,
    WEBGL_UNIFORM_BUFFER            = 0x8A11,
    WEBGL_TEXTURE_2D                = 0x0DE1,
    WEBGL_TEXTURE0                  = 0x84C0,
    WEBGL_RGBA                      = 0x1908,
    WEBGL_RGBA8                     = 0x8058,
    WEBGL_R8                        = 0x8229,
    WEBGL_RG8                       = 0x822B,
    WEBGL_R16F                      = 0x822D,
    WEBGL_RGBA16F                   = 0x881A,
    WEBGL_RGBA32F                   = 0x8814,
    WEBGL_DEPTH_COMPONENT16         = 0x81A5,
    WEBGL_DEPTH_COMPONENT32F        = 0x8CAC,
    WEBGL_DEPTH24_STENCIL8          = 0x88F0,
    WEBGL_DEPTH_COMPONENT           = 0x1902,
    WEBGL_DEPTH_STENCIL             = 0x84F9,
    WEBGL_RED                       = 0x1903,
    WEBGL_RG                        = 0x8227,
    WEBGL_HALF_FLOAT                = 0x140B,
    WEBGL_FLOAT                     = 0x1406,
    WEBGL_UNSIGNED_BYTE             = 0x1401,
    WEBGL_UNSIGNED_SHORT            = 0x1403,
    WEBGL_UNSIGNED_INT              = 0x1405,
    WEBGL_UNSIGNED_INT_24_8         = 0x84FA,
    WEBGL_TEXTURE_MIN_FILTER        = 0x2801,
    WEBGL_TEXTURE_MAG_FILTER        = 0x2800,
    WEBGL_TEXTURE_WRAP_S            = 0x2802,
    WEBGL_TEXTURE_WRAP_T            = 0x2803,
    WEBGL_NEAREST                   = 0x2600,
    WEBGL_LINEAR                    = 0x2601,
    WEBGL_LINEAR_MIPMAP_LINEAR      = 0x2703,
    WEBGL_REPEAT                    = 0x2901,
    WEBGL_MIRRORED_REPEAT           = 0x8370,
    WEBGL_CLAMP_TO_EDGE             = 0x812F,
    WEBGL_COLOR_BUFFER_BIT          = 0x00004000,
    WEBGL_DEPTH_BUFFER_BIT          = 0x00000100,
    WEBGL_STENCIL_BUFFER_BIT        = 0x00000400,
    WEBGL_TRIANGLES                 = 0x0004,
    WEBGL_TRIANGLE_STRIP            = 0x0005,
    WEBGL_LINES                     = 0x0001,
    WEBGL_POINTS                    = 0x0000,
    WEBGL_FRAGMENT_SHADER           = 0x8B30,
    WEBGL_VERTEX_SHADER             = 0x8B31,
    WEBGL_LESS                      = 0x0201,
    WEBGL_LEQUAL                    = 0x0203,
    WEBGL_EQUAL                     = 0x0202,
    WEBGL_GREATER                   = 0x0204,
    WEBGL_NOTEQUAL                  = 0x0205,
    WEBGL_GEQUAL                    = 0x0206,
    WEBGL_ALWAYS                    = 0x0207,
    WEBGL_NEVER                     = 0x0200,
    WEBGL_FRONT                     = 0x0404,
    WEBGL_BACK                      = 0x0405,
    WEBGL_CW                        = 0x0900,
    WEBGL_CCW                       = 0x0901,
    WEBGL_CULL_FACE                 = 0x0B44,
    WEBGL_DEPTH_TEST                = 0x0B71,
    WEBGL_BLEND                     = 0x0BE2,
    WEBGL_SCISSOR_TEST              = 0x0C11,
    WEBGL_ZERO                      = 0,
    WEBGL_ONE                       = 1,
    WEBGL_SRC_COLOR                 = 0x0300,
    WEBGL_ONE_MINUS_SRC_COLOR       = 0x0301,
    WEBGL_DST_COLOR                 = 0x0306,
    WEBGL_ONE_MINUS_DST_COLOR       = 0x0307,
    WEBGL_SRC_ALPHA                 = 0x0302,
    WEBGL_ONE_MINUS_SRC_ALPHA       = 0x0303,
    WEBGL_DST_ALPHA                 = 0x0304,
    WEBGL_ONE_MINUS_DST_ALPHA       = 0x0305,
    WEBGL_CONSTANT_COLOR            = 0x8001,
    WEBGL_ONE_MINUS_CONSTANT_COLOR  = 0x8002,
    WEBGL_SRC_ALPHA_SATURATE        = 0x0308,
    WEBGL_FUNC_ADD                  = 0x8006,
    WEBGL_FUNC_SUBTRACT             = 0x800A,
    WEBGL_FUNC_REVERSE_SUBTRACT     = 0x800B,
    WEBGL_MIN                       = 0x8007,
    WEBGL_MAX                       = 0x8008,
    WEBGL_FRAMEBUFFER               = 0x8D40,
    WEBGL_COLOR_ATTACHMENT0         = 0x8CE0,
    WEBGL_SAMPLER_2D                = 0x8B5E,
    WEBGL_SAMPLER_3D                = 0x8B5F,
    WEBGL_SAMPLER_CUBE              = 0x8B60,
    WEBGL_SAMPLER_2D_SHADOW         = 0x8B62,
    WEBGL_SAMPLER_2D_ARRAY          = 0x8DC1,
};
