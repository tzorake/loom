#ifndef TZMARGINS_HPP
#define TZMARGINS_HPP

struct TzMargins
{
    explicit TzMargins(double uniform = 0.0);
    TzMargins(double l, double t, double r, double b);

    double left{ 0.0 };
    double top{ 0.0 };
    double right{ 0.0 };
    double bottom{ 0.0 };
};

#endif // TZMARGINS_HPP
