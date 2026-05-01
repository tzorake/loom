#include <event-loop/tzobject.hpp>
#include <event-loop/tzdeferreddeleteevent.hpp>
#include <event-loop/tzcoreapplication.hpp>

#include "tzobject_p.hpp"

TzObjectPrivate::TzObjectPrivate(TzObject *q)
    : q_ptr(q)
{
}

void TzObjectPrivate::unlinkFromParent()
{
    if (!parent)
        return;

    TzObjectPrivate *pd = parent->d_ptr.get();
    if (previousSibling)
        previousSibling->d_ptr->nextSibling = nextSibling;
    else
        pd->firstChild = nextSibling;

    if (nextSibling)
        nextSibling->d_ptr->previousSibling = previousSibling;

    parent      = nullptr;
    previousSibling = nullptr;
    nextSibling = nullptr;
}

void TzObjectPrivate::appendToParent(TzObject *newParent)
{
    TzObjectPrivate *pd = newParent->d_ptr.get();
    parent = newParent;

    if (!pd->firstChild) {
        pd->firstChild = q_ptr;
        return;
    }

    // Walk to last sibling
    TzObject *last = pd->firstChild;
    while (last->d_ptr->nextSibling)
        last = last->d_ptr->nextSibling;

    last->d_ptr->nextSibling = q_ptr;
    previousSibling = last;
}

TzObject::TzObject(TzObject *parent)
    : d_ptr(new TzObjectPrivate(this))
{
    if (parent)
        d_ptr->appendToParent(parent);
}

TzObject::~TzObject()
{
    if (TzCoreApplication *app = TzCoreApplication::instance())
        app->removePostedEvents(this);

    // Remove self from parent's children list without touching siblings
    d_ptr->unlinkFromParent();

    // Destroy all children; clear their parent pointer first so their
    // destructors don't attempt to unlink from us while we iterate.
    TzObject *child = d_ptr->firstChild;
    while (child) {
        TzObject *next = child->d_ptr->nextSibling;
        child->d_ptr->parent      = nullptr;
        child->d_ptr->previousSibling = nullptr;
        child->d_ptr->nextSibling = nullptr;
        delete child;
        child = next;
    }
    d_ptr->firstChild = nullptr;
}

void TzObject::setParent(TzObject *parent)
{
    if (d_ptr->parent == parent)
        return;

    d_ptr->unlinkFromParent();

    if (parent)
        d_ptr->appendToParent(parent);
}

TzObject *TzObject::parent() const
{
    return d_ptr->parent;
}

TzObject *TzObject::firstChild() const
{
    return d_ptr->firstChild;
}

TzObject *TzObject::nextSibling() const
{
    return d_ptr->nextSibling;
}

TzObject *TzObject::previousSibling() const
{
    return d_ptr->previousSibling;
}

std::vector<TzObject *> TzObject::children() const
{
    std::vector<TzObject *> result;
    for (TzObject *c = d_ptr->firstChild; c; c = c->d_ptr->nextSibling)
        result.push_back(c);
    return result;
}

bool TzObject::event(TzEvent * /*event*/)
{
    return false;
}

void TzObject::deleteLater()
{
    if (d_ptr->pendingDelete)
        return;
    d_ptr->pendingDelete = true;
    TzCoreApplication::postEvent(this, new TzDeferredDeleteEvent());
}
