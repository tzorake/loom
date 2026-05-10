#ifndef TZOBJECT_HPP
#define TZOBJECT_HPP

#include <event-loop/tzclasshelpermacros.hpp>

#include <vector>

class TzEvent;
class TzCoreApplication;
class TzObjectPrivate;

class TzObject
{
    TZ_DECLARE_PRIVATE(TzObject)
    TZ_DISABLE_COPY_MOVE(TzObject)
public:
    explicit TzObject(TzObject *parent = nullptr);
    virtual ~TzObject();

    void setParent(TzObject *parent);
    TzObject *parent() const;

    TzObject *firstChild()      const;
    TzObject *nextSibling()     const;
    TzObject *previousSibling() const;

    std::vector<TzObject *> children() const;

    virtual bool event(TzEvent *event);

    void deleteLater();

protected:
    // Used by subclasses that supply their own (derived) private object so only
    // one allocation is made for the entire class hierarchy.
    explicit TzObject(TzObjectPrivate &d, TzObject *parent = nullptr);

    // Owned by this class; deleted in ~TzObject().  Subclasses access it via
    // TZ_DECLARE_PRIVATE_D(d_ptr, SubClass) and the TZ_D() macro.
    TzObjectPrivate *d_ptr;

    friend class TzCoreApplication;
};

#endif // TZOBJECT_HPP
