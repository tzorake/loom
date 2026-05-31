#include "tzrhiwebgl.hpp"
#include "tzwebglimports.hpp"

#include <loom/tzrhiswapchain.hpp>
#include <loom/tzrhirendertarget.hpp>
#include <loom/tzrhirenderpassdescriptor.hpp>
#include <loom/tzrhishaderresourcebindings.hpp>
#include <loom/tzrhigraphicspipeline.hpp>
#include <loom/tzrhicommandbuffer.hpp>
#include <loom/tzrhiresourceupdatebatch.hpp>
#include <loom/tzrhibuffer.hpp>
#include <loom/tzrhitexture.hpp>
#include <loom/tzrhisampler.hpp>
#include <loom/tzrhirenderbuffer.hpp>

#include <algorithm>
#include <cstring>
#include <vector>
#include <string>
#include <cstdint>

// ─────────────────────────────────────────────────────────────────────────────
// Format / enum helpers (WebGL constants → RHI enums)
// ─────────────────────────────────────────────────────────────────────────────

struct WglTextureFormat { int internalFormat; int baseFormat; int type; };

static WglTextureFormat wglFormatOf(TzRhiTexture::Format fmt)
{
    switch (fmt) {
    case TzRhiTexture::RGBA8:      return {WEBGL_RGBA8,             WEBGL_RGBA,           WEBGL_UNSIGNED_BYTE};
    case TzRhiTexture::BGRA8:      return {WEBGL_RGBA8,             WEBGL_RGBA,           WEBGL_UNSIGNED_BYTE}; // no BGRA in ES
    case TzRhiTexture::R8:         return {WEBGL_R8,                WEBGL_RED,            WEBGL_UNSIGNED_BYTE};
    case TzRhiTexture::RG8:        return {WEBGL_RG8,               WEBGL_RG,             WEBGL_UNSIGNED_BYTE};
    case TzRhiTexture::R16F:       return {WEBGL_R16F,              WEBGL_RED,            WEBGL_HALF_FLOAT};
    case TzRhiTexture::RGBA16F:    return {WEBGL_RGBA16F,           WEBGL_RGBA,           WEBGL_HALF_FLOAT};
    case TzRhiTexture::RGBA32F:    return {WEBGL_RGBA32F,           WEBGL_RGBA,           WEBGL_FLOAT};
    case TzRhiTexture::D16:        return {WEBGL_DEPTH_COMPONENT16, WEBGL_DEPTH_COMPONENT,WEBGL_UNSIGNED_SHORT};
    case TzRhiTexture::D24S8:      return {WEBGL_DEPTH24_STENCIL8,  WEBGL_DEPTH_STENCIL,  WEBGL_UNSIGNED_INT_24_8};
    case TzRhiTexture::D32F:       return {WEBGL_DEPTH_COMPONENT32F,WEBGL_DEPTH_COMPONENT,WEBGL_FLOAT};
    case TzRhiTexture::RGBA8_sRGB: return {0x8C43 /*SRGB8_ALPHA8*/, WEBGL_RGBA,           WEBGL_UNSIGNED_BYTE};
    default:                       return {WEBGL_RGBA8,             WEBGL_RGBA,           WEBGL_UNSIGNED_BYTE};
    }
}

static int wglCompareOp(TzRhiGraphicsPipeline::CompareOp op)
{
    switch (op) {
    case TzRhiGraphicsPipeline::Never:          return WEBGL_NEVER;
    case TzRhiGraphicsPipeline::Less:           return WEBGL_LESS;
    case TzRhiGraphicsPipeline::Equal:          return WEBGL_EQUAL;
    case TzRhiGraphicsPipeline::LessOrEqual:    return WEBGL_LEQUAL;
    case TzRhiGraphicsPipeline::Greater:        return WEBGL_GREATER;
    case TzRhiGraphicsPipeline::NotEqual:       return WEBGL_NOTEQUAL;
    case TzRhiGraphicsPipeline::GreaterOrEqual: return WEBGL_GEQUAL;
    case TzRhiGraphicsPipeline::Always:         return WEBGL_ALWAYS;
    }
    return WEBGL_LESS;
}

