#ifndef TZSCENE_P_HPP
#define TZSCENE_P_HPP

#include <loom/tzclasshelpermacros.hpp>
#include <loom/tzscene.hpp>

#include <cstdint>
#include <vector>

class TzPlatformSurface;
class TzWidget;
struct TzRect;

// Forward-declare RHI types so we don't force a hard dependency on loom-rhi
// when loom-rhi is not built. Callers who use the RHI path must link loom-rhi.
class TzRhi;
class TzRhiSwapChain;

class TzScenePrivate
{
    TZ_DECLARE_PUBLIC(TzScene)
public:
    void layoutWidget(TzWidget *w, int passesLeft);
    void paintWidget(TzWidget *w, double parentAbsX, double parentAbsY, const TzRect &parentClip);
    void registerSubtree(TzWidget *w);
    void unregisterSubtree(TzWidget *w);
    TzWidget *widgetAtHelper(TzWidget *w, double x, double y, double parentAbsX,
                             double parentAbsY) const;

    TzScene *q_ptr{nullptr};

    TzPlatformSurface *platformSurface{nullptr};
    TzWidget *root{nullptr};
    TzWidget *focusedWidget{nullptr};

    std::vector<uint32_t> pixels;
    int width{0};
    int height{0};
    bool paintDirty{true};
    bool layoutDirty{false};

    // Optional GPU path. Both are null when the software rasterizer is used.
    TzRhi         *rhi{nullptr};
    TzRhiSwapChain *rhiSwapChain{nullptr};
};

#endif // TZSCENE_P_HPP
