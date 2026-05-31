#pragma once

#include "tzrhiresource.hpp"
#include "tzrhishaderstage.hpp"
#include "tzrhivertexinputlayout.hpp"
#include <vector>

class TzRhiShaderResourceBindings;
class TzRhiRenderPassDescriptor;

// Graphics pipeline — immutable compiled state.
//
// Set all properties (shaders, vertex layout, topology, blend/depth/cull
// state, resource bindings, render pass descriptor) then call create().
// The pipeline is immutable once created.

class TzRhiGraphicsPipeline : public TzRhiResource
{
public:
    // ── Topology ─────────────────────────────────────────────────────────────
    enum Topology { Triangles, TriangleStrip, Lines, Points };

    // ── Cull mode ────────────────────────────────────────────────────────────
    enum CullMode { NoCulling, Front, Back };

    // ── Front face ───────────────────────────────────────────────────────────
    enum FrontFace { CCW, CW };

    // ── Comparison function (depth / stencil) ────────────────────────────────
    enum CompareOp { Never, Less, Equal, LessOrEqual, Greater, NotEqual, GreaterOrEqual, Always };

    // ── Stencil operation ────────────────────────────────────────────────────
    enum StencilOp { StencilZero, Keep, Replace, IncrementAndClamp, DecrementAndClamp,
                     Invert, IncrementAndWrap, DecrementAndWrap };

    // ── Blend factor ─────────────────────────────────────────────────────────
    enum BlendFactor { Zero, One, SrcColor, OneMinusSrcColor, DstColor, OneMinusDstColor,
                       SrcAlpha, OneMinusSrcAlpha, DstAlpha, OneMinusDstAlpha,
                       ConstantColor, OneMinusConstantColor, SrcAlphaSaturate };

    // ── Blend operation ──────────────────────────────────────────────────────
    enum BlendOp { Add, Subtract, ReverseSubtract, BlendMin, BlendMax };

    // ── Per-render-target blend state ────────────────────────────────────────
    struct TargetBlend {
        bool        enable{false};
        BlendFactor srcColor{One};
        BlendFactor dstColor{Zero};
        BlendOp     opColor{Add};
        BlendFactor srcAlpha{One};
        BlendFactor dstAlpha{Zero};
        BlendOp     opAlpha{Add};
        // Color write mask (all enabled by default)
        bool writeR{true}, writeG{true}, writeB{true}, writeA{true};
    };

    // ── Stencil face state ───────────────────────────────────────────────────
    struct StencilOpState {
        StencilOp failOp{Keep};
        StencilOp depthFailOp{Keep};
        StencilOp passOp{Keep};
        CompareOp compareOp{Always};
    };

    ~TzRhiGraphicsPipeline() override;

    TzRhiResource::Type resourceType() const override
    {
        return TzRhiResource::GraphicsPipeline;
    }

    // ── Shader stages ────────────────────────────────────────────────────────
    void setShaderStages(std::vector<TzRhiShaderStage> stages)
    {
        m_stages = std::move(stages);
    }
    const std::vector<TzRhiShaderStage> &shaderStages() const { return m_stages; }

    // ── Vertex input ─────────────────────────────────────────────────────────
    void setVertexInputLayout(const TzRhiVertexInputLayout &layout) { m_vertexLayout = layout; }
    const TzRhiVertexInputLayout &vertexInputLayout() const { return m_vertexLayout; }

    // ── Primitive topology ───────────────────────────────────────────────────
    void setTopology(Topology t) { m_topology = t; }
    Topology topology()    const { return m_topology; }

    // ── Rasterization ────────────────────────────────────────────────────────
    void setCullMode(CullMode m)  { m_cullMode = m; }
    void setFrontFace(FrontFace f){ m_frontFace = f; }
    void setLineWidth(float w)    { m_lineWidth = w; }
    CullMode  cullMode()  const { return m_cullMode; }
    FrontFace frontFace() const { return m_frontFace; }
    float     lineWidth() const { return m_lineWidth; }

    // ── Depth test / write ───────────────────────────────────────────────────
    void setDepthTest(bool on)       { m_depthTest = on; }
    void setDepthWrite(bool on)      { m_depthWrite = on; }
    void setDepthOp(CompareOp op)    { m_depthOp = op; }
    bool      depthTest()  const { return m_depthTest; }
    bool      depthWrite() const { return m_depthWrite; }
    CompareOp depthOp()    const { return m_depthOp; }

    // ── Stencil test ─────────────────────────────────────────────────────────
    void setStencilTest(bool on)                       { m_stencilTest = on; }
    void setStencilFront(const StencilOpState &state)  { m_stencilFront = state; }
    void setStencilBack(const StencilOpState &state)   { m_stencilBack = state; }
    void setStencilReadMask(uint32_t mask)              { m_stencilReadMask = mask; }
    void setStencilWriteMask(uint32_t mask)             { m_stencilWriteMask = mask; }
    bool              stencilTest()      const { return m_stencilTest; }
    StencilOpState    stencilFront()     const { return m_stencilFront; }
    StencilOpState    stencilBack()      const { return m_stencilBack; }
    uint32_t          stencilReadMask()  const { return m_stencilReadMask; }
    uint32_t          stencilWriteMask() const { return m_stencilWriteMask; }

    // ── Blend ────────────────────────────────────────────────────────────────
    void setTargetBlends(std::vector<TargetBlend> blends) { m_targetBlends = std::move(blends); }
    const std::vector<TargetBlend> &targetBlends() const  { return m_targetBlends; }

    // ── Resource bindings + render pass (must set before create()) ───────────
    void setShaderResourceBindings(TzRhiShaderResourceBindings *srb) { m_srb = srb; }
    void setRenderPassDescriptor(TzRhiRenderPassDescriptor *rpd)     { m_rpd = rpd; }
    TzRhiShaderResourceBindings *shaderResourceBindings() const { return m_srb; }
    TzRhiRenderPassDescriptor   *renderPassDescriptor()   const { return m_rpd; }

    virtual bool create() = 0;

protected:
    explicit TzRhiGraphicsPipeline(TzRhi *rhi);

    std::vector<TzRhiShaderStage>  m_stages;
    TzRhiVertexInputLayout         m_vertexLayout;
    Topology                       m_topology{Triangles};
    CullMode                       m_cullMode{NoCulling};
    FrontFace                      m_frontFace{CCW};
    float                          m_lineWidth{1.0f};
    bool                           m_depthTest{false};
    bool                           m_depthWrite{false};
    CompareOp                      m_depthOp{Less};
    bool                           m_stencilTest{false};
    StencilOpState                 m_stencilFront;
    StencilOpState                 m_stencilBack;
    uint32_t                       m_stencilReadMask{0xFF};
    uint32_t                       m_stencilWriteMask{0xFF};
    std::vector<TargetBlend>       m_targetBlends;
    TzRhiShaderResourceBindings   *m_srb{nullptr};
    TzRhiRenderPassDescriptor     *m_rpd{nullptr};
};
