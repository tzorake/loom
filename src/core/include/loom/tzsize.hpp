#ifndef TZSIZE_HPP
#define TZSIZE_HPP

struct TzSize
{
    bool isEmpty() const;

    bool operator==(const TzSize &o) const;
    bool operator!=(const TzSize &o) const;

    double width{0.0};
    double height{0.0};
};

#endif // TZSIZE_HPP
