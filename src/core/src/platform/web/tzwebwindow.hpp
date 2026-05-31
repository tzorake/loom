#pragma once

#include "tzplatformwindow.hpp"

#include <cstdint>
#include <vector>

// Web platform window — backed by an HTML <canvas> element.
//
// There is only ever one canvas, so this class keeps a module-level singleton
// pointer.  JavaScript delivers DOM events (mouse, keyboard, resize, close)
// through the exported C functions loom_mouse_move(), loom_key_event(), etc.,
// which route them to this instance.
//
// Rendering
// ─────────
// The framework's painter produces pixels as packed uint32 values in
// 0xAARRGGBB format (big-endian bit layout; little-endian byte layout in
// WASM memory → BGRA bytes).  Canvas ImageData expects RGBA bytes, so
// render() converts the buffer before passing the pointer to JS.

class TzWebWindow : public TzPlatformWindow
{
public:
    explicit TzWebWindow(int width, int height);
    ~TzWebWindow() override;

    // TzPlatformWindow
    void setTitle(const std::string &title) override;
    void show()                             override;
    void hide()                             override;

    // TzPlatformSurface
    void render(const std::vector<uint32_t> &pixels,
                int width, int height)      override;

    TzNativeWindowHandle nativeWindowHandle() const override;

    // Called from the exported loom_resize() to update cached dimensions
    // and post a TzResizeEvent.
    void onResize(int width, int height);

    // Module-level singleton.
    static TzWebWindow *instance();

private:
    int m_width{0};
    int m_height{0};

    // Scratch buffer for ARGB→RGBA conversion; reused across frames.
    std::vector<uint32_t> m_rgba;

    static TzWebWindow *s_instance;
};
