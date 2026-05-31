// quad.cpp — Textured quad example for loom-rhi.
//
// Renders a UV-mapped quad that spins at ~60 fps using the loom-rhi OpenGL
// or WebGL backend.  Demonstrates the minimal usage pattern:
//
//   TzRhi::create → newSwapChain → newBuffer → newTexture → newSampler
//   → newShaderResourceBindings → newGraphicsPipeline
//   → frame loop: beginFrame → beginPass → draw calls → endPass → endFrame
//
// Compile with LOOM_BUILD_RHI=ON.  The executable is loom_quad (native) or
// loom_quad.wasm (web).

#include <loom/tzguiapplication.hpp>
#include <loom/tzwindow.hpp>
#include <loom/tzrhi.hpp>
#include <loom/tzrhiswapchain.hpp>
#include <loom/tzrhirenderpassdescriptor.hpp>
#include <loom/tzrhibuffer.hpp>
#include <loom/tzrhitexture.hpp>
#include <loom/tzrhisampler.hpp>
#include <loom/tzrhishaderresourcebindings.hpp>
#include <loom/tzrhigraphicspipeline.hpp>
#include <loom/tzrhicommandbuffer.hpp>
#include <loom/tzrhiresourceupdatebatch.hpp>
#include <loom/tzrhivertexinputlayout.hpp>
#include <loom/tzshader.hpp>
#include <loom/tzrhishaderstage.hpp>
#include <loom/tzresizeevent.hpp>

#include <cmath>
#include <cstdint>
#include <vector>

// ── Vertex layout ─────────────────────────────────────────────────────────────
// Each vertex: position (x, y) + UV (u, v) — 4 floats / 16 bytes.

struct Vertex { float x, y, u, v; };

static const Vertex kVerts[] = {
    { -0.6f,  0.6f,  0.0f, 1.0f },   // top-left
    {  0.6f,  0.6f,  1.0f, 1.0f },   // top-right
    {  0.6f, -0.6f,  1.0f, 0.0f },   // bottom-right
    { -0.6f, -0.6f,  0.0f, 0.0f },   // bottom-left
};
static const uint16_t kIdx[] = { 0, 1, 2,  0, 2, 3 };

// ── Uniform buffer ────────────────────────────────────────────────────────────
struct Uniforms {
    float cosA, sinA, pad0, pad1;   // column-major 2×2 rotation, padded to vec4
};

// ── Checkerboard texture ──────────────────────────────────────────────────────

static std::vector<uint32_t> makeCheckerboard(int size)
{
    std::vector<uint32_t> px(static_cast<size_t>(size * size));
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            bool on = ((x / (size / 8)) + (y / (size / 8))) % 2 == 0;
            px[static_cast<size_t>(y * size + x)] = on ? 0xFFFF9900u : 0xFF006699u; // ARGB
        }
    }
    // Convert ARGB → RGBA (swap R and B)
    for (auto &p : px) {
        uint8_t a = (p >> 24) & 0xFF;
        uint8_t r = (p >> 16) & 0xFF;
        uint8_t g = (p >>  8) & 0xFF;
        uint8_t b =  p        & 0xFF;
        p = (static_cast<uint32_t>(a) << 24)
          | (static_cast<uint32_t>(b) << 16)
          | (static_cast<uint32_t>(g) <<  8)
          |  static_cast<uint32_t>(r);
    }
    return px;
}

// ── GLSL sources ─────────────────────────────────────────────────────────────
//
// Desktop (OpenGL 3.3 Core): #version 330 core
// Web (WebGL 2 / GLSL ES 3.0): #version 300 es
//
// We select the correct preamble at runtime.

#if defined(LOOM_RHI_WEBGL)
static const char kVertSrc[] =
    "#version 300 es\n"
    "layout(std140) uniform Uniforms { float cosA; float sinA; float p0; float p1; };\n"
    "layout(location = 0) in vec2 aPos;\n"
    "layout(location = 1) in vec2 aUV;\n"
    "out vec2 vUV;\n"
    "void main() {\n"
    "    vec2 r = vec2(cosA * aPos.x - sinA * aPos.y,\n"
    "                  sinA * aPos.x + cosA * aPos.y);\n"
    "    gl_Position = vec4(r, 0.0, 1.0);\n"
    "    vUV = aUV;\n"
    "}\n";
static const char kFragSrc[] =
    "#version 300 es\n"
    "precision mediump float;\n"
    "uniform sampler2D uTex;\n"
    "in vec2 vUV;\n"
    "out vec4 fragColor;\n"
    "void main() { fragColor = texture(uTex, vUV); }\n";
