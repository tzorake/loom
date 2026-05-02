#ifndef TZPAINTER_HPP
#define TZPAINTER_HPP

#include <event-loop/tzclasshelpermacros.hpp>
#include <event-loop/tzgeometry.hpp>

#include <cstdint>
#include <memory>
#include <string>

class TzPainterPrivate;

class TzPainter
{
    TZ_DECLARE_PRIVATE(TzPainter)
public:
    TzPainter(uint32_t *pixels, int bufferWidth, int bufferHeight,
              const TzRect &clip, const TzPoint &offset);
    ~TzPainter();

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
    TzPainter childPainter(const TzPoint &childOffset, const TzRect &childClip) const;

private:
    std::unique_ptr<TzPainterPrivate> d_ptr;
};

#endif // TZPAINTER_HPP
