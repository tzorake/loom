#ifndef TZSCOPEDPOINTER_HPP
#define TZSCOPEDPOINTER_HPP

#include <loom/tzclasshelpermacros.hpp>

#include <functional>
#include <memory>

template<typename T>
class TzScopedPointer : public std::unique_ptr<T>
{
public:
    TzScopedPointer(T *pointer = nullptr)
        : std::unique_ptr<T>(pointer)
    {}
    using std::unique_ptr<T>::unique_ptr;
};

template<typename T>
TzScopedPointer<T> tzScopedPointer(T *ptr)
{
    return TzScopedPointer<T>(ptr);
}

#endif // TZSCOPEDPOINTER_HPP
