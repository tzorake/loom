#ifndef TZPOINT_HPP
#define TZPOINT_HPP

struct TzPoint
{
    bool operator==(const TzPoint &o) const;
    bool operator!=(const TzPoint &o) const;

    double x{0.0};
    double y{0.0};
};

#endif // TZPOINT_HPP
