#include <loom/tzmargins.hpp>
#include <loom/tzpoint.hpp>
#include <loom/tzrect.hpp>
#include <loom/tzsize.hpp>

#include <algorithm>

TzPoint TzRect::topLeft() const
{
    return {x, y};
}

TzPoint TzRect::topRight() const
{
    return {x + width, y};
}

TzPoint TzRect::bottomLeft() const
{
    return {x, y + height};
}

TzPoint TzRect::bottomRight() const
{
    return {x + width, y + height};
}

TzPoint TzRect::center() const
{
    return {x + width / 2.0, y + height / 2.0};
}

TzSize TzRect::size() const
{
    return {width, height};
}

bool TzRect::isValid() const
{
    return width >= 0.0 && height >= 0.0;
}
bool TzRect::isEmpty() const
{
    return width <= 0.0 || height <= 0.0;
}

bool TzRect::contains(double px, double py) const
{
    return px >= x && px < x + width && py >= y && py < y + height;
}

bool TzRect::contains(const TzPoint &p) const
{
    return contains(p.x, p.y);
}

bool TzRect::equals(double x, double y, double width, double height) const
{
    return this->x == x && this->y == y && this->width == width && this->height == height;
}

TzRect TzRect::adjusted(const TzMargins &m) const
{
    return {x + m.left, y + m.top, width - m.left - m.right, height - m.top - m.bottom};
}

TzRect TzRect::intersected(const TzRect &r) const
{
    double x1 = std::max(x, r.x);
    double y1 = std::max(y, r.y);
    double x2 = std::min(x + width, r.x + r.width);
    double y2 = std::min(y + height, r.y + r.height);
    if (x2 <= x1 || y2 <= y1)
        return {};
    return {x1, y1, x2 - x1, y2 - y1};
}

bool TzRect::operator==(const TzRect &o) const
{
    return equals(o.x, o.y, o.width, o.height);
}

bool TzRect::operator!=(const TzRect &o) const
{
    return !(*this == o);
}
