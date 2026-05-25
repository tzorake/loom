#ifndef TZPAINTER_HPP
#define TZPAINTER_HPP

#include <loom/tzclasshelpermacros.hpp>

#include <cstdint>
#include <memory>
#include <string>

class TzPainterPrivate;
struct TzRect;
struct TzPoint;

class TzPainter
{
    TZ_DECLARE_PRIVATE(TzPainter)
public:
    TzPainter(uint32_t *pixels, int bufferWidth, int bufferHeight, const TzRect &clip,
              const TzPoint &offset);
    ~TzPainter();

    void fillRect(const TzRect &rect, uint32_t argb);
    void fillRect(double x, double y, double w, double h, uint32_t argb);

    void drawRect(const TzRect &rect, uint32_t argb, double lineWidth = 1.0);
    void drawLine(const TzPoint &a, const TzPoint &b, uint32_t argb, double lineWidth = 1.0);
    void drawCircle(const TzPoint &center, double radius, uint32_t argb);

    void drawText(const TzPoint &pos, const std::string &text, uint32_t argb);
    void drawText(double x, double y, const std::string &text, uint32_t argb);

    TzRect clipRect() const;
    TzPoint offset() const;

    TzPainter childPainter(const TzPoint &childOffset, const TzRect &childClip) const;

private:
    std::unique_ptr<TzPainterPrivate> d_ptr;
};

#endif // TZPAINTER_HPP
