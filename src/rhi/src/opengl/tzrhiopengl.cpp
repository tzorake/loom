#include "tzrhiopengl.hpp"
#include "tzrhiopengl_p.hpp"

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
#include <cassert>
#include <vector>
#include <string>

// ─────────────────────────────────────────────────────────────────────────────
// Windows: define and load GL 3.3 entry points
// ─────────────────────────────────────────────────────────────────────────────

#if defined(_WIN32)
PFNGLGENVERTEXARRAYSPROC       glGenVertexArrays        = nullptr;
PFNGLBINDVERTEXARRAYPROC       glBindVertexArray        = nullptr;
PFNGLDELETEVERTEXARRAYSPROC    glDeleteVertexArrays     = nullptr;
PFNGLGENBUFFERSPROC            glGenBuffers             = nullptr;
PFNGLBINDBUFFERPROC            glBindBuffer             = nullptr;
PFNGLBUFFERDATAPROC            glBufferData             = nullptr;
PFNGLBUFFERSUBDATAPROC         glBufferSubData          = nullptr;
PFNGLDELETEBUFFERSPROC         glDeleteBuffers          = nullptr;
PFNGLGENTEXTURESPROC           glGenTextures            = nullptr;
PFNGLBINDTEXTUREPROC           glBindTexture            = nullptr;
PFNGLTEXIMAGE2DPROC            glTexImage2D             = nullptr;
PFNGLTEXSUBIMAGE2DPROC         glTexSubImage2D          = nullptr;
PFNGLTEXPARAMETERIPROC         glTexParameteri          = nullptr;
PFNGLDELETETEXTURESPROC        glDeleteTextures         = nullptr;
PFNGLGENSAMPLERSPROC           glGenSamplers            = nullptr;
PFNGLBINDSAMPLERPROC           glBindSampler            = nullptr;
PFNGLSAMPLERPARAMETERIPROC     glSamplerParameteri      = nullptr;
PFNGLDELETESAMPLERSPROC        glDeleteSamplers         = nullptr;
PFNGLCREATESHADERPROC          glCreateShader           = nullptr;
PFNGLSHADERSOURCEPROC          glShaderSource           = nullptr;
PFNGLCOMPILESHADERPROC         glCompileShader          = nullptr;
PFNGLGETSHADERIVPROC           glGetShaderiv            = nullptr;
PFNGLGETSHADERINFOLOGPROC      glGetShaderInfoLog       = nullptr;
PFNGLDELETESHADERPROC          glDeleteShader           = nullptr;
PFNGLCREATEPROGRAMPROC         glCreateProgram          = nullptr;
PFNGLATTACHSHADERPROC          glAttachShader           = nullptr;
PFNGLLINKPROGRAMPROC           glLinkProgram            = nullptr;
PFNGLGETPROGRAMIVPROC          glGetProgramiv           = nullptr;
PFNGLGETPROGRAMINFOLOGPROC     glGetProgramInfoLog      = nullptr;
PFNGLUSEPROGRAMPROC            glUseProgram             = nullptr;
PFNGLDELETEPROGRAMPROC         glDeleteProgram          = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC   glVertexAttribPointer    = nullptr;
PFNGLDRAWARRAYSPROC            glDrawArrays             = nullptr;
PFNGLDRAWELEMENTSPROC          glDrawElements           = nullptr;
PFNGLDRAWELEMENTSPROC          glDrawElementsBaseVertex = nullptr;
PFNGLBINDBUFFERRANGEPROC       glBindBufferRange        = nullptr;
PFNGLGETUNIFORMBLOCKINDEXPROC  glGetUniformBlockIndex   = nullptr;
PFNGLUNIFORMBLOCKBINDINGPROC   glUniformBlockBinding    = nullptr;
PFNGLACTIVETEXTUREPROC         glActiveTexture          = nullptr;
PFNGLVIEWPORTPROC              glViewport               = nullptr;
PFNGLCLEARCOLORPROC            glClearColor             = nullptr;
PFNGLCLEARPROC                 glClear                  = nullptr;
PFNGLDEPTHFUNCPROC             glDepthFunc              = nullptr;
PFNGLDEPTHMASKPROC             glDepthMask              = nullptr;
PFNGLBLENDFUNCSEPARATEPROC     glBlendFuncSeparate      = nullptr;
PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparate  = nullptr;
PFNGLCULLFACEPROC              glCullFace               = nullptr;
PFNGLFRONTFACEPROC             glFrontFace              = nullptr;
PFNGLENABLEPROC                glEnable                 = nullptr;
PFNGLDISABLEPROC               glDisable                = nullptr;
PFNGLSCISSORPROC               glScissor                = nullptr;
PFNGLLINEWIDTHPROC             glLineWidth              = nullptr;
PFNGLFLUSHPROC                 glFlush                  = nullptr;
PFNGLBINDFRAMEBUFFERPROC       glBindFramebuffer        = nullptr;
PFNGLGENFRAMEBUFFERSPROC       glGenFramebuffers        = nullptr;
PFNGLDELETEFRAMEBUFFERSPROC    glDeleteFramebuffers     = nullptr;
PFNGLFRAMEBUFFERTEXTURE2DPROC  glFramebufferTexture2D   = nullptr;
PFNGLGETACTIVEUNIFORMPROC      glGetActiveUniform       = nullptr;
PFNGLGETUNIFORMLOCATIONPROC    glGetUniformLocation     = nullptr;
PFNGLUNIFORM1IPROC             glUniform1i              = nullptr;

#define TZ_LOAD(T, name) name = (T)wglGetProcAddress(#name)

