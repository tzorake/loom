#ifndef TZWINDOW_P_HPP
#define TZWINDOW_P_HPP

#include <loom/tzclasshelpermacros.hpp>
#include <loom/tzwindow.hpp>

#include "tzobject_p.hpp"

#include <cstdint>
#include <vector>

class TzPlatformWindow;
class TzScene;
class TzTimer;

class TzWindowPrivate : public TzObjectPrivate
{
    TZ_DECLARE_PUBLIC(TzWindow)
public:
    ~TzWindowPrivate() override;

    TzPlatformWindow *platformWindow{nullptr};
    TzScene *scene{nullptr};
    TzTimer *paintTimer{nullptr};

    std::vector<uint32_t> pixels;
    bool paintDirty{false};
};

#endif // TZWINDOW_P_HPP