static int wglBlendFactor(TzRhiGraphicsPipeline::BlendFactor f)
{
    switch (f) {
    case TzRhiGraphicsPipeline::Zero:                  return WEBGL_ZERO;
    case TzRhiGraphicsPipeline::One:                   return WEBGL_ONE;
    case TzRhiGraphicsPipeline::SrcColor:              return WEBGL_SRC_COLOR;
    case TzRhiGraphicsPipeline::OneMinusSrcColor:      return WEBGL_ONE_MINUS_SRC_COLOR;
    case TzRhiGraphicsPipeline::DstColor:              return WEBGL_DST_COLOR;
    case TzRhiGraphicsPipeline::OneMinusDstColor:      return WEBGL_ONE_MINUS_DST_COLOR;
    case TzRhiGraphicsPipeline::SrcAlpha:              return WEBGL_SRC_ALPHA;
    case TzRhiGraphicsPipeline::OneMinusSrcAlpha:      return WEBGL_ONE_MINUS_SRC_ALPHA;
    case TzRhiGraphicsPipeline::DstAlpha:              return WEBGL_DST_ALPHA;
    case TzRhiGraphicsPipeline::OneMinusDstAlpha:      return WEBGL_ONE_MINUS_DST_ALPHA;
    case TzRhiGraphicsPipeline::ConstantColor:         return WEBGL_CONSTANT_COLOR;
    case TzRhiGraphicsPipeline::OneMinusConstantColor: return WEBGL_ONE_MINUS_CONSTANT_COLOR;
    case TzRhiGraphicsPipeline::SrcAlphaSaturate:      return WEBGL_SRC_ALPHA_SATURATE;
    }
    return WEBGL_ONE;
}

static int wglBlendOp(TzRhiGraphicsPipeline::BlendOp op)
{
    switch (op) {
    case TzRhiGraphicsPipeline::Add:             return WEBGL_FUNC_ADD;
    case TzRhiGraphicsPipeline::Subtract:        return WEBGL_FUNC_SUBTRACT;
    case TzRhiGraphicsPipeline::ReverseSubtract: return WEBGL_FUNC_REVERSE_SUBTRACT;
    case TzRhiGraphicsPipeline::BlendMin:        return WEBGL_MIN;
    case TzRhiGraphicsPipeline::BlendMax:        return WEBGL_MAX;
    }
    return WEBGL_FUNC_ADD;
}

static int wglPrimitive(TzRhiGraphicsPipeline::Topology t)
{
    switch (t) {
    case TzRhiGraphicsPipeline::Triangles:     return WEBGL_TRIANGLES;
    case TzRhiGraphicsPipeline::TriangleStrip: return WEBGL_TRIANGLE_STRIP;
    case TzRhiGraphicsPipeline::Lines:         return WEBGL_LINES;
    case TzRhiGraphicsPipeline::Points:        return WEBGL_POINTS;
    }
    return WEBGL_TRIANGLES;
}

static int wglAttribType(TzRhiVertexInputAttribute::Format f)
{
    switch (f) {
    case TzRhiVertexInputAttribute::Float4:
    case TzRhiVertexInputAttribute::Float3:
    case TzRhiVertexInputAttribute::Float2:
    case TzRhiVertexInputAttribute::Float1: return WEBGL_FLOAT;
    case TzRhiVertexInputAttribute::UNorm4:
    case TzRhiVertexInputAttribute::UNorm2: return WEBGL_UNSIGNED_BYTE;
    case TzRhiVertexInputAttribute::UInt4:
    case TzRhiVertexInputAttribute::UInt3:
    case TzRhiVertexInputAttribute::UInt2:
    case TzRhiVertexInputAttribute::UInt1: return WEBGL_UNSIGNED_INT;
    default: return WEBGL_FLOAT;
    }
}