void tzgl_load_procs()
{
    TZ_LOAD(PFNGLGENVERTEXARRAYSPROC,       glGenVertexArrays);
    TZ_LOAD(PFNGLBINDVERTEXARRAYPROC,       glBindVertexArray);
    TZ_LOAD(PFNGLDELETEVERTEXARRAYSPROC,    glDeleteVertexArrays);
    TZ_LOAD(PFNGLGENBUFFERSPROC,            glGenBuffers);
    TZ_LOAD(PFNGLBINDBUFFERPROC,            glBindBuffer);
    TZ_LOAD(PFNGLBUFFERDATAPROC,            glBufferData);
    TZ_LOAD(PFNGLBUFFERSUBDATAPROC,         glBufferSubData);
    TZ_LOAD(PFNGLDELETEBUFFERSPROC,         glDeleteBuffers);
    TZ_LOAD(PFNGLGENTEXTURESPROC,           glGenTextures);
    TZ_LOAD(PFNGLBINDTEXTUREPROC,           glBindTexture);
    TZ_LOAD(PFNGLTEXIMAGE2DPROC,            glTexImage2D);
    TZ_LOAD(PFNGLTEXSUBIMAGE2DPROC,         glTexSubImage2D);
    TZ_LOAD(PFNGLTEXPARAMETERIPROC,         glTexParameteri);
    TZ_LOAD(PFNGLDELETETEXTURESPROC,        glDeleteTextures);
    TZ_LOAD(PFNGLGENSAMPLERSPROC,           glGenSamplers);
    TZ_LOAD(PFNGLBINDSAMPLERPROC,           glBindSampler);
    TZ_LOAD(PFNGLSAMPLERPARAMETERIPROC,     glSamplerParameteri);
    TZ_LOAD(PFNGLDELETESAMPLERSPROC,        glDeleteSamplers);
    TZ_LOAD(PFNGLCREATESHADERPROC,          glCreateShader);
    TZ_LOAD(PFNGLSHADERSOURCEPROC,          glShaderSource);
    TZ_LOAD(PFNGLCOMPILESHADERPROC,         glCompileShader);
    TZ_LOAD(PFNGLGETSHADERIVPROC,           glGetShaderiv);
    TZ_LOAD(PFNGLGETSHADERINFOLOGPROC,      glGetShaderInfoLog);
    TZ_LOAD(PFNGLDELETESHADERPROC,          glDeleteShader);
    TZ_LOAD(PFNGLCREATEPROGRAMPROC,         glCreateProgram);
    TZ_LOAD(PFNGLATTACHSHADERPROC,          glAttachShader);
    TZ_LOAD(PFNGLLINKPROGRAMPROC,           glLinkProgram);
    TZ_LOAD(PFNGLGETPROGRAMIVPROC,          glGetProgramiv);
    TZ_LOAD(PFNGLGETPROGRAMINFOLOGPROC,     glGetProgramInfoLog);
    TZ_LOAD(PFNGLUSEPROGRAMPROC,            glUseProgram);
    TZ_LOAD(PFNGLDELETEPROGRAMPROC,         glDeleteProgram);
    TZ_LOAD(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray);
    TZ_LOAD(PFNGLVERTEXATTRIBPOINTERPROC,   glVertexAttribPointer);
    TZ_LOAD(PFNGLDRAWARRAYSPROC,            glDrawArrays);
    TZ_LOAD(PFNGLDRAWELEMENTSPROC,          glDrawElements);
    TZ_LOAD(PFNGLDRAWELEMENTSPROC,          glDrawElementsBaseVertex);
    TZ_LOAD(PFNGLBINDBUFFERRANGEPROC,       glBindBufferRange);
    TZ_LOAD(PFNGLGETUNIFORMBLOCKINDEXPROC,  glGetUniformBlockIndex);
    TZ_LOAD(PFNGLUNIFORMBLOCKBINDINGPROC,   glUniformBlockBinding);
    TZ_LOAD(PFNGLACTIVETEXTUREPROC,         glActiveTexture);
    TZ_LOAD(PFNGLVIEWPORTPROC,              glViewport);
    TZ_LOAD(PFNGLCLEARCOLORPROC,            glClearColor);
    TZ_LOAD(PFNGLCLEARPROC,                 glClear);
    TZ_LOAD(PFNGLDEPTHFUNCPROC,             glDepthFunc);
    TZ_LOAD(PFNGLDEPTHMASKPROC,             glDepthMask);
    TZ_LOAD(PFNGLBLENDFUNCSEPARATEPROC,     glBlendFuncSeparate);
    TZ_LOAD(PFNGLBLENDEQUATIONSEPARATEPROC, glBlendEquationSeparate);
    TZ_LOAD(PFNGLCULLFACEPROC,              glCullFace);
    TZ_LOAD(PFNGLFRONTFACEPROC,             glFrontFace);
    TZ_LOAD(PFNGLENABLEPROC,                glEnable);
    TZ_LOAD(PFNGLDISABLEPROC,               glDisable);
    TZ_LOAD(PFNGLSCISSORPROC,               glScissor);
    TZ_LOAD(PFNGLLINEWIDTHPROC,             glLineWidth);
    TZ_LOAD(PFNGLFLUSHPROC,                 glFlush);
    TZ_LOAD(PFNGLBINDFRAMEBUFFERPROC,       glBindFramebuffer);
    TZ_LOAD(PFNGLGENFRAMEBUFFERSPROC,       glGenFramebuffers);
    TZ_LOAD(PFNGLDELETEFRAMEBUFFERSPROC,    glDeleteFramebuffers);
    TZ_LOAD(PFNGLFRAMEBUFFERTEXTURE2DPROC,  glFramebufferTexture2D);
    TZ_LOAD(PFNGLGETACTIVEUNIFORMPROC,      glGetActiveUniform);
    TZ_LOAD(PFNGLGETUNIFORMLOCATIONPROC,    glGetUniformLocation);
    TZ_LOAD(PFNGLUNIFORM1IPROC,             glUniform1i);
}
#endif // _WIN32