#else
static const char kVertSrc[] =
    "#version 330 core\n"
    "layout(std140) uniform Uniforms { float cosA; float sinA; float p0; float p1; };\n"
    "layout(location = 0) in vec2 aPos;\n"
    "layout(location = 1) in vec2 aUV;\n"
    "out vec2 vUV;\n"
    "void main() {\n"
    "    vec2 r = vec2(cosA * aPos.x - sinA * aPos.y,\n"
    "                  sinA * aPos.x + cosA * aPos.y);\n"
    "    gl_Position = vec4(r, 0.0, 1.0);\n"
    "    vUV = aUV;\n"
    "}\n";
static const char kFragSrc[] =
    "#version 330 core\n"
    "uniform sampler2D uTex;\n"
    "in vec2 vUV;\n"
    "out vec4 fragColor;\n"
    "void main() { fragColor = texture(uTex, vUV); }\n";
#endif

// ── QuadWindow ────────────────────────────────────────────────────────────────

class QuadWindow : public TzWindow
{
public:
    QuadWindow() : TzWindow(640, 480, nullptr) {}
    ~QuadWindow() override { cleanup(); }

    void init()
    {
#if defined(LOOM_RHI_WEBGL)
        m_rhi = TzRhi::create(TzRhi::Backend::WebGL);
#else
        m_rhi = TzRhi::create(TzRhi::Backend::OpenGL);
#endif
        if (!m_rhi) return;

        // SwapChain
        m_sc = m_rhi->newSwapChain();
        m_sc->setWindow(surfaceHandle()->nativeWindowHandle());
        m_sc->setPixelSize(width(), height()); // set initial size before createOrResize
        if (!m_sc->createOrResize()) return;

        m_rpd = m_rhi->newCompatibleRenderPassDescriptor(m_sc);
        m_sc->setRenderPassDescriptor(m_rpd);

        // Buffers
        m_vbuf = m_rhi->newBuffer(TzRhiBuffer::Immutable,
                                  TzRhiBuffer::VertexBuffer, sizeof(kVerts));
        m_vbuf->create();
        m_ibuf = m_rhi->newBuffer(TzRhiBuffer::Immutable,
                                  TzRhiBuffer::IndexBuffer, sizeof(kIdx));
        m_ibuf->create();
        m_ubuf = m_rhi->newBuffer(TzRhiBuffer::Dynamic,
                                  TzRhiBuffer::UniformBuffer, sizeof(Uniforms));
        m_ubuf->create();

        // Texture
        constexpr int texSize = 256;
        m_tex = m_rhi->newTexture(TzRhiTexture::RGBA8, texSize, texSize);
        m_tex->create();
        m_sampler = m_rhi->newSampler(TzRhiSampler::Linear, TzRhiSampler::Linear,
                                      TzRhiSampler::None,
                                      TzRhiSampler::ClampToEdge, TzRhiSampler::ClampToEdge);
        m_sampler->create();

        // Bindings
        m_srb = m_rhi->newShaderResourceBindings();
        m_srb->setBindings({
            TzRhiShaderResourceBinding::uniformBuffer(
                0,
                TzRhiShaderResourceBinding::VertexStage,
                m_ubuf),
            TzRhiShaderResourceBinding::sampledTexture(
                1,
                TzRhiShaderResourceBinding::FragmentStage,
                m_tex, m_sampler),
        });
        m_srb->create();

        // Vertex input layout
        TzRhiVertexInputLayout vil;
        vil.setBindings({ TzRhiVertexInputBinding(sizeof(Vertex)) });
        vil.setAttributes({
            TzRhiVertexInputAttribute(0, 0, TzRhiVertexInputAttribute::Float2,
                                      offsetof(Vertex, x)),
            TzRhiVertexInputAttribute(0, 1, TzRhiVertexInputAttribute::Float2,
                                      offsetof(Vertex, u)),
        });

        // Pipeline
        m_ps = m_rhi->newGraphicsPipeline();
        m_ps->setShaderStages({
            TzRhiShaderStage(TzRhiShaderStage::Vertex,   TzShader::fromSource(kVertSrc)),
            TzRhiShaderStage(TzRhiShaderStage::Fragment, TzShader::fromSource(kFragSrc)),
        });
        m_ps->setVertexInputLayout(vil);
        m_ps->setShaderResourceBindings(m_srb);
        m_ps->setRenderPassDescriptor(m_rpd);
        m_ps->setTopology(TzRhiGraphicsPipeline::Triangles);
        m_ps->create();

        // Upload static data once
        auto *batch = m_rhi->nextResourceUpdateBatch();
        batch->uploadStaticBuffer(m_vbuf, kVerts, sizeof(kVerts));
        batch->uploadStaticBuffer(m_ibuf, kIdx,   sizeof(kIdx));
        auto checker = makeCheckerboard(texSize);
        batch->uploadTexture(m_tex, checker.data(),
                             static_cast<uint32_t>(checker.size() * sizeof(uint32_t)));
        // batch is consumed in the first beginPass call below (stored for first frame)
        m_firstBatch = batch;

        setRhi(m_rhi.get(), m_sc);
    }

