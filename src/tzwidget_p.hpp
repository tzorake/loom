#ifndef TZWIDGET_P_HPP
#define TZWIDGET_P_HPP

#include <event-loop/tzwidget.hpp>

class TzScene;
class TzAnchors;

class TzWidgetPrivate
{
public:
    explicit TzWidgetPrivate() = default;

    TzWidget  *q_ptr{ nullptr };

    TzRect    geometry{};
    TzSize    implicitSize{};

    bool      explicitWidth{ false };
    bool      explicitHeight{ false };
    bool      visible{ true };
    bool      focused{ false };

    TzAnchors *anchors{ nullptr };  // lazily allocated
    TzScene   *scene{ nullptr };    // set by TzScene when widget joins scene
};

#endif // TZWIDGET_P_HPP