// ─────────────────────────────────────────────────────────────────────────────
// GL format helpers
// ─────────────────────────────────────────────────────────────────────────────

struct GlTextureFormat { GLenum internalFormat; GLenum baseFormat; GLenum type; };

static GlTextureFormat glFormatOf(TzRhiTexture::Format fmt)
{
    switch (fmt) {
    case TzRhiTexture::RGBA8:      return {GL_RGBA8,              GL_RGBA,         GL_UNSIGNED_BYTE};
    case TzRhiTexture::BGRA8:      return {GL_RGBA8,              GL_BGRA,         GL_UNSIGNED_BYTE};
    case TzRhiTexture::R8:         return {GL_R8,                 GL_RED,          GL_UNSIGNED_BYTE};
    case TzRhiTexture::RG8:        return {GL_RG8,                GL_RG,           GL_UNSIGNED_BYTE};
    case TzRhiTexture::R16F:       return {GL_R16F,               GL_RED,          GL_HALF_FLOAT};
    case TzRhiTexture::RGBA16F:    return {GL_RGBA16F,            GL_RGBA,         GL_HALF_FLOAT};
    case TzRhiTexture::RGBA32F:    return {GL_RGBA32F,            GL_RGBA,         GL_FLOAT};
    case TzRhiTexture::D16:        return {GL_DEPTH_COMPONENT16,  GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT};
    case TzRhiTexture::D24S8:      return {GL_DEPTH24_STENCIL8,   GL_DEPTH_STENCIL,   GL_UNSIGNED_INT_24_8};
    case TzRhiTexture::D32F:       return {GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT};
    case TzRhiTexture::RGBA8_sRGB: return {0x8C43 /*GL_SRGB8_ALPHA8*/, GL_RGBA,   GL_UNSIGNED_BYTE};
    default:                       return {GL_RGBA8,              GL_RGBA,         GL_UNSIGNED_BYTE};
    }
}

static GLenum glCompareOp(TzRhiGraphicsPipeline::CompareOp op)
{
    switch (op) {
    case TzRhiGraphicsPipeline::Never:          return GL_NEVER;
    case TzRhiGraphicsPipeline::Less:           return GL_LESS;
    case TzRhiGraphicsPipeline::Equal:          return GL_EQUAL;
    case TzRhiGraphicsPipeline::LessOrEqual:    return GL_LEQUAL;
    case TzRhiGraphicsPipeline::Greater:        return GL_GREATER;
    case TzRhiGraphicsPipeline::NotEqual:       return GL_NOTEQUAL;
    case TzRhiGraphicsPipeline::GreaterOrEqual: return GL_GEQUAL;
    case TzRhiGraphicsPipeline::Always:         return GL_ALWAYS;
    }
    return GL_LESS;
}

static GLenum glBlendFactor(TzRhiGraphicsPipeline::BlendFactor f)
{
    switch (f) {
    case TzRhiGraphicsPipeline::Zero:                 return GL_ZERO;
    case TzRhiGraphicsPipeline::One:                  return GL_ONE;
    case TzRhiGraphicsPipeline::SrcColor:             return GL_SRC_COLOR;
    case TzRhiGraphicsPipeline::OneMinusSrcColor:     return GL_ONE_MINUS_SRC_COLOR;
    case TzRhiGraphicsPipeline::DstColor:             return GL_DST_COLOR;
    case TzRhiGraphicsPipeline::OneMinusDstColor:     return GL_ONE_MINUS_DST_COLOR;
    case TzRhiGraphicsPipeline::SrcAlpha:             return GL_SRC_ALPHA;
    case TzRhiGraphicsPipeline::OneMinusSrcAlpha:     return GL_ONE_MINUS_SRC_ALPHA;
    case TzRhiGraphicsPipeline::DstAlpha:             return GL_DST_ALPHA;
    case TzRhiGraphicsPipeline::OneMinusDstAlpha:     return GL_ONE_MINUS_DST_ALPHA;
    case TzRhiGraphicsPipeline::ConstantColor:        return GL_CONSTANT_COLOR;
    case TzRhiGraphicsPipeline::OneMinusConstantColor:return GL_ONE_MINUS_CONSTANT_COLOR;
    case TzRhiGraphicsPipeline::SrcAlphaSaturate:     return GL_SRC_ALPHA_SATURATE;
    }
    return GL_ONE;
}

static GLenum glBlendOp(TzRhiGraphicsPipeline::BlendOp op)
{
    switch (op) {
    case TzRhiGraphicsPipeline::Add:             return GL_FUNC_ADD;
    case TzRhiGraphicsPipeline::Subtract:        return GL_FUNC_SUBTRACT;
    case TzRhiGraphicsPipeline::ReverseSubtract: return GL_FUNC_REVERSE_SUBTRACT;
    case TzRhiGraphicsPipeline::BlendMin:        return GL_MIN;
    case TzRhiGraphicsPipeline::BlendMax:        return GL_MAX;
    }
    return GL_FUNC_ADD;
}

static GLenum glPrimitive(TzRhiGraphicsPipeline::Topology t)
{
    switch (t) {
    case TzRhiGraphicsPipeline::Triangles:     return GL_TRIANGLES;
    case TzRhiGraphicsPipeline::TriangleStrip: return GL_TRIANGLE_STRIP;
    case TzRhiGraphicsPipeline::Lines:         return GL_LINES;
    case TzRhiGraphicsPipeline::Points:        return GL_POINTS;
    }
    return GL_TRIANGLES;
}

