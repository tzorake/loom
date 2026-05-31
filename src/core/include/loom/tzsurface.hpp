#ifndef TZSURFACE_HPP
#define TZSURFACE_HPP

class TzPlatformSurface;

class TzSurface
{
public:
    enum SurfaceType {
        RasterSurface,   // CPU pixel buffer — existing software rasterizer path
        OpenGLSurface,   // Desktop OpenGL 3.3+ (loom-rhi OpenGL backend)
        WebGLSurface     // WebGL 2.0 in browser (loom-rhi WebGL backend)
    };

    virtual ~TzSurface();

    virtual SurfaceType surfaceType() const = 0;

    virtual TzPlatformSurface *surfaceHandle() = 0;
    virtual const TzPlatformSurface *surfaceHandle() const = 0;
};

#endif // TZSURFACE_HPP