    void renderFrame()
    {
        if (!m_rhi || !m_sc) return;

        // Advance rotation
        m_angle += 0.02f;

        // Update uniform buffer
        Uniforms u;
        u.cosA = std::cos(m_angle);
        u.sinA = std::sin(m_angle);
        u.pad0 = u.pad1 = 0.0f;
        auto *dynBatch = m_rhi->nextResourceUpdateBatch();
        dynBatch->updateDynamicBuffer(m_ubuf, 0, sizeof(Uniforms), &u);

        if (!m_rhi->beginFrame(m_sc))
            return;

        auto *cb = m_rhi->currentFrameCommandBuffer();
        auto *rt = m_rhi->currentFrameRenderTarget(m_sc);

        TzRhiCommandBuffer::ClearValue cv;
        cv.r = 0.1f; cv.g = 0.1f; cv.b = 0.15f; cv.a = 1.0f;

        cb->beginPass(rt, m_rpd, cv, m_firstBatch ? m_firstBatch : dynBatch);
        m_firstBatch = nullptr;

        cb->setGraphicsPipeline(m_ps);
        cb->setShaderResources(m_srb);
        cb->setViewport(0.0f, 0.0f,
                        static_cast<float>(rt->pixelWidth()),
                        static_cast<float>(rt->pixelHeight()));

        const TzRhiBuffer *vbufs[] = { m_vbuf };
        const uint32_t    voffs[]  = { 0 };
        cb->setVertexInput(0, 1, vbufs, voffs, m_ibuf, 0, TzRhiCommandBuffer::UInt16);
        cb->drawIndexed(6);

        cb->endPass();
        m_rhi->endFrame(m_sc);
    }

protected:
    void resizeEvent(TzResizeEvent *event) override
    {
        TzWindow::resizeEvent(event);
        if (m_sc) {
            // Update swapchain size on the WebGL side.
            // On OpenGL the context tracks the surface automatically.
            m_sc->createOrResize();
        }
    }

private:
    void cleanup()
    {
        if (!m_rhi) return;
        if (m_ps)      { m_ps->destroy();      delete m_ps;      }
        if (m_srb)     { m_srb->destroy();     delete m_srb;     }
        if (m_sampler) { m_sampler->destroy(); delete m_sampler; }
        if (m_tex)     { m_tex->destroy();     delete m_tex;     }
        if (m_ubuf)    { m_ubuf->destroy();    delete m_ubuf;    }
        if (m_ibuf)    { m_ibuf->destroy();    delete m_ibuf;    }
        if (m_vbuf)    { m_vbuf->destroy();    delete m_vbuf;    }
        if (m_rpd)     { m_rpd->destroy();     delete m_rpd;     }
        if (m_sc)      { m_sc->destroy();      delete m_sc;      }
    }

    std::unique_ptr<TzRhi>        m_rhi;
    TzRhiSwapChain               *m_sc{nullptr};
    TzRhiRenderPassDescriptor    *m_rpd{nullptr};
    TzRhiBuffer                  *m_vbuf{nullptr};
    TzRhiBuffer                  *m_ibuf{nullptr};
    TzRhiBuffer                  *m_ubuf{nullptr};
    TzRhiTexture                 *m_tex{nullptr};
    TzRhiSampler                 *m_sampler{nullptr};
    TzRhiShaderResourceBindings  *m_srb{nullptr};
    TzRhiGraphicsPipeline        *m_ps{nullptr};
    TzRhiResourceUpdateBatch     *m_firstBatch{nullptr};  // consumed in first frame
    float                         m_angle{0.0f};
};

// ── Entry point ───────────────────────────────────────────────────────────────

int loom_main(int argc, char *argv[])
{
    auto *app = new TzGuiApplication(argc, argv);
    auto *win = new QuadWindow();
    win->setTitle("loom-rhi textured quad");
    win->init();
    win->show();
    return app->exec();
}
