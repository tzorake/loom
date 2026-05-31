#include <loom/tzrhiresource.hpp>
#include <loom/tzrhibuffer.hpp>
#include <loom/tzrhitexture.hpp>
#include <loom/tzrhisampler.hpp>
#include <loom/tzrhirenderbuffer.hpp>
#include <loom/tzrhirenderpassdescriptor.hpp>
#include <loom/tzrhiswapchain.hpp>
#include <loom/tzrhishaderresourcebindings.hpp>
#include <loom/tzrhigraphicspipeline.hpp>
#include <loom/tzrhirendertarget.hpp>
#include <loom/tzrhicommandbuffer.hpp>
#include <loom/tzrhiresourceupdatebatch.hpp>

// ── TzRhiResource ─────────────────────────────────────────────────────────────

TzRhiResource::TzRhiResource(TzRhi *rhi)
    : m_rhi(rhi)
{}

TzRhiResource::~TzRhiResource() = default;

// ── TzRhiBuffer ───────────────────────────────────────────────────────────────

TzRhiBuffer::TzRhiBuffer(TzRhi *rhi, Type type, UsageFlags usage, uint32_t size)
    : TzRhiResource(rhi)
    , m_type(type)
    , m_usage(usage)
    , m_size(size)
{}

TzRhiBuffer::~TzRhiBuffer() = default;

// ── TzRhiTexture ──────────────────────────────────────────────────────────────

TzRhiTexture::TzRhiTexture(TzRhi *rhi, Format format, int w, int h, Flags flags)
    : TzRhiResource(rhi)
    , m_format(format)
    , m_width(w)
    , m_height(h)
    , m_flags(flags)
{}

TzRhiTexture::~TzRhiTexture() = default;

// ── TzRhiSampler ─────────────────────────────────────────────────────────────

TzRhiSampler::TzRhiSampler(TzRhi *rhi,
                            Filter min, Filter mag, Filter mip,
                            AddressMode u, AddressMode v)
    : TzRhiResource(rhi)
    , m_min(min)
    , m_mag(mag)
    , m_mip(mip)
    , m_addressU(u)
    , m_addressV(v)
{}

TzRhiSampler::~TzRhiSampler() = default;

// ── TzRhiRenderBuffer ────────────────────────────────────────────────────────

TzRhiRenderBuffer::TzRhiRenderBuffer(TzRhi *rhi, Type type, int w, int h, int samples)
    : TzRhiResource(rhi)
    , m_type(type)
    , m_width(w)
    , m_height(h)
    , m_samples(samples)
{}

TzRhiRenderBuffer::~TzRhiRenderBuffer() = default;

// ── TzRhiRenderTarget ────────────────────────────────────────────────────────

TzRhiRenderTarget::~TzRhiRenderTarget() = default;

// ── TzRhiRenderPassDescriptor ────────────────────────────────────────────────

TzRhiRenderPassDescriptor::TzRhiRenderPassDescriptor(TzRhi *rhi)
    : TzRhiResource(rhi)
{}

TzRhiRenderPassDescriptor::~TzRhiRenderPassDescriptor() = default;

// ── TzRhiSwapChain ───────────────────────────────────────────────────────────

TzRhiSwapChain::TzRhiSwapChain(TzRhi *rhi)
    : TzRhiResource(rhi)
{}

TzRhiSwapChain::~TzRhiSwapChain() = default;

// ── TzRhiShaderResourceBindings ──────────────────────────────────────────────

TzRhiShaderResourceBindings::TzRhiShaderResourceBindings(TzRhi *rhi)
    : TzRhiResource(rhi)
{}

TzRhiShaderResourceBindings::~TzRhiShaderResourceBindings() = default;

// ── TzRhiGraphicsPipeline ────────────────────────────────────────────────────

TzRhiGraphicsPipeline::TzRhiGraphicsPipeline(TzRhi *rhi)
    : TzRhiResource(rhi)
{}

TzRhiGraphicsPipeline::~TzRhiGraphicsPipeline() = default;

// ── TzRhiCommandBuffer ───────────────────────────────────────────────────────

TzRhiCommandBuffer::~TzRhiCommandBuffer() = default;

// ── TzRhiResourceUpdateBatch ─────────────────────────────────────────────────

TzRhiResourceUpdateBatch::~TzRhiResourceUpdateBatch() = default;