static GLenum glAttribType(TzRhiVertexInputAttribute::Format f, GLboolean &normalised)
{
    normalised = GL_FALSE;
    switch (f) {
    case TzRhiVertexInputAttribute::Float4:
    case TzRhiVertexInputAttribute::Float3:
    case TzRhiVertexInputAttribute::Float2:
    case TzRhiVertexInputAttribute::Float1: return GL_FLOAT;
    case TzRhiVertexInputAttribute::UNorm4:
    case TzRhiVertexInputAttribute::UNorm2: normalised = GL_TRUE; return GL_UNSIGNED_BYTE;
    case TzRhiVertexInputAttribute::UInt4:
    case TzRhiVertexInputAttribute::UInt3:
    case TzRhiVertexInputAttribute::UInt2:
    case TzRhiVertexInputAttribute::UInt1: return GL_UNSIGNED_INT;
    case TzRhiVertexInputAttribute::SInt4:
    case TzRhiVertexInputAttribute::SInt3:
    case TzRhiVertexInputAttribute::SInt2:
    case TzRhiVertexInputAttribute::SInt1: return GL_INT;
    }
    return GL_FLOAT;
}

static GLint glAttribComponents(TzRhiVertexInputAttribute::Format f)
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
// Concrete resource types
// ─────────────────────────────────────────────────────────────────────────────

// ── Buffer ────────────────────────────────────────────────────────────────────

class TzRhiOpenGLBuffer : public TzRhiBuffer
{
public:
    TzRhiOpenGLBuffer(TzRhi *rhi, Type t, UsageFlags u, uint32_t sz)
        : TzRhiBuffer(rhi, t, u, sz) {}
    ~TzRhiOpenGLBuffer() override { destroy(); }

    bool create() override
    {
        glGenBuffers(1, &m_glBuf);
        GLenum target = (m_usage & UniformBuffer) ? GL_UNIFORM_BUFFER
                      : (m_usage & IndexBuffer)   ? GL_ELEMENT_ARRAY_BUFFER
                                                  : GL_ARRAY_BUFFER;
        GLenum usage = (m_type == Dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
        glBindBuffer(target, m_glBuf);
        glBufferData(target, m_size, nullptr, usage);
        glBindBuffer(target, 0);
        return m_glBuf != 0;
    }

    void destroy() override
    {
        if (m_glBuf) { glDeleteBuffers(1, &m_glBuf); m_glBuf = 0; }
    }

    GLuint m_glBuf{0};
};

// ── Texture ───────────────────────────────────────────────────────────────────

class TzRhiOpenGLTexture : public TzRhiTexture
{
public:
    TzRhiOpenGLTexture(TzRhi *rhi, Format fmt, int w, int h, Flags f)
        : TzRhiTexture(rhi, fmt, w, h, f) {}
    ~TzRhiOpenGLTexture() override { destroy(); }

    bool create() override
    {
        auto glfmt = glFormatOf(m_format);
        glGenTextures(1, &m_glTex);
        glBindTexture(GL_TEXTURE_2D, m_glTex);
        glTexImage2D(GL_TEXTURE_2D, 0,
                     static_cast<GLint>(glfmt.internalFormat),
                     m_width, m_height, 0,
                     glfmt.baseFormat, glfmt.type, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
        return m_glTex != 0;
    }

    void destroy() override
    {
        if (m_glTex) { glDeleteTextures(1, &m_glTex); m_glTex = 0; }
    }

    GLuint m_glTex{0};
};

// ── Sampler ───────────────────────────────────────────────────────────────────

class TzRhiOpenGLSampler : public TzRhiSampler
{
public:
    TzRhiOpenGLSampler(TzRhi *rhi, Filter min, Filter mag, Filter mip,
                       AddressMode u, AddressMode v)
        : TzRhiSampler(rhi, min, mag, mip, u, v) {}
    ~TzRhiOpenGLSampler() override { destroy(); }

    static GLint toGLFilter(Filter f, Filter mip)
    {
        if (f == Linear && mip == Linear)  return GL_LINEAR_MIPMAP_LINEAR;
        if (f == Linear)                   return GL_LINEAR;
        return GL_NEAREST;
    }
    static GLint toGLAddr(AddressMode a)
    {
        switch (a) {
        case Repeat:         return GL_REPEAT;
        case MirroredRepeat: return GL_MIRRORED_REPEAT;
        case ClampToEdge:    return GL_CLAMP_TO_EDGE;
        }
        return GL_REPEAT;
    }

    bool create() override
    {
        glGenSamplers(1, &m_glSampler);
        glSamplerParameteri(m_glSampler, GL_TEXTURE_MIN_FILTER, toGLFilter(m_min, m_mip));
        glSamplerParameteri(m_glSampler, GL_TEXTURE_MAG_FILTER, m_mag == Linear ? GL_LINEAR : GL_NEAREST);
        glSamplerParameteri(m_glSampler, GL_TEXTURE_WRAP_S, toGLAddr(m_addressU));
        glSamplerParameteri(m_glSampler, GL_TEXTURE_WRAP_T, toGLAddr(m_addressV));
        return m_glSampler != 0;
    }

    void destroy() override
    {
        if (m_glSampler) { glDeleteSamplers(1, &m_glSampler); m_glSampler = 0; }
    }

