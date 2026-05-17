#ifndef TZRECT_HPP
#define TZRECT_HPP

struct TzPoint;
struct TzSize;
struct TzMargins;

struct TzRect
{
    TzPoint topLeft() const;
    TzPoint topRight() const;
    TzPoint bottomLeft() const;
    TzPoint bottomRight() const;
    TzPoint center() const;
    TzSize size() const;

    bool isValid() const;
    bool isEmpty() const;

    bool contains(double px, double py) const;
    bool contains(const TzPoint &p) const;
    bool equals(double x, double y, double width, double height) const;

    TzRect adjusted(const TzMargins &m) const;
    TzRect intersected(const TzRect &b) const;

    bool operator==(const TzRect &o) const;
    bool operator!=(const TzRect &o) const;

    double x{0.0};
    double y{0.0};
    double width{0.0};
    double height{0.0};
};

#endif // TZRECT_HPP
