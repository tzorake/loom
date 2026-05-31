#pragma once

#include "tzrhiresource.hpp"
#include "tzrhirendertarget.hpp"
#include <loom/tzplatformsurface.hpp>

class TzRhiRenderPassDescriptor;

// SwapChain — wraps a platform window surface and manages frame presentation.
//
// Usage:
//   auto *sc = rhi->newSwapChain();
//   sc->setWindow(window->nativeWindowHandle());
//   sc->createOrResize();                          // call again on resize
//   auto *rpd = rhi->newCompatibleRenderPassDescriptor(sc);
//   sc->setRenderPassDescriptor(rpd);
//
// Call createOrResize() whenever the window is resized (e.g. inside the app's
// resize event, before the next beginFrame).

class TzRhiSwapChain : public TzRhiResource, public TzRhiRenderTarget
{
public:
    ~TzRhiSwapChain() override;

    TzRhiResource::Type resourceType() const override { return TzRhiResource::SwapChain; }

    void setWindow(const TzNativeWindowHandle &handle) { m_nativeHandle = handle; }
    const TzNativeWindowHandle &window() const         { return m_nativeHandle; }

    void setRenderPassDescriptor(TzRhiRenderPassDescriptor *rpd) { m_rpd = rpd; }
    TzRhiRenderPassDescriptor *renderPassDescriptor() const      { return m_rpd; }

    // Create (or recreate after resize) the underlying surface and framebuffer.
    // Must be called before the first beginFrame and after every window resize.
    virtual bool createOrResize() = 0;

    // TzRhiRenderTarget
    int pixelWidth()  const override { return m_pixelWidth; }
    int pixelHeight() const override { return m_pixelHeight; }

protected:
    explicit TzRhiSwapChain(TzRhi *rhi);

    TzNativeWindowHandle        m_nativeHandle;
    TzRhiRenderPassDescriptor  *m_rpd{nullptr};
    int                         m_pixelWidth{0};
    int                         m_pixelHeight{0};
};
