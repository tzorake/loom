#pragma once

#include <cstdint>

class TzRhiBuffer;
class TzRhiTexture;

// Batches CPU→GPU data uploads.
//
// Obtain via TzRhi::nextResourceUpdateBatch(). Pass to beginPass() to execute
// the uploads at the start of the render pass, or to endPass() to schedule
// them after the pass. The batch is consumed and recycled by the backend —
// do not use the pointer after it has been passed to beginPass/endPass.
//
// TextureUploadDescription — a simple struct for specifying what to upload
// to a texture layer/mip. The data pointer must remain valid until the batch
// is submitted.

struct TzRhiTextureUploadDescription
{
    const void *data{nullptr};
    uint32_t    byteSize{0};
    int         layer{0};       // array layer (0 for 2-D textures)
    int         level{0};       // mip level
    int         x{0}, y{0};     // destination origin (pixels)
    int         width{-1};      // -1 = full texture width at this mip
    int         height{-1};     // -1 = full texture height at this mip
};

class TzRhiResourceUpdateBatch
{
public:
    virtual ~TzRhiResourceUpdateBatch();

    // Buffer uploads
    virtual void uploadStaticBuffer(TzRhiBuffer *buf,
                                    const void *data, uint32_t byteSize) = 0;
    virtual void updateDynamicBuffer(TzRhiBuffer *buf,
                                     uint32_t offset, uint32_t size,
                                     const void *data) = 0;

    // Texture uploads
    virtual void uploadTexture(TzRhiTexture *tex,
                               const TzRhiTextureUploadDescription &desc) = 0;

    // Convenience: upload the full base-level image.
    void uploadTexture(TzRhiTexture *tex, const void *data, uint32_t byteSize)
    {
        TzRhiTextureUploadDescription desc;
        desc.data     = data;
        desc.byteSize = byteSize;
        uploadTexture(tex, desc);
    }
};
