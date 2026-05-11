#ifndef TZWAYLANDWINDOW_P_HPP
#define TZWAYLANDWINDOW_P_HPP

#include "tzwaylandwindow.hpp"
#include "tzwaylandglobals.hpp"

#include <loom/tzcloseevent.hpp>
#include <loom/tzkeyevent.hpp>
#include <loom/tzmouseevent.hpp>
#include <loom/tzresizeevent.hpp>
#include <loom/tzcoreapplication.hpp>

#include <wayland-client.h>
#include "protocol/xdg-shell-client-protocol.h"
#include "protocol/xdg-decoration-client-protocol.h"

#include <sys/mman.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

struct ShmBuffer {
    int           fd       { -1 };
    void         *data     { nullptr };
    size_t        capacity { 0 };   // mmap'd bytes (may be larger than byteSize)
    size_t        byteSize { 0 };   // bytes for current width*height
    wl_shm_pool  *pool     { nullptr };
    wl_buffer    *wlBuf    { nullptr };
    bool          busy     { false };
    int           width    { 0 };
    int           height   { 0 };

    ~ShmBuffer() { destroyPool(); }

    void destroyBuffer()
    {
        if (wlBuf) { wl_buffer_destroy(wlBuf); wlBuf = nullptr; }
    }

    void destroyPool()
    {
        destroyBuffer();
        if (pool) { wl_shm_pool_destroy(pool); pool = nullptr; }
        if (data && data != MAP_FAILED) { munmap(data, capacity); data = nullptr; }
        if (fd >= 0) { close(fd); fd = -1; }
        capacity = 0;
        busy = false;
    }

    // Grow the pool to at least `needed` bytes without remapping if already large enough.
    bool ensureCapacity(wl_shm *shm, size_t needed)
    {
        if (capacity >= needed)
            return true;

        size_t newCap = capacity ? capacity * 2 : needed;
        if (newCap < needed) newCap = needed;

        if (fd < 0) {
            // First allocation.
#if defined(__NR_memfd_create)
            fd = static_cast<int>(syscall(__NR_memfd_create, "loom-shm", 0));
#endif
            if (fd < 0) {
                char name[] = "/loom-shm-XXXXXX";
                fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
                if (fd >= 0) shm_unlink(name);
            }
            if (fd < 0) return false;
            if (ftruncate(fd, static_cast<off_t>(newCap)) < 0) { close(fd); fd = -1; return false; }
            data = mmap(nullptr, newCap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            if (data == MAP_FAILED) { data = nullptr; close(fd); fd = -1; return false; }
            pool = wl_shm_create_pool(shm, fd, static_cast<int32_t>(newCap));
            if (!pool) { munmap(data, newCap); data = nullptr; close(fd); fd = -1; return false; }
        } else {
            // Grow existing allocation.
            if (ftruncate(fd, static_cast<off_t>(newCap)) < 0) return false;
            void *newData = mremap(data, capacity, newCap, MREMAP_MAYMOVE);
            if (newData == MAP_FAILED) return false;
            data = newData;
            wl_shm_pool_resize(pool, static_cast<int32_t>(newCap));
        }
        capacity = newCap;
        return true;
    }
};

class TzWaylandWindowPrivate
{
public:
    TzWaylandWindowPrivate(int width, int height, TzWaylandWindow *owner);
    ~TzWaylandWindowPrivate();

    void setTitle(const std::string &title);
    void show();
    void hide();
    void render(const std::vector<uint32_t> &pixels, int width, int height);

    void onConfigure(int32_t width, int32_t height);
    void onClose();

    TzWaylandWindow *owner       { nullptr };

    wl_surface                    *surface      { nullptr };
    xdg_surface                   *xdgSurface   { nullptr };
    xdg_toplevel                  *xdgToplevel  { nullptr };
    zxdg_toplevel_decoration_v1   *decoration   { nullptr };

    int windowWidth  { 0 };
    int windowHeight { 0 };
    int pendingWidth { 0 };
    int pendingHeight{ 0 };

    // Up to 2 SHM buffers alternated to avoid stalling the compositor.
    ShmBuffer buffers[2];
    int currentBuffer { 0 };

    // Frame callback — fires once per VSync after the compositor presents the
    // current frame.  Used to coalesce rapid configure events into one render.
    wl_callback *frameCallback  { nullptr };
    bool         pendingRedraw  { false };

    std::mutex pixelMutex;
};

#endif // TZWAYLANDWINDOW_P_HPP
