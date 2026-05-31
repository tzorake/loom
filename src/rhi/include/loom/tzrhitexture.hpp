#pragma once

#include "tzrhiresource.hpp"

// 2-D GPU texture.
//
// Set format, size, and flags before calling create(). Pixel data is uploaded
// via TzRhiResourceUpdateBatch::uploadTexture().

class TzRhiTexture : public TzRhiResource
{
public:
    enum Format {
        UnknownFormat,
        RGBA8,
        BGRA8,
        R8,
        RG8,
        R16F,
        RGBA16F,
        RGBA32F,
        D16,
        D24S8,
        D32F,
        RGBA8_sRGB
    };

    enum Flag {
        GenerateMipmaps        = 1 << 0,
        UsedAsColorAttachment  = 1 << 1
    };
    using Flags = unsigned int;

    ~TzRhiTexture() override;

    TzRhiResource::Type resourceType() const override { return TzRhiResource::Texture; }

    void setFormat(Format f) { m_format = f; }
    void setWidth(int w)     { m_width = w; }
    void setHeight(int h)    { m_height = h; }
    void setFlags(Flags f)   { m_flags = f; }

    Format format() const { return m_format; }
    int    width()  const { return m_width; }
    int    height() const { return m_height; }
    Flags  flags()  const { return m_flags; }

    virtual bool create() = 0;

protected:
    TzRhiTexture(TzRhi *rhi, Format format, int w, int h, Flags flags);

    Format m_format;
    int    m_width;
    int    m_height;
    Flags  m_flags;
};