    GLuint m_glSampler{0};
};

// ── RenderBuffer ──────────────────────────────────────────────────────────────

class TzRhiOpenGLRenderBuffer : public TzRhiRenderBuffer
{
public:
    TzRhiOpenGLRenderBuffer(TzRhi *rhi, Type t, int w, int h, int s)
        : TzRhiRenderBuffer(rhi, t, w, h, s) {}
    ~TzRhiOpenGLRenderBuffer() override { destroy(); }

    bool create() override { return true; } // Placeholder — not fully used yet
    void destroy() override {}
};

// ── RenderPassDescriptor ──────────────────────────────────────────────────────

class TzRhiOpenGLRenderPassDescriptor : public TzRhiRenderPassDescriptor
{
public:
    explicit TzRhiOpenGLRenderPassDescriptor(TzRhi *rhi)
        : TzRhiRenderPassDescriptor(rhi) {}
    ~TzRhiOpenGLRenderPassDescriptor() override { destroy(); }

    bool isCompatible(const TzRhiRenderPassDescriptor *) const override { return true; }
    void destroy() override {}
};

// ── SwapChain ─────────────────────────────────────────────────────────────────

class TzRhiOpenGLSwapChain : public TzRhiSwapChain
{
public:
    explicit TzRhiOpenGLSwapChain(TzRhi *rhi) : TzRhiSwapChain(rhi) {}
    ~TzRhiOpenGLSwapChain() override { destroy(); }

    bool createOrResize() override
    {
        if (!m_ctx.isValid()) {
            if (!m_ctx.create(m_nativeHandle, m_pixelWidth, m_pixelHeight))
                return false;
            m_ctx.makeCurrent();
            TZ_GL_LOAD_PROCS();
        } else {
            m_ctx.resize(m_pixelWidth, m_pixelHeight);
        }
        return true;
    }

    void destroy() override { m_ctx.destroy(); }

    TzGLContext m_ctx;
};

// ── ShaderResourceBindings ────────────────────────────────────────────────────

class TzRhiOpenGLShaderResourceBindings : public TzRhiShaderResourceBindings
{
public:
    explicit TzRhiOpenGLShaderResourceBindings(TzRhi *rhi)
        : TzRhiShaderResourceBindings(rhi) {}
    ~TzRhiOpenGLShaderResourceBindings() override { destroy(); }

    bool create() override { return true; }
    void destroy() override {}
};

// ── GraphicsPipeline ─────────────────────────────────────────────────────────

struct GlPipelineData
{
    GLuint program{0};
    GLuint vao{0};
};

class TzRhiOpenGLGraphicsPipeline : public TzRhiGraphicsPipeline
{
public:
    explicit TzRhiOpenGLGraphicsPipeline(TzRhi *rhi)
        : TzRhiGraphicsPipeline(rhi) {}
    ~TzRhiOpenGLGraphicsPipeline() override { destroy(); }

    bool create() override
    {
        // Compile shaders
        GLuint vert = 0, frag = 0;
        for (const auto &stage : m_stages) {
            GLuint sh = glCreateShader(stage.type() == TzRhiShaderStage::Vertex
                                       ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);
            const char *src = stage.shader().source().c_str();
            glShaderSource(sh, 1, &src, nullptr);
            glCompileShader(sh);
            GLint ok = 0;
            glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
            if (!ok) {
                char log[512]; glGetShaderInfoLog(sh, 512, nullptr, log);
                glDeleteShader(sh);
                return false;
            }
            if (stage.type() == TzRhiShaderStage::Vertex) vert = sh;
            else                                           frag = sh;
        }
        m_data.program = glCreateProgram();
        if (vert) glAttachShader(m_data.program, vert);
        if (frag) glAttachShader(m_data.program, frag);
        glLinkProgram(m_data.program);
        if (vert) glDeleteShader(vert);
        if (frag) glDeleteShader(frag);
        GLint linked = 0;
        glGetProgramiv(m_data.program, GL_LINK_STATUS, &linked);
        if (!linked) {
            glDeleteProgram(m_data.program);
            m_data.program = 0;
            return false;
        }

        // Build VAO — only enable the attribute arrays here.
        // glVertexAttribPointer is deferred to setVertexInput() where the
        // ARRAY_BUFFER is bound (required by WebGL; good practice on desktop too).
        glGenVertexArrays(1, &m_data.vao);
        glBindVertexArray(m_data.vao);
        for (const auto &attr : m_vertexLayout.attributes())
            glEnableVertexAttribArray(static_cast<GLuint>(attr.location()));
        glBindVertexArray(0);

        // Assign UBO binding points: uniform block i → binding point i.
        // glUniformBlockBinding(prog, blockIndex, bindingPoint) — the block index
        // comes from the linked program, not from the SRB.
        {
            GLint numBlocks = 0;
            glGetProgramiv(m_data.program, GL_ACTIVE_UNIFORM_BLOCKS, &numBlocks);
            for (GLint i = 0; i < numBlocks; ++i)
                glUniformBlockBinding(m_data.program,
                                      static_cast<GLuint>(i),
                                      static_cast<GLuint>(i));
        }

        // Point sampler uniforms at the texture units used by the SRB.
        // glUniform1i must be called while the program is in use.
        if (m_srb) {
            // Collect the texture units for each SampledTexture binding, sorted.
            std::vector<int> samplerUnits;
            for (const auto &b : m_srb->bindings())
                if (b.type() == TzRhiShaderResourceBinding::SampledTexture)
                    samplerUnits.push_back(b.binding());
            std::sort(samplerUnits.begin(), samplerUnits.end());

            if (!samplerUnits.empty()) {
                glUseProgram(m_data.program);
                GLint numUniforms = 0;
                glGetProgramiv(m_data.program, GL_ACTIVE_UNIFORMS, &numUniforms);
                int si = 0;
                for (GLint i = 0; i < numUniforms && si < (int)samplerUnits.size(); ++i) {
                    GLint sz = 0; GLenum type = 0; GLchar name[128] = {};
                    glGetActiveUniform(m_data.program, static_cast<GLuint>(i),
                                       128, nullptr, &sz, &type, name);
                    if (type == GL_SAMPLER_2D     || type == GL_SAMPLER_3D   ||
                        type == GL_SAMPLER_CUBE   || type == GL_SAMPLER_2D_SHADOW ||
                        type == GL_SAMPLER_2D_ARRAY) {
                        GLint loc = glGetUniformLocation(m_data.program, name);
                        if (loc >= 0)
                            glUniform1i(loc, samplerUnits[si]);
                        ++si;
                    }
                }
            }
        }
        return true;
    }

