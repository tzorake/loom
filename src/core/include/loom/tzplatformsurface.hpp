#ifndef TZPLATFORMSURFACE_HPP
#define TZPLATFORMSURFACE_HPP

#include <cstdint>
#include <functional>
#include <vector>

class TzSurface;
class TzKeyEvent;
class TzMouseEvent;

class TzPlatformSurface
{
public:
    using CloseCallback = std::function<void()>;
    using ResizeCallback = std::function<void(int, int)>;
    using KeyCallback = std::function<void(TzKeyEvent *)>;
    using MouseCallback = std::function<void(TzMouseEvent *)>;

    virtual ~TzPlatformSurface();

    // Back-pointer to the public TzSurface that owns this platform surface.
    virtual TzSurface *surface() const = 0;
    virtual void setSurface(TzSurface *surface) = 0;

    // Input / lifecycle routing
    virtual void setCloseCallback(CloseCallback cb) = 0;
    virtual void setResizeCallback(ResizeCallback cb) = 0;
    virtual void setKeyCallback(KeyCallback cb) = 0;
    virtual void setMouseCallback(MouseCallback cb) = 0;

    // Raster rendering
    virtual void render(const std::vector<uint32_t> &pixels, int width, int height) = 0;
};

#endif // TZPLATFORMSURFACE_HPP
