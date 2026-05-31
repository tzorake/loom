#pragma once

#include "tzrhiresource.hpp"

// Renderbuffer — color or depth/stencil attachment for off-screen rendering.
//
// Used when the rendered result is not needed as a texture (e.g. depth buffer
// or MSAA resolve target). Call create() after setting all properties.

class TzRhiRenderBuffer : public TzRhiResource
{
public:
    enum Type {
        Color,
        DepthStencil
    };

    ~TzRhiRenderBuffer() override;

    TzRhiResource::Type resourceType() const override { return TzRhiResource::RenderBuffer; }

    void setType(Type t)    { m_type = t; }
    void setWidth(int w)    { m_width = w; }
    void setHeight(int h)   { m_height = h; }
    void setSamples(int s)  { m_samples = s; }

    Type type()    const { return m_type; }
    int  width()   const { return m_width; }
    int  height()  const { return m_height; }
    int  samples() const { return m_samples; }

    virtual bool create() = 0;

protected:
    TzRhiRenderBuffer(TzRhi *rhi, Type type, int w, int h, int samples);

    Type m_type;
    int  m_width;
    int  m_height;
    int  m_samples;
};