    void destroy() override
    {
        if (m_data.vao)     { glDeleteVertexArrays(1, &m_data.vao);  m_data.vao = 0; }
        if (m_data.program) { glDeleteProgram(m_data.program);       m_data.program = 0; }
    }

    GlPipelineData m_data;
};

// ─────────────────────────────────────────────────────────────────────────────
// ResourceUpdateBatch
// ─────────────────────────────────────────────────────────────────────────────

struct GlUploadOp
{
    enum Kind { StaticBuffer, DynamicBuffer, Texture };
    Kind kind;
    // buffer
    GLuint  glBuf{0};
    GLenum  bufTarget{0};
    uint32_t bufOffset{0};
    uint32_t bufSize{0};
    std::vector<uint8_t> data;
    // texture
    GLuint  glTex{0};
    GLint   texInternal{0};
    GLenum  texBase{0};
    GLenum  texType{0};
    int     texX{0}, texY{0}, texW{0}, texH{0};
    int     texLevel{0};
};

class TzRhiOpenGLResourceUpdateBatch : public TzRhiResourceUpdateBatch
{
public:
    void uploadStaticBuffer(TzRhiBuffer *buf, const void *data, uint32_t byteSize) override
    {
        auto *glBuf = static_cast<TzRhiOpenGLBuffer *>(buf);
        GlUploadOp op;
        op.kind      = GlUploadOp::StaticBuffer;
        op.glBuf     = glBuf->m_glBuf;
        op.bufTarget = (buf->usage() & TzRhiBuffer::UniformBuffer) ? GL_UNIFORM_BUFFER
                     : (buf->usage() & TzRhiBuffer::IndexBuffer)   ? GL_ELEMENT_ARRAY_BUFFER
                                                                    : GL_ARRAY_BUFFER;
        op.bufSize   = byteSize;
        op.data.assign(static_cast<const uint8_t *>(data),
                       static_cast<const uint8_t *>(data) + byteSize);
        m_ops.push_back(std::move(op));
    }

    void updateDynamicBuffer(TzRhiBuffer *buf, uint32_t offset,
                             uint32_t size, const void *data) override
    {
        auto *glBuf = static_cast<TzRhiOpenGLBuffer *>(buf);
        GlUploadOp op;
        op.kind      = GlUploadOp::DynamicBuffer;
        op.glBuf     = glBuf->m_glBuf;
        op.bufTarget = (buf->usage() & TzRhiBuffer::UniformBuffer) ? GL_UNIFORM_BUFFER
                     : (buf->usage() & TzRhiBuffer::IndexBuffer)   ? GL_ELEMENT_ARRAY_BUFFER
                                                                    : GL_ARRAY_BUFFER;
        op.bufOffset = offset;
        op.bufSize   = size;
        op.data.assign(static_cast<const uint8_t *>(data),
                       static_cast<const uint8_t *>(data) + size);
        m_ops.push_back(std::move(op));
    }

    void uploadTexture(TzRhiTexture *tex,
                       const TzRhiTextureUploadDescription &desc) override
    {
        auto *glTex = static_cast<TzRhiOpenGLTexture *>(tex);
        auto glfmt  = glFormatOf(tex->format());
        int  w      = (desc.width  < 0) ? tex->width()  : desc.width;
        int  h      = (desc.height < 0) ? tex->height() : desc.height;
        GlUploadOp op;
        op.kind        = GlUploadOp::Texture;
        op.glTex       = glTex->m_glTex;
        op.texInternal = static_cast<GLint>(glfmt.internalFormat);
        op.texBase     = glfmt.baseFormat;
        op.texType     = glfmt.type;
        op.texX        = desc.x;
        op.texY        = desc.y;
        op.texW        = w;
        op.texH        = h;
        op.texLevel    = desc.level;
        op.data.assign(static_cast<const uint8_t *>(desc.data),
                       static_cast<const uint8_t *>(desc.data) + desc.byteSize);
        m_ops.push_back(std::move(op));
    }

    void flush()
    {
        for (const auto &op : m_ops) {
            switch (op.kind) {
            case GlUploadOp::StaticBuffer:
                glBindBuffer(op.bufTarget, op.glBuf);
                glBufferData(op.bufTarget, op.bufSize, op.data.data(), GL_STATIC_DRAW);
                glBindBuffer(op.bufTarget, 0);
                break;
            case GlUploadOp::DynamicBuffer:
                glBindBuffer(op.bufTarget, op.glBuf);
                glBufferSubData(op.bufTarget, op.bufOffset, op.bufSize, op.data.data());
                glBindBuffer(op.bufTarget, 0);
                break;
            case GlUploadOp::Texture:
                glBindTexture(GL_TEXTURE_2D, op.glTex);
                glTexSubImage2D(GL_TEXTURE_2D, op.texLevel,
                                op.texX, op.texY, op.texW, op.texH,
                                op.texBase, op.texType, op.data.data());
                glBindTexture(GL_TEXTURE_2D, 0);
                break;
            }
        }
        m_ops.clear();
    }

