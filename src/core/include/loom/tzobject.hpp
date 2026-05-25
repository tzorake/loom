#ifndef TZOBJECT_HPP
#define TZOBJECT_HPP

#include <loom/tzclasshelpermacros.hpp>

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

    TzObject *firstChild() const;
    TzObject *nextSibling() const;
    TzObject *previousSibling() const;

    std::vector<TzObject *> children() const;

    virtual bool event(TzEvent *event);

    void deleteLater();

protected:
    TzObject(TzObjectPrivate &dd, TzObject *parent = nullptr);

    std::unique_ptr<TzObjectPrivate> d_ptr;

    friend class TzCoreApplication;
};

#endif // TZOBJECT_HPP
