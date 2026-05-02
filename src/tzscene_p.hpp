#ifndef TZSCENE_P_HPP
#define TZSCENE_P_HPP

#include <event-loop/tzclasshelpermacros.hpp>
#include <event-loop/tzscene.hpp>
#include <event-loop/tzgeometry.hpp>

#include <cstdint>
#include <vector>

class TzAbstractWindow;
class TzWidget;

class TzScenePrivate
{
    TZ_DECLARE_PUBLIC(TzScene)
public:
    TzScene          *q_ptr{ nullptr };

    TzAbstractWindow *window{ nullptr };
    TzWidget         *root{ nullptr };
    TzWidget         *focusedWidget{ nullptr };

    std::vector<uint32_t> pixels;
    int  width{ 0 };
    int  height{ 0 };
    bool paintDirty{ true };
    bool layoutDirty{ false };

    // ── Helpers called from TzScene methods ───────────────────────────────
    void layoutWidget(TzWidget *w, int passesLeft);
    void paintWidget (TzWidget *w, double parentAbsX, double parentAbsY,
                      const TzRect &parentClip);
    void registerSubtree  (TzWidget *w);
    void unregisterSubtree(TzWidget *w);
    TzWidget *widgetAtHelper(TzWidget *w, double x, double y,
                              double parentAbsX, double parentAbsY) const;
};

#endif // TZSCENE_P_HPP