    bool isEmpty() const { return m_ops.empty(); }

private:
    std::vector<GlUploadOp> m_ops;
};

// ─────────────────────────────────────────────────────────────────────────────
// CommandBuffer
// ─────────────────────────────────────────────────────────────────────────────

class TzRhiOpenGLCommandBuffer : public TzRhiCommandBuffer
{
public:
    void beginPass(TzRhiRenderTarget *rt,
                   TzRhiRenderPassDescriptor *,
                   const ClearValue &cv,
                   TzRhiResourceUpdateBatch *uploads) override
    {
        if (uploads) {
            static_cast<TzRhiOpenGLResourceUpdateBatch *>(uploads)->flush();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        if (rt) {
            glViewport(0, 0,
                       static_cast<GLsizei>(rt->pixelWidth()),
                       static_cast<GLsizei>(rt->pixelHeight()));
        }

        glClearColor(cv.r, cv.g, cv.b, cv.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    void endPass(TzRhiResourceUpdateBatch *uploads) override
    {
        if (uploads)
            static_cast<TzRhiOpenGLResourceUpdateBatch *>(uploads)->flush();
    }

    void setGraphicsPipeline(TzRhiGraphicsPipeline *ps) override
    {
        m_currentPipeline = static_cast<TzRhiOpenGLGraphicsPipeline *>(ps);
        glUseProgram(m_currentPipeline->m_data.program);
        glBindVertexArray(m_currentPipeline->m_data.vao);

        // Depth
        if (ps->depthTest()) {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(glCompareOp(ps->depthOp()));
        } else {
            glDisable(GL_DEPTH_TEST);
        }
        glDepthMask(ps->depthWrite() ? GL_TRUE : GL_FALSE);

        // Cull
        if (ps->cullMode() != TzRhiGraphicsPipeline::NoCulling) {
            glEnable(GL_CULL_FACE);
            glCullFace(ps->cullMode() == TzRhiGraphicsPipeline::Front ? GL_FRONT : GL_BACK);
        } else {
            glDisable(GL_CULL_FACE);
        }
        glFrontFace(ps->frontFace() == TzRhiGraphicsPipeline::CW ? GL_CW : GL_CCW);
        glLineWidth(ps->lineWidth());

        // Blend (first target only for now)
        const auto &blends = ps->targetBlends();
        if (!blends.empty() && blends[0].enable) {
            glEnable(GL_BLEND);
            glBlendFuncSeparate(
                glBlendFactor(blends[0].srcColor), glBlendFactor(blends[0].dstColor),
                glBlendFactor(blends[0].srcAlpha), glBlendFactor(blends[0].dstAlpha));
            glBlendEquationSeparate(glBlendOp(blends[0].opColor), glBlendOp(blends[0].opAlpha));
        } else {
            glDisable(GL_BLEND);
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
                auto *glBuf = static_cast<TzRhiOpenGLBuffer *>(b.buffer());
                uint32_t sz = (b.ubSize() > 0) ? b.ubSize() : b.buffer()->size();
                glBindBufferRange(GL_UNIFORM_BUFFER,
                                  static_cast<GLuint>(b.binding()),
                                  glBuf->m_glBuf,
                                  static_cast<GLintptr>(b.ubOffset()),
                                  static_cast<GLsizeiptr>(sz));
            } else if (b.type() == TzRhiShaderResourceBinding::SampledTexture) {
                auto *glTex  = static_cast<TzRhiOpenGLTexture *>(b.texture());
                auto *glSamp = static_cast<TzRhiOpenGLSampler *>(b.sampler());
                glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + b.binding()));
                glBindTexture(GL_TEXTURE_2D, glTex ? glTex->m_glTex : 0);
                glBindSampler(static_cast<GLuint>(b.binding()),
                              glSamp ? glSamp->m_glSampler : 0);
            }
        }
    }

    void setViewport(float x, float y, float w, float h,
                     float /*minDepth*/, float /*maxDepth*/) override
    {
        glViewport(static_cast<GLint>(x), static_cast<GLint>(y),
                   static_cast<GLsizei>(w), static_cast<GLsizei>(h));
    }

    void setScissor(int x, int y, int w, int h) override
    {
        glEnable(GL_SCISSOR_TEST);
        glScissor(x, y, w, h);
    }

    void setVertexInput(int startBinding, int count,
                        const TzRhiBuffer *const *bufs,
                        const uint32_t *offsets,
                        TzRhiBuffer *indexBuf, uint32_t indexOffset,
                        IndexFormat indexFormat) override
    {
        // Re-bind vertex buffers into the VAO's attribute slots.
        // Each binding slot maps to one or more vertex attributes —
        // we just bind the GL buffer and the VAO remembers the pointer setup.
        for (int i = 0; i < count; ++i) {
            const auto *glBuf = static_cast<const TzRhiOpenGLBuffer *>(bufs[i]);
            glBindBuffer(GL_ARRAY_BUFFER, glBuf->m_glBuf);
            // Re-issue VertexAttribPointer for each attribute that uses this binding,
            // accounting for the per-draw offset.
            if (m_currentPipeline) {
                uint32_t off = offsets ? offsets[i] : 0;
                for (const auto &attr : m_currentPipeline->vertexInputLayout().attributes()) {
                    if (attr.binding() != startBinding + i)
                        continue;
                    GLboolean norm = GL_FALSE;
                    GLenum    type = glAttribType(attr.format(), norm);
                    GLint     comp = glAttribComponents(attr.format());
                    const auto &binds = m_currentPipeline->vertexInputLayout().bindings();
                    GLsizei stride = (attr.binding() < (int)binds.size())
                                   ? static_cast<GLsizei>(binds[attr.binding()].stride()) : 0;
                    glVertexAttribPointer(
                        static_cast<GLuint>(attr.location()),
                        comp, type, norm, stride,
                        reinterpret_cast<const void *>(
                            static_cast<uintptr_t>(attr.offset() + off)));
                }
            }
        }

        if (indexBuf) {
            auto *ib = static_cast<TzRhiOpenGLBuffer *>(indexBuf);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib->m_glBuf);
            m_indexOffset = indexOffset;
            m_indexFormat = indexFormat;
        }

