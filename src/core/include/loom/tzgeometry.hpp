#ifndef TZGEOMETRY_HPP
#define TZGEOMETRY_HPP

#include <algorithm>

struct TzPoint
{
    double x{0.0};
    double y{0.0};

    bool operator==(const TzPoint &o) const { return x == o.x && y == o.y; }
    bool operator!=(const TzPoint &o) const { return !(*this == o); }
};

struct TzSize
{
    double width{0.0};
    double height{0.0};

    bool isEmpty() const { return width <= 0.0 || height <= 0.0; }

    bool operator==(const TzSize &o) const { return width == o.width && height == o.height; }
    bool operator!=(const TzSize &o) const { return !(*this == o); }
};

struct TzMargins
{
    double left{0.0};
    double top{0.0};
    double right{0.0};
    double bottom{0.0};

    explicit TzMargins(double uniform = 0.0)
        : left(uniform)
        , top(uniform)
        , right(uniform)
        , bottom(uniform)
    {}

    TzMargins(double l, double t, double r, double b)
        : left(l)
        , top(t)
        , right(r)
        , bottom(b)
    {}
};

struct TzRect
{
    double x{0.0};
    double y{0.0};
    double width{0.0};
    double height{0.0};

    TzPoint topLeft() const { return {x, y}; }
    TzPoint topRight() const { return {x + width, y}; }
    TzPoint bottomLeft() const { return {x, y + height}; }
    TzPoint bottomRight() const { return {x + width, y + height}; }
    TzPoint center() const { return {x + width / 2.0, y + height / 2.0}; }
    TzSize size() const { return {width, height}; }

    bool isValid() const { return width >= 0.0 && height >= 0.0; }
    bool isEmpty() const { return width <= 0.0 || height <= 0.0; }

    bool contains(double px, double py) const
    {
        return px >= x && px < x + width && py >= y && py < y + height;
    }
    bool contains(const TzPoint &p) const { return contains(p.x, p.y); }

    TzRect adjusted(const TzMargins &m) const
    {
        return {x + m.left, y + m.top, width - m.left - m.right, height - m.top - m.bottom};
    }

    bool operator==(const TzRect &o) const
    {
        return x == o.x && y == o.y && width == o.width && height == o.height;
    }
    bool operator!=(const TzRect &o) const { return !(*this == o); }
};

inline TzRect intersected(const TzRect &a, const TzRect &b)
{
    double x1 = std::max(a.x, b.x);
    double y1 = std::max(a.y, b.y);
    double x2 = std::min(a.x + a.width, b.x + b.width);
    double y2 = std::min(a.y + a.height, b.y + b.height);
    if (x2 <= x1 || y2 <= y1)
        return {};
    return {x1, y1, x2 - x1, y2 - y1};
}

#endif // TZGEOMETRY_HPP
