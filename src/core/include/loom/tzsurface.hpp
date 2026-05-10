#ifndef TZSURFACE_HPP
#define TZSURFACE_HPP

class TzPlatformSurface;

class TzSurface
{
public:
    enum SurfaceType { RasterSurface };

    virtual ~TzSurface();

    virtual SurfaceType surfaceType() const = 0;

    virtual TzPlatformSurface *surfaceHandle() = 0;
    virtual const TzPlatformSurface *surfaceHandle() const = 0;
};

#endif // TZSURFACE_HPP