static int wglAttribComponents(TzRhiVertexInputAttribute::Format f)
{
    switch (f) {
    case TzRhiVertexInputAttribute::Float4:
    case TzRhiVertexInputAttribute::UNorm4:
    case TzRhiVertexInputAttribute::UInt4:
    case TzRhiVertexInputAttribute::SInt4: return 4;
    case TzRhiVertexInputAttribute::Float3:
    case TzRhiVertexInputAttribute::UInt3:
    case TzRhiVertexInputAttribute::SInt3: return 3;
    case TzRhiVertexInputAttribute::Float2:
    case TzRhiVertexInputAttribute::UNorm2:
    case TzRhiVertexInputAttribute::UInt2:
    case TzRhiVertexInputAttribute::SInt2: return 2;
    default:                               return 1;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Concrete WebGL resource types
// ─────────────────────────────────────────────────────────────────────────────

// ── Buffer ────────────────────────────────────────────────────────────────────

class TzRhiWebGLBuffer : public TzRhiBuffer
{
public:
    TzRhiWebGLBuffer(TzRhi *rhi, Type t, UsageFlags u, uint32_t sz)
        : TzRhiBuffer(rhi, t, u, sz) {}
    ~TzRhiWebGLBuffer() override { destroy(); }

    bool create() override
    {
        m_handle = gl_create_buffer();
        int target = (m_usage & UniformBuffer) ? WEBGL_UNIFORM_BUFFER
                   : (m_usage & IndexBuffer)   ? WEBGL_ELEMENT_ARRAY_BUFFER
                                               : WEBGL_ARRAY_BUFFER;
        gl_bind_buffer(target, m_handle);
        // Allocate with null data to reserve storage
        gl_buffer_data_static(target, nullptr, static_cast<int>(m_size));
        gl_bind_buffer(target, 0);
        return m_handle > 0;
    }

    void destroy() override
    {
        if (m_handle > 0) { gl_delete_buffer(m_handle); m_handle = 0; }
    }

    int m_handle{0};
};

// ── Texture ───────────────────────────────────────────────────────────────────

class TzRhiWebGLTexture : public TzRhiTexture
{
public:
    TzRhiWebGLTexture(TzRhi *rhi, Format fmt, int w, int h, Flags f)
        : TzRhiTexture(rhi, fmt, w, h, f) {}
    ~TzRhiWebGLTexture() override { destroy(); }

    bool create() override
    {
        auto wfmt = wglFormatOf(m_format);
        m_handle = gl_create_texture();
        gl_bind_texture(WEBGL_TEXTURE_2D, m_handle);
        gl_tex_image_2d(WEBGL_TEXTURE_2D, 0, wfmt.internalFormat,
                        m_width, m_height, 0,
                        wfmt.baseFormat, wfmt.type, nullptr, 0);
        gl_tex_parameteri(WEBGL_TEXTURE_2D, WEBGL_TEXTURE_MIN_FILTER, WEBGL_LINEAR);
        gl_tex_parameteri(WEBGL_TEXTURE_2D, WEBGL_TEXTURE_MAG_FILTER, WEBGL_LINEAR);
        gl_bind_texture(WEBGL_TEXTURE_2D, 0);
        return m_handle > 0;
    }

    void destroy() override
    {
        if (m_handle > 0) { gl_delete_texture(m_handle); m_handle = 0; }
    }

    int m_handle{0};
};

// ── Sampler ───────────────────────────────────────────────────────────────────

class TzRhiWebGLSampler : public TzRhiSampler
{
public:
    TzRhiWebGLSampler(TzRhi *rhi, Filter min, Filter mag, Filter mip,
                      AddressMode u, AddressMode v)
        : TzRhiSampler(rhi, min, mag, mip, u, v) {}
    ~TzRhiWebGLSampler() override { destroy(); }

    static int toFilter(Filter f, Filter mip)
    {
        if (f == Linear && mip == Linear) return WEBGL_LINEAR_MIPMAP_LINEAR;
        if (f == Linear)                  return WEBGL_LINEAR;
        return WEBGL_NEAREST;
    }
    static int toAddr(AddressMode a)
    {
        switch (a) {
        case Repeat:         return WEBGL_REPEAT;
        case MirroredRepeat: return WEBGL_MIRRORED_REPEAT;
        case ClampToEdge:    return WEBGL_CLAMP_TO_EDGE;
        }
        return WEBGL_REPEAT;
    }

    bool create() override
    {
        m_handle = gl_create_sampler();
        gl_sampler_parameteri(m_handle, WEBGL_TEXTURE_MIN_FILTER, toFilter(m_min, m_mip));
        gl_sampler_parameteri(m_handle, WEBGL_TEXTURE_MAG_FILTER,
                              m_mag == Linear ? WEBGL_LINEAR : WEBGL_NEAREST);
        gl_sampler_parameteri(m_handle, WEBGL_TEXTURE_WRAP_S, toAddr(m_addressU));
        gl_sampler_parameteri(m_handle, WEBGL_TEXTURE_WRAP_T, toAddr(m_addressV));
        return m_handle > 0;
    }

    void destroy() override
    {
        if (m_handle > 0) { gl_delete_sampler(m_handle); m_handle = 0; }
    }

    int m_handle{0};
};

// ── RenderBuffer ──────────────────────────────────────────────────────────────

class TzRhiWebGLRenderBuffer : public TzRhiRenderBuffer
{
public:
    TzRhiWebGLRenderBuffer(TzRhi *rhi, Type t, int w, int h, int s)
        : TzRhiRenderBuffer(rhi, t, w, h, s) {}
    ~TzRhiWebGLRenderBuffer() override {}
    bool create() override { return true; }
    void destroy() override {}
};

// ── RenderPassDescriptor ──────────────────────────────────────────────────────

class TzRhiWebGLRenderPassDescriptor : public TzRhiRenderPassDescriptor
{
public:
    explicit TzRhiWebGLRenderPassDescriptor(TzRhi *rhi)
        : TzRhiRenderPassDescriptor(rhi) {}
    ~TzRhiWebGLRenderPassDescriptor() override {}
    bool isCompatible(const TzRhiRenderPassDescriptor *) const override { return true; }
    void destroy() override {}
};

// ── SwapChain — backed by the canvas (default FBO 0) ─────────────────────────

class TzRhiWebGLSwapChain : public TzRhiSwapChain
{
public:
    explicit TzRhiWebGLSwapChain(TzRhi *rhi) : TzRhiSwapChain(rhi) {}
    ~TzRhiWebGLSwapChain() override {}

    bool createOrResize() override
    {
        // Canvas dimensions are driven by JS; read them back via the web
        // window imports so we can report correct pixelWidth/pixelHeight.
        // For now we rely on the caller to set m_pixelWidth/m_pixelHeight
        // via the resize event or direct call.
        return true;
    }

    void setPixelSize(int w, int h) { m_pixelWidth = w; m_pixelHeight = h; }
    void destroy() override {}
};

// ── ShaderResourceBindings ────────────────────────────────────────────────────

class TzRhiWebGLShaderResourceBindings : public TzRhiShaderResourceBindings
{
public:
    explicit TzRhiWebGLShaderResourceBindings(TzRhi *rhi)
        : TzRhiShaderResourceBindings(rhi) {}
    ~TzRhiWebGLShaderResourceBindings() override {}
    bool create() override { return true; }
    void destroy() override {}
};

// ── GraphicsPipeline ─────────────────────────────────────────────────────────

class TzRhiWebGLGraphicsPipeline : public TzRhiGraphicsPipeline
{
public:
    explicit TzRhiWebGLGraphicsPipeline(TzRhi *rhi)
        : TzRhiGraphicsPipeline(rhi) {}
    ~TzRhiWebGLGraphicsPipeline() override { destroy(); }

    bool create() override
    {
        // Compile shaders
        int vert = 0, frag = 0;
        for (const auto &stage : m_stages) {
            int sh = gl_create_shader(stage.type() == TzRhiShaderStage::Vertex
                                      ? WEBGL_VERTEX_SHADER : WEBGL_FRAGMENT_SHADER);
            const std::string &src = stage.shader().source();
            gl_shader_source(sh, src.c_str(), static_cast<int>(src.size()));
            gl_compile_shader(sh);
            if (!gl_get_shader_compile_status(sh)) {
                gl_delete_shader(sh);
                return false;
            }
            if (stage.type() == TzRhiShaderStage::Vertex) vert = sh;
            else                                           frag = sh;
        }
        m_program = gl_create_program();
        if (vert) gl_attach_shader(m_program, vert);
        if (frag) gl_attach_shader(m_program, frag);
        gl_link_program(m_program);
        if (vert) gl_delete_shader(vert);
        if (frag) gl_delete_shader(frag);
        if (!gl_get_program_link_status(m_program)) {
            gl_delete_program(m_program);
            m_program = 0;
            return false;
        }

        // Assign UBO binding points: uniform block i → binding point i.
        {
            int numBlocks = gl_get_active_uniform_block_count(m_program);
            for (int i = 0; i < numBlocks; ++i)
                gl_uniform_block_binding(m_program, i, i);
        }

        // Point sampler uniforms at the texture units used by the SRB.
        if (m_srb) {
            std::vector<int> samplerUnits;
            for (const auto &b : m_srb->bindings())
                if (b.type() == TzRhiShaderResourceBinding::SampledTexture)
                    samplerUnits.push_back(b.binding());
            std::sort(samplerUnits.begin(), samplerUnits.end());

            if (!samplerUnits.empty()) {
                gl_use_program(m_program);
                int numUniforms = gl_get_active_uniform_count(m_program);
                int si = 0;
                for (int i = 0; i < numUniforms && si < (int)samplerUnits.size(); ++i) {
                    int type = gl_get_active_uniform_type(m_program, i);
                    if (type == WEBGL_SAMPLER_2D     || type == WEBGL_SAMPLER_3D  ||
                        type == WEBGL_SAMPLER_CUBE   || type == WEBGL_SAMPLER_2D_SHADOW ||
                        type == WEBGL_SAMPLER_2D_ARRAY) {
                        char name[128] = {};
                        gl_get_active_uniform_name(m_program, i, name, sizeof(name));
                        int loc = gl_get_uniform_location(m_program, name,
                                                          static_cast<int>(std::strlen(name)));
                        if (loc >= 0)
                            gl_uniform1i(loc, samplerUnits[si]);
                        ++si;
                    }
                }
            }
        }

        // Build VAO — only enable the attribute arrays here.
        // gl_vertex_attrib_pointer is deferred to setVertexInput() where the
        // ARRAY_BUFFER is bound; WebGL requires a bound buffer when offset != 0.
        m_vao = gl_create_vertex_array();
        gl_bind_vertex_array(m_vao);
        for (const auto &attr : m_vertexLayout.attributes())
            gl_enable_vertex_attrib_array(attr.location());
        gl_bind_vertex_array(0);

        return true;
    }

    void destroy() override
    {
        if (m_vao)     { gl_delete_vertex_array(m_vao); m_vao = 0; }
        if (m_program) { gl_delete_program(m_program);  m_program = 0; }
    }

    int m_program{0};
    int m_vao{0};
};

// ─────────────────────────────────────────────────────────────────────────────
// ResourceUpdateBatch
// ─────────────────────────────────────────────────────────────────────────────

struct WglUploadOp
{
    enum Kind { StaticBuffer, DynamicBuffer, Texture };
    Kind kind;
    int    handle{0};
    int    bufTarget{0};
    int    bufOffset{0};
    // texture fields
    int    texInternal{0}, texBase{0}, texType{0};
    int    texX{0}, texY{0}, texW{0}, texH{0}, texLevel{0};
    std::vector<uint8_t> data;
};

class TzRhiWebGLResourceUpdateBatch : public TzRhiResourceUpdateBatch
{
public:
    void uploadStaticBuffer(TzRhiBuffer *buf, const void *data, uint32_t byteSize) override
    {
        auto *wb = static_cast<TzRhiWebGLBuffer *>(buf);
        WglUploadOp op;
        op.kind      = WglUploadOp::StaticBuffer;
        op.handle    = wb->m_handle;
        op.bufTarget = (buf->usage() & TzRhiBuffer::UniformBuffer) ? WEBGL_UNIFORM_BUFFER
                     : (buf->usage() & TzRhiBuffer::IndexBuffer)   ? WEBGL_ELEMENT_ARRAY_BUFFER
                                                                    : WEBGL_ARRAY_BUFFER;
        op.data.assign(static_cast<const uint8_t*>(data),
                       static_cast<const uint8_t*>(data) + byteSize);
        m_ops.push_back(std::move(op));
    }

    void updateDynamicBuffer(TzRhiBuffer *buf, uint32_t offset,
                             uint32_t size, const void *data) override
    {
        auto *wb = static_cast<TzRhiWebGLBuffer *>(buf);
        WglUploadOp op;
        op.kind      = WglUploadOp::DynamicBuffer;
        op.handle    = wb->m_handle;
        op.bufTarget = (buf->usage() & TzRhiBuffer::UniformBuffer) ? WEBGL_UNIFORM_BUFFER
                     : (buf->usage() & TzRhiBuffer::IndexBuffer)   ? WEBGL_ELEMENT_ARRAY_BUFFER
                                                                    : WEBGL_ARRAY_BUFFER;
        op.bufOffset = static_cast<int>(offset);
        op.data.assign(static_cast<const uint8_t*>(data),
                       static_cast<const uint8_t*>(data) + size);
        m_ops.push_back(std::move(op));
    }

    void uploadTexture(TzRhiTexture *tex,
                       const TzRhiTextureUploadDescription &desc) override
    {
        auto *wt  = static_cast<TzRhiWebGLTexture *>(tex);
        auto wfmt = wglFormatOf(tex->format());
        int  w    = (desc.width  < 0) ? tex->width()  : desc.width;
        int  h    = (desc.height < 0) ? tex->height() : desc.height;
        WglUploadOp op;
        op.kind       = WglUploadOp::Texture;
        op.handle     = wt->m_handle;
        op.texInternal= wfmt.internalFormat;
        op.texBase    = wfmt.baseFormat;
        op.texType    = wfmt.type;
        op.texX       = desc.x;
        op.texY       = desc.y;
        op.texW       = w;
        op.texH       = h;
        op.texLevel   = desc.level;
        op.data.assign(static_cast<const uint8_t*>(desc.data),
                       static_cast<const uint8_t*>(desc.data) + desc.byteSize);
        m_ops.push_back(std::move(op));
    }

    void flush()
    {
        for (const auto &op : m_ops) {
            switch (op.kind) {
            case WglUploadOp::StaticBuffer:
                gl_bind_buffer(op.bufTarget, op.handle);
                gl_buffer_data_static(op.bufTarget, op.data.data(),
                                      static_cast<int>(op.data.size()));
                gl_bind_buffer(op.bufTarget, 0);
                break;
            case WglUploadOp::DynamicBuffer:
                gl_bind_buffer(op.bufTarget, op.handle);
                gl_buffer_sub_data(op.bufTarget, op.bufOffset,
                                   op.data.data(), static_cast<int>(op.data.size()));
                gl_bind_buffer(op.bufTarget, 0);
                break;
            case WglUploadOp::Texture:
                gl_bind_texture(WEBGL_TEXTURE_2D, op.handle);
                gl_tex_sub_image_2d(WEBGL_TEXTURE_2D, op.texLevel,
                                    op.texX, op.texY, op.texW, op.texH,
                                    op.texBase, op.texType,
                                    op.data.data(), static_cast<int>(op.data.size()));
                gl_bind_texture(WEBGL_TEXTURE_2D, 0);
                break;
            }
        }
        m_ops.clear();
    }

private:
    std::vector<WglUploadOp> m_ops;
};

// ─────────────────────────────────────────────────────────────────────────────
// CommandBuffer
// ─────────────────────────────────────────────────────────────────────────────

class TzRhiWebGLCommandBuffer : public TzRhiCommandBuffer
{
public:
    void beginPass(TzRhiRenderTarget *rt,
                   TzRhiRenderPassDescriptor *,
                   const ClearValue &cv,
                   TzRhiResourceUpdateBatch *uploads) override
    {
        if (uploads)
            static_cast<TzRhiWebGLResourceUpdateBatch *>(uploads)->flush();

        gl_bind_framebuffer(WEBGL_FRAMEBUFFER, 0);

        if (rt)
            gl_viewport(0, 0, rt->pixelWidth(), rt->pixelHeight());

        gl_clear_color(cv.r, cv.g, cv.b, cv.a);
        gl_clear(WEBGL_COLOR_BUFFER_BIT | WEBGL_DEPTH_BUFFER_BIT | WEBGL_STENCIL_BUFFER_BIT);
    }

    void endPass(TzRhiResourceUpdateBatch *uploads) override
    {
        if (uploads)
            static_cast<TzRhiWebGLResourceUpdateBatch *>(uploads)->flush();
    }

    void setGraphicsPipeline(TzRhiGraphicsPipeline *ps) override
    {
        m_currentPipeline = static_cast<TzRhiWebGLGraphicsPipeline *>(ps);
        gl_use_program(m_currentPipeline->m_program);
        gl_bind_vertex_array(m_currentPipeline->m_vao);

        if (ps->depthTest()) {
            gl_enable(WEBGL_DEPTH_TEST);
            gl_depth_func(wglCompareOp(ps->depthOp()));
        } else {
            gl_disable(WEBGL_DEPTH_TEST);
        }
        gl_depth_mask(ps->depthWrite() ? 1 : 0);

        if (ps->cullMode() != TzRhiGraphicsPipeline::NoCulling) {
            gl_enable(WEBGL_CULL_FACE);
            gl_cull_face(ps->cullMode() == TzRhiGraphicsPipeline::Front
                         ? WEBGL_FRONT : WEBGL_BACK);
        } else {
            gl_disable(WEBGL_CULL_FACE);
        }
        gl_front_face(ps->frontFace() == TzRhiGraphicsPipeline::CW ? WEBGL_CW : WEBGL_CCW);
        gl_line_width(ps->lineWidth());

        const auto &blends = ps->targetBlends();
        if (!blends.empty() && blends[0].enable) {
            gl_enable(WEBGL_BLEND);
            gl_blend_func_separate(
                wglBlendFactor(blends[0].srcColor), wglBlendFactor(blends[0].dstColor),
                wglBlendFactor(blends[0].srcAlpha), wglBlendFactor(blends[0].dstAlpha));
            gl_blend_equation_separate(wglBlendOp(blends[0].opColor),
                                       wglBlendOp(blends[0].opAlpha));
        } else {
            gl_disable(WEBGL_BLEND);
        }
    }

    void setShaderResources(TzRhiShaderResourceBindings *srb) override
    {
        if (!srb && m_currentPipeline)
            srb = m_currentPipeline->shaderResourceBindings();
        if (!srb)
            return;

        for (const auto &b : srb->bindings()) {
            if (b.type() == TzRhiShaderResourceBinding::UniformBuffer && b.buffer()) {
                auto *wb = static_cast<TzRhiWebGLBuffer *>(b.buffer());
                int sz = (b.ubSize() > 0) ? static_cast<int>(b.ubSize())
                                          : static_cast<int>(b.buffer()->size());
                gl_bind_buffer_range(WEBGL_UNIFORM_BUFFER, b.binding(),
                                     wb->m_handle,
                                     static_cast<int>(b.ubOffset()), sz);
            } else if (b.type() == TzRhiShaderResourceBinding::SampledTexture) {
                auto *wt = static_cast<TzRhiWebGLTexture *>(b.texture());
                auto *ws = static_cast<TzRhiWebGLSampler *>(b.sampler());
                gl_active_texture(WEBGL_TEXTURE0 + b.binding());
                gl_bind_texture(WEBGL_TEXTURE_2D, wt ? wt->m_handle : 0);
                gl_bind_sampler(b.binding(), ws ? ws->m_handle : 0);
            }
        }
    }

    void setViewport(float x, float y, float w, float h,
                     float /*minDepth*/, float /*maxDepth*/) override
    {
        gl_viewport(static_cast<int>(x), static_cast<int>(y),
                    static_cast<int>(w), static_cast<int>(h));
    }

    void setScissor(int x, int y, int w, int h) override
    {
        gl_enable(WEBGL_SCISSOR_TEST);
        gl_scissor(x, y, w, h);
    }

    void setVertexInput(int startBinding, int count,
                        const TzRhiBuffer *const *bufs,
                        const uint32_t *offsets,
                        TzRhiBuffer *indexBuf, uint32_t indexOffset,
                        IndexFormat indexFormat) override
    {
        for (int i = 0; i < count; ++i) {
            const auto *wb = static_cast<const TzRhiWebGLBuffer *>(bufs[i]);
            gl_bind_buffer(WEBGL_ARRAY_BUFFER, wb->m_handle);
            if (m_currentPipeline) {
                int off = offsets ? static_cast<int>(offsets[i]) : 0;
                for (const auto &attr : m_currentPipeline->vertexInputLayout().attributes()) {
                    if (attr.binding() != startBinding + i)
                        continue;
                    const auto &binds = m_currentPipeline->vertexInputLayout().bindings();
                    int stride = (attr.binding() < (int)binds.size())
                               ? static_cast<int>(binds[attr.binding()].stride()) : 0;
                    int type = wglAttribType(attr.format());
                    int comp = wglAttribComponents(attr.format());
                    int norm = (attr.format() == TzRhiVertexInputAttribute::UNorm4 ||
                                attr.format() == TzRhiVertexInputAttribute::UNorm2) ? 1 : 0;
                    gl_vertex_attrib_pointer(attr.location(), comp, type, norm,
                                             stride,
                                             static_cast<int>(attr.offset()) + off);
                }
            }
        }
        if (indexBuf) {
            auto *ib = static_cast<TzRhiWebGLBuffer *>(indexBuf);
            gl_bind_buffer(WEBGL_ELEMENT_ARRAY_BUFFER, ib->m_handle);
            m_indexOffset = indexOffset;
            m_indexFormat = indexFormat;
        }
        m_primitive = m_currentPipeline ? wglPrimitive(m_currentPipeline->topology())
                                        : WEBGL_TRIANGLES;
    }

    void draw(uint32_t vertexCount, uint32_t /*instanceCount*/,
              uint32_t firstVertex, uint32_t /*firstInstance*/) override
    {
        gl_draw_arrays(m_primitive, static_cast<int>(firstVertex),
                       static_cast<int>(vertexCount));
    }

    void drawIndexed(uint32_t indexCount, uint32_t /*instanceCount*/,
                     uint32_t firstIndex, int32_t /*vertexOffset*/,
                     uint32_t /*firstInstance*/) override
    {
        int type   = (m_indexFormat == UInt16) ? WEBGL_UNSIGNED_SHORT : WEBGL_UNSIGNED_INT;
        int byteOff = static_cast<int>(m_indexOffset) +
                      static_cast<int>(firstIndex) *
                      ((m_indexFormat == UInt16) ? 2 : 4);
        gl_draw_elements(m_primitive, static_cast<int>(indexCount), type, byteOff);
    }

    void debugMarkBegin(const char *) override {}
    void debugMarkEnd() override {}

private:
    TzRhiWebGLGraphicsPipeline *m_currentPipeline{nullptr};
    int      m_primitive{WEBGL_TRIANGLES};
    uint32_t m_indexOffset{0};
    IndexFormat m_indexFormat{UInt16};
};

// ─────────────────────────────────────────────────────────────────────────────
// TzRhiWebGL
// ─────────────────────────────────────────────────────────────────────────────

TzRhiWebGL::TzRhiWebGL()
    : TzRhi(Backend::WebGL)
    , m_commandBuffer(std::make_unique<TzRhiWebGLCommandBuffer>())
    , m_updateBatch(std::make_unique<TzRhiWebGLResourceUpdateBatch>())
{}

TzRhiWebGL::~TzRhiWebGL() = default;

TzRhiBuffer *TzRhiWebGL::newBuffer(TzRhiBuffer::Type t,
                                   TzRhiBuffer::UsageFlags u, uint32_t sz)
{
    return new TzRhiWebGLBuffer(this, t, u, sz);
}

TzRhiTexture *TzRhiWebGL::newTexture(TzRhiTexture::Format fmt, int w, int h,
                                     TzRhiTexture::Flags f)
{
    return new TzRhiWebGLTexture(this, fmt, w, h, f);
}

TzRhiSampler *TzRhiWebGL::newSampler(TzRhiSampler::Filter min,
                                     TzRhiSampler::Filter mag,
                                     TzRhiSampler::Filter mip,
                                     TzRhiSampler::AddressMode u,
                                     TzRhiSampler::AddressMode v)
{
    return new TzRhiWebGLSampler(this, min, mag, mip, u, v);
}

TzRhiRenderBuffer *TzRhiWebGL::newRenderBuffer(TzRhiRenderBuffer::Type t,
                                               int w, int h, int s)
{
    return new TzRhiWebGLRenderBuffer(this, t, w, h, s);
}

TzRhiRenderPassDescriptor *TzRhiWebGL::newCompatibleRenderPassDescriptor(TzRhiRenderTarget *)
{
    return new TzRhiWebGLRenderPassDescriptor(this);
}

TzRhiSwapChain *TzRhiWebGL::newSwapChain()
{
    return new TzRhiWebGLSwapChain(this);
}

TzRhiShaderResourceBindings *TzRhiWebGL::newShaderResourceBindings()
{
    return new TzRhiWebGLShaderResourceBindings(this);
}

TzRhiGraphicsPipeline *TzRhiWebGL::newGraphicsPipeline()
{
    return new TzRhiWebGLGraphicsPipeline(this);
}

TzRhiResourceUpdateBatch *TzRhiWebGL::nextResourceUpdateBatch()
{
    return m_updateBatch.get();
}

bool TzRhiWebGL::beginFrame(TzRhiSwapChain *)
{
    // WebGL context is always current in the browser's main thread.
    return true;
}

void TzRhiWebGL::endFrame(TzRhiSwapChain *)
{
    // The browser presents the canvas at the end of the RAF callback;
    // no explicit swap needed.
}

TzRhiCommandBuffer *TzRhiWebGL::currentFrameCommandBuffer()
{
    return m_commandBuffer.get();
}

TzRhiRenderTarget *TzRhiWebGL::currentFrameRenderTarget(TzRhiSwapChain *sc)
{
    return static_cast<TzRhiWebGLSwapChain *>(sc);
}
