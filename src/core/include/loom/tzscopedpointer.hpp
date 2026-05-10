#ifndef TZSCOPEDPOINTER_HPP
#define TZSCOPEDPOINTER_HPP

#include <event-loop/tzclasshelpermacros.hpp>

#include <functional>
#include <memory>

template <typename T>
class TzScopedPointer : public std::unique_ptr<T>
{
public:
    TzScopedPointer(T *pointer = nullptr) : std::unique_ptr<T>(pointer) {}
    using std::unique_ptr<T>::unique_ptr;
};

namespace tz {

template <typename T>
TzScopedPointer<T> as_scoped_ptr(T *ptr)
{
    return TzScopedPointer<T>(ptr);
}

} // namespace tz

#endif // TZSCOPEDPOINTER_HPP
