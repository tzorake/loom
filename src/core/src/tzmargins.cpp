#include <loom/tzmargins.hpp>

TzMargins::TzMargins(double uniform)
    : left(uniform)
    , top(uniform)
    , right(uniform)
    , bottom(uniform)
{
}

TzMargins::TzMargins(double l, double t, double r, double b)
    : left(l)
    , top(t)
    , right(r)
    , bottom(b)
{
}