        m_primitive = m_currentPipeline
            ? glPrimitive(m_currentPipeline->topology())
            : GL_TRIANGLES;
    }

    void draw(uint32_t vertexCount, uint32_t /*instanceCount*/,
              uint32_t firstVertex, uint32_t /*firstInstance*/) override
    {
        glDrawArrays(m_primitive, static_cast<GLint>(firstVertex),
                     static_cast<GLsizei>(vertexCount));
    }

    void drawIndexed(uint32_t indexCount, uint32_t /*instanceCount*/,
                     uint32_t firstIndex, int32_t /*vertexOffset*/,
                     uint32_t /*firstInstance*/) override
    {
        GLenum type = (m_indexFormat == UInt16) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
        uint32_t byteOff = m_indexOffset + firstIndex *
                           ((m_indexFormat == UInt16) ? 2u : 4u);
        glDrawElements(m_primitive, static_cast<GLsizei>(indexCount),
                       type,
                       reinterpret_cast<const void *>(static_cast<uintptr_t>(byteOff)));
    }

    void debugMarkBegin(const char *) override {}
    void debugMarkEnd() override {}

private:
    TzRhiOpenGLGraphicsPipeline *m_currentPipeline{nullptr};
    GLenum   m_primitive{GL_TRIANGLES};
    uint32_t m_indexOffset{0};
    IndexFormat m_indexFormat{UInt16};
};

// ─────────────────────────────────────────────────────────────────────────────
// TzRhiOpenGL
// ─────────────────────────────────────────────────────────────────────────────

TzRhiOpenGL::TzRhiOpenGL()
    : TzRhi(Backend::OpenGL)
    , m_commandBuffer(std::make_unique<TzRhiOpenGLCommandBuffer>())
    , m_updateBatch(std::make_unique<TzRhiOpenGLResourceUpdateBatch>())
{}

TzRhiOpenGL::~TzRhiOpenGL() = default;

TzRhiBuffer *TzRhiOpenGL::newBuffer(TzRhiBuffer::Type t,
                                    TzRhiBuffer::UsageFlags u, uint32_t sz)
{
    return new TzRhiOpenGLBuffer(this, t, u, sz);
}

TzRhiTexture *TzRhiOpenGL::newTexture(TzRhiTexture::Format fmt, int w, int h,
                                      TzRhiTexture::Flags f)
{
    return new TzRhiOpenGLTexture(this, fmt, w, h, f);
}

TzRhiSampler *TzRhiOpenGL::newSampler(TzRhiSampler::Filter min,
                                      TzRhiSampler::Filter mag,
                                      TzRhiSampler::Filter mip,
                                      TzRhiSampler::AddressMode u,
                                      TzRhiSampler::AddressMode v)
{
    return new TzRhiOpenGLSampler(this, min, mag, mip, u, v);
}

TzRhiRenderBuffer *TzRhiOpenGL::newRenderBuffer(TzRhiRenderBuffer::Type t,
                                                int w, int h, int s)
{
    return new TzRhiOpenGLRenderBuffer(this, t, w, h, s);
}

TzRhiRenderPassDescriptor *TzRhiOpenGL::newCompatibleRenderPassDescriptor(
    TzRhiRenderTarget *)
{
    return new TzRhiOpenGLRenderPassDescriptor(this);
}

TzRhiSwapChain *TzRhiOpenGL::newSwapChain()
{
    return new TzRhiOpenGLSwapChain(this);
}

TzRhiShaderResourceBindings *TzRhiOpenGL::newShaderResourceBindings()
{
    return new TzRhiOpenGLShaderResourceBindings(this);
}

TzRhiGraphicsPipeline *TzRhiOpenGL::newGraphicsPipeline()
{
    return new TzRhiOpenGLGraphicsPipeline(this);
}

TzRhiResourceUpdateBatch *TzRhiOpenGL::nextResourceUpdateBatch()
{
    return m_updateBatch.get();
}

bool TzRhiOpenGL::beginFrame(TzRhiSwapChain *swapChain)
{
    auto *sc = static_cast<TzRhiOpenGLSwapChain *>(swapChain);
    if (!sc->m_ctx.isValid())
        return false;
    sc->m_ctx.makeCurrent();
    return true;
}

void TzRhiOpenGL::endFrame(TzRhiSwapChain *swapChain)
{
    auto *sc = static_cast<TzRhiOpenGLSwapChain *>(swapChain);
    glFlush();
    sc->m_ctx.swapBuffers();
}

TzRhiCommandBuffer *TzRhiOpenGL::currentFrameCommandBuffer()
{
    return m_commandBuffer.get();
}

TzRhiRenderTarget *TzRhiOpenGL::currentFrameRenderTarget(TzRhiSwapChain *swapChain)
{
    return static_cast<TzRhiOpenGLSwapChain *>(swapChain);
}
