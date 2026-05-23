#pragma once
#include <loom/tzclasshelpermacros.hpp>
#include <loom/tzeventemitter.hpp>
#include <loom/tzscopedeventlistener.hpp>
#include <any>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <ranges>

class TzTreeNodePrivate;
class TzTreeNode;

using TzTreeNodePtr = std::shared_ptr<TzTreeNode>;

class TzTreeNode : public std::enable_shared_from_this<TzTreeNode>
{
    TZ_DECLARE_PRIVATE(TzTreeNode)
    class PrivateToken {};
public:
    enum Type { Item, Folder };

    static TzTreeNodePtr create(const std::string &name, Type type = Type::Item);

    TzTreeNode(const std::string &name, Type type, PrivateToken);

    bool setName(const std::string &newName);
    const std::string &name() const;

    Type type() const;
    bool isFolder() const;
    bool isItem() const;

    int childCount() const;
    const std::vector<TzTreeNodePtr> &children() const;
    TzTreeNodePtr childAt(int index) const;

    int indexOf(const TzTreeNodePtr &child) const;

    bool setParent(const TzTreeNodePtr &newParent);
    TzTreeNodePtr parent() const;

    void setData(const std::any &value);
    const std::any &data() const;

    template<typename T>
    std::optional<T> unwrap() const;

    TzEventEmitter &events();
    const TzEventEmitter &events() const;

    static TzTreeNodePtr createFolder(const std::string &name);
    static TzTreeNodePtr createItem(const std::string &name);

private:
    std::unique_ptr<TzTreeNodePrivate> d_ptr;
};

template<typename T>
inline std::optional<T> TzTreeNode::unwrap() const
{
    if (const T *ptr = std::any_cast<T>(&data()))
        return *ptr;
    return std::nullopt;
}

class TzTreeNodePrivate
{
public:
    TzTreeNodePrivate(const std::string &name, TzTreeNode::Type type);

    static TzTreeNodePrivate *get(TzTreeNode &q);
    static const TzTreeNodePrivate *get(const TzTreeNode &q);

    std::string name;
    TzTreeNode::Type type;
    std::any data;
    std::weak_ptr<TzTreeNode> parent;
    std::vector<TzTreeNodePtr> children;
    TzEventEmitter events;
};

inline TzTreeNodePrivate::TzTreeNodePrivate(const std::string &name, TzTreeNode::Type type)
    : name(name)
    , type(type)
{}

inline TzTreeNodePrivate *TzTreeNodePrivate::get(TzTreeNode &q)
{
    return q.d_ptr.get();
}

inline const TzTreeNodePrivate *TzTreeNodePrivate::get(const TzTreeNode &q)
{
    return q.d_ptr.get();
}

inline TzTreeNodePtr TzTreeNode::create(const std::string &name, Type type)
{
    return std::make_shared<TzTreeNode>(name, type, PrivateToken{});
}

inline TzTreeNode::TzTreeNode(const std::string &name, Type type, PrivateToken)
    : d_ptr(new TzTreeNodePrivate(name, type))
{}

inline bool TzTreeNode::setName(const std::string &newName)
{
    TZ_D(TzTreeNode);
    if (d->name == newName)
        return false;
    d->name = newName;
    d->events.emit("nameChanged", d->name);
    return true;
}

inline const std::string &TzTreeNode::name() const
{
    TZ_D(const TzTreeNode);
    return d->name;
}

inline TzTreeNode::Type TzTreeNode::type() const
{
    TZ_D(const TzTreeNode);
    return d->type;
}

inline bool TzTreeNode::isFolder() const
{
    TZ_D(const TzTreeNode);
    return d->type == TzTreeNode::Type::Folder;
}

inline bool TzTreeNode::isItem() const
{
    TZ_D(const TzTreeNode);
    return d->type == TzTreeNode::Type::Item;
}

inline int TzTreeNode::childCount() const
{
    TZ_D(const TzTreeNode);
    return static_cast<int>(d->children.size());
}

inline const std::vector<TzTreeNodePtr> &TzTreeNode::children() const
{
    TZ_D(const TzTreeNode);
    return d->children;
}

inline TzTreeNodePtr TzTreeNode::childAt(int index) const
{
    TZ_D(const TzTreeNode);
    if (index < 0 || static_cast<std::size_t>(index) >= d->children.size())
        return nullptr;
    return d->children[static_cast<std::size_t>(index)];
}

inline int TzTreeNode::indexOf(const TzTreeNodePtr &child) const
{
    TZ_D(const TzTreeNode);
    auto it = std::ranges::find(d->children, child);
    return it != d->children.end() ? static_cast<int>(std::distance(d->children.begin(), it)) : -1;
}

inline bool TzTreeNode::setParent(const TzTreeNodePtr &newParent)
{
    TZ_D(TzTreeNode);

    if (newParent == d->parent.lock())
        return false;

    TzTreeNodePtr self = shared_from_this();
    if (TzTreeNodePtr oldParent = d->parent.lock()) {
        std::vector<TzTreeNodePtr> &siblings = TzTreeNodePrivate::get(*oldParent)->children;
        auto it = std::ranges::find(siblings, self);
        if (it != siblings.end()) {
            int idx = static_cast<int>(std::distance(siblings.begin(), it));
            siblings.erase(it);
            oldParent->events().emit("childRemoved", self, idx);
        }
        d->parent.reset();
    }

    d->events.emit("parentAboutToChange", newParent);

    if (newParent) {
        std::vector<TzTreeNodePtr> &newChildren = TzTreeNodePrivate::get(*newParent)->children;
        newChildren.push_back(self);
        int insertIdx = static_cast<int>(newChildren.size() - 1);
        d->parent = newParent;
        newParent->events().emit("childAdded", self, insertIdx);
    }

    d->events.emit("parentChanged", newParent);

    return true;
}

inline TzTreeNodePtr TzTreeNode::parent() const
{
    TZ_D(const TzTreeNode);
    return d->parent.lock();
}

inline void TzTreeNode::setData(const std::any &value)
{
    TZ_D(TzTreeNode);
    d->data = value;
    d->events.emit("dataChanged");
}

inline const std::any &TzTreeNode::data() const
{
    TZ_D(const TzTreeNode);
    return d->data;
}

inline TzEventEmitter &TzTreeNode::events()
{
    TZ_D(TzTreeNode);
    return d->events;
}

inline const TzEventEmitter &TzTreeNode::events() const
{
    TZ_D(const TzTreeNode);
    return d->events;
}

inline TzTreeNodePtr TzTreeNode::createFolder(const std::string &name)
{
    return TzTreeNode::create(name, TzTreeNode::Folder);
}

inline TzTreeNodePtr TzTreeNode::createItem(const std::string &name)
{
    return TzTreeNode::create(name, TzTreeNode::Item);
}
