#pragma once

#include "tzrhibuffer.hpp"
#include "tzrhitexture.hpp"
#include "tzrhisampler.hpp"
#include "tzrhirenderbuffer.hpp"
#include "tzrhirendertarget.hpp"
#include "tzrhirenderpassdescriptor.hpp"
#include "tzrhiswapchain.hpp"
#include "tzrhigraphicspipeline.hpp"
#include "tzrhishaderresourcebindings.hpp"
#include "tzrhicommandbuffer.hpp"
#include "tzrhiresourceupdatebatch.hpp"

#include <memory>

// TzRhi — Rendering Hardware Interface factory and frame lifecycle.
//
// Create via the static factory:
//   auto rhi = TzRhi::create(TzRhi::Backend::OpenGL);
//
// All resource objects are created through this class and must be destroyed
// (via their destroy() method) before the TzRhi instance is deleted.
//
// Frame lifecycle:
//   if (rhi->beginFrame(swapChain)) {
//       auto *cb = rhi->currentFrameCommandBuffer();
//       cb->beginPass(rhi->currentFrameRenderTarget(sc), rpd, clearValue, uploads);
//       // ... draw calls ...
//       cb->endPass();
//       rhi->endFrame(swapChain);
//   }

class TzRhi
{
public:
    enum class Backend { OpenGL, WebGL };

    virtual ~TzRhi();

    // Factory — returns nullptr if the requested backend is unavailable.
    static std::unique_ptr<TzRhi> create(Backend backend);

    Backend backend() const { return m_backend; }

    // ── Resource factories ────────────────────────────────────────────────────
    // All returned objects are owned by the caller.  Call destroy() on each
    // resource before deleting the TzRhi that created it.

    virtual TzRhiBuffer *newBuffer(TzRhiBuffer::Type type,
                                   TzRhiBuffer::UsageFlags usage,
                                   uint32_t size) = 0;

    virtual TzRhiTexture *newTexture(TzRhiTexture::Format format,
                                     int w, int h,
                                     TzRhiTexture::Flags flags = 0) = 0;

    virtual TzRhiSampler *newSampler(TzRhiSampler::Filter minFilter,
                                     TzRhiSampler::Filter magFilter,
                                     TzRhiSampler::Filter mipmapFilter,
                                     TzRhiSampler::AddressMode addressU,
                                     TzRhiSampler::AddressMode addressV) = 0;

    virtual TzRhiRenderBuffer *newRenderBuffer(TzRhiRenderBuffer::Type type,
                                               int w, int h,
                                               int samples = 1) = 0;

    virtual TzRhiRenderPassDescriptor *newCompatibleRenderPassDescriptor(
        TzRhiRenderTarget *rt) = 0;

    virtual TzRhiSwapChain            *newSwapChain() = 0;
    virtual TzRhiShaderResourceBindings *newShaderResourceBindings() = 0;
    virtual TzRhiGraphicsPipeline     *newGraphicsPipeline() = 0;

    // Obtain (or recycle) a resource update batch. The returned batch is valid
    // until it is passed to beginPass() or endPass(), at which point the
    // backend reclaims it. Never delete it manually.
    virtual TzRhiResourceUpdateBatch  *nextResourceUpdateBatch() = 0;

    // ── Frame lifecycle ───────────────────────────────────────────────────────
    // beginFrame returns false if the frame should be skipped (e.g. window
    // minimised, surface lost). Always check the return value.
    virtual bool beginFrame(TzRhiSwapChain *swapChain) = 0;
    virtual void endFrame(TzRhiSwapChain *swapChain) = 0;

    // Valid between beginFrame / endFrame.
    virtual TzRhiCommandBuffer *currentFrameCommandBuffer() = 0;
    virtual TzRhiRenderTarget  *currentFrameRenderTarget(TzRhiSwapChain *swapChain) = 0;

    // ── Coordinate-system queries ─────────────────────────────────────────────
    // GL and WebGL share the same NDC (+Y up) but differ in framebuffer
    // orientation (GL: +Y up; WebGL canvas: +Y down via CSS transform).
    virtual bool isYUpInNdc()         const = 0;
    virtual bool isYUpInFramebuffer() const = 0;

protected:
    explicit TzRhi(Backend backend);

    Backend m_backend;
};
