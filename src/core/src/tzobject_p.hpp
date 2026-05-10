#ifndef TZOBJECT_P_HPP
#define TZOBJECT_P_HPP

#include <loom/tzclasshelpermacros.hpp>
#include <loom/tzobject.hpp>

class TzObjectPrivate
{
    TZ_DECLARE_PUBLIC(TzObject)
public:
    explicit TzObjectPrivate();
    virtual ~TzObjectPrivate();

    TzObject *q_ptr{ nullptr };
    TzObject *parent{ nullptr };
    TzObject *firstChild{ nullptr };
    TzObject *nextSibling{ nullptr };
    TzObject *previousSibling{ nullptr };
    bool pendingDelete{ false };

    void unlinkFromParent();
    void appendToParent(TzObject *newParent);
};

#endif // TZOBJECT_P_HPP
