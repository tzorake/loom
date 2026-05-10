#include <loom/tzpoint.hpp>

bool TzPoint::operator==(const TzPoint &o) const
{
    return x == o.x && y == o.y;
}

bool TzPoint::operator!=(const TzPoint &o) const
{
    return !(*this == o);
}
