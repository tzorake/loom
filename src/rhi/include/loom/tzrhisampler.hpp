#pragma once

#include "tzrhiresource.hpp"

// Texture sampler — controls filtering and address modes.
//
// Configure all fields then call create().

class TzRhiSampler : public TzRhiResource
{
public:
    enum Filter {
        None,
        Nearest,
        Linear
    };

    enum AddressMode {
        Repeat,
        MirroredRepeat,
        ClampToEdge
    };

    ~TzRhiSampler() override;

    TzRhiResource::Type resourceType() const override { return TzRhiResource::Sampler; }

    void setMinFilter(Filter f)    { m_min = f; }
    void setMagFilter(Filter f)    { m_mag = f; }
    void setMipmapFilter(Filter f) { m_mip = f; }
    void setAddressModeU(AddressMode a) { m_addressU = a; }
    void setAddressModeV(AddressMode a) { m_addressV = a; }

    Filter      minFilter()    const { return m_min; }
    Filter      magFilter()    const { return m_mag; }
    Filter      mipmapFilter() const { return m_mip; }
    AddressMode addressModeU() const { return m_addressU; }
    AddressMode addressModeV() const { return m_addressV; }

    virtual bool create() = 0;

protected:
    TzRhiSampler(TzRhi *rhi,
                 Filter min, Filter mag, Filter mip,
                 AddressMode u, AddressMode v);

    Filter      m_min;
    Filter      m_mag;
    Filter      m_mip;
    AddressMode m_addressU;
    AddressMode m_addressV;
};
