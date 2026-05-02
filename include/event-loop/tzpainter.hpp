#ifndef TZPAINTER_HPP
#define TZPAINTER_HPP

#include <event-loop/tzgeometry.hpp>

#include <cstdint>
#include <string>

// TzPainter — immediate-mode pixel-buffer renderer.
//
// Coordinates passed to draw methods are in the widget's local space
// (0,0 = top-left corner of the widget).  The painter internally translates
// by its offset and clips to its clip rectangle before writing pixels.
class TzPainter
{
public:
    TzPainter(uint32_t *pixels, int bufferWidth, int bufferHeight,
              const TzRect &clip, const TzPoint &offset);

    // ── Filled shapes ────────────────────────────────────────────────────
    void fillRect(const TzRect &rect, uint32_t argb);
    void fillRect(double x, double y, double w, double h, uint32_t argb);

    // ── Outlined shapes ──────────────────────────────────────────────────
    void drawRect(const TzRect &rect, uint32_t argb, double lineWidth = 1.0);
    void drawLine(const TzPoint &a, const TzPoint &b, uint32_t argb, double lineWidth = 1.0);

    // ── Text (embedded 8×8 bitmap font, printable ASCII) ─────────────────
    void drawText(const TzPoint &pos, const std::string &text, uint32_t argb);
    void drawText(double x, double y, const std::string &text, uint32_t argb);

    // ── Inspection ───────────────────────────────────────────────────────
    TzRect  clipRect() const;
    TzPoint offset()   const;

    // ── Create a child painter for a sub-widget ───────────────────────────
    // childOffset is the sub-widget's top-left in the current widget's local coords.
    // childClip   is the sub-widget's bounding box in the current widget's local coords.
    TzPainter childPainter(const TzPoint &childOffset, const TzRect &childClip) const;

private:
    uint32_t *m_pixels;
    int       m_bufferWidth;
    int       m_bufferHeight;
    TzRect    m_clip;   // window-absolute clip rectangle
    TzPoint   m_offset; // widget top-left in window coords

    // Sets pixel at window-absolute (px, py) with alpha-over blending.
    void setPixel(int px, int py, uint32_t argb);

    // Returns true if (px, py) is inside m_clip.
    bool inClip(int px, int py) const;

    static uint32_t blendOver(uint32_t dst, uint32_t src);
};

#endif // TZPAINTER_HPP
