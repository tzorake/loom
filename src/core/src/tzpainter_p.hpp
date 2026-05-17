#ifndef TZPAINTER_P_HPP
#define TZPAINTER_P_HPP

#include <loom/tzclasshelpermacros.hpp>
#include <loom/tzpainter.hpp>

#include <cstdint>

class TzPainterPrivate
{
    TZ_DECLARE_PUBLIC(TzPainter)
public:
    TzPainter *q_ptr{nullptr};

    uint32_t *pixels{nullptr};
    int bufferWidth{0};
    int bufferHeight{0};
    TzRect clip{};
    TzPoint offset{};

    void setPixel(int px, int py, uint32_t argb);
    bool inClip(int px, int py) const;
    static uint32_t blendOver(uint32_t dst, uint32_t src);
};

#endif // TZPAINTER_P_HPP
