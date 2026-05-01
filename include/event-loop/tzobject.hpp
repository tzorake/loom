#ifndef TZOBJECT_HPP
#define TZOBJECT_HPP

#include <event-loop/tzclasshelpermacros.hpp>

#include <memory>
#include <vector>

class TzEvent;

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

private:
    std::unique_ptr<TzObjectPrivate> d_ptr;
};

#endif // TZOBJECT_HPP
