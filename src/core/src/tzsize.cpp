#include <loom/tzsize.hpp>

bool TzSize::isEmpty() const
{
    return width <= 0.0 || height <= 0.0;
}

bool TzSize::operator==(const TzSize &o) const
{
    return width == o.width && height == o.height;
}

bool TzSize::operator!=(const TzSize &o) const
{
    return !(*this == o);
}
