#pragma once
#include <loom/tzscopedeventlistener.h>
#include <loom/tzeventemitter.h>
#include <loom/tzclasshelpermacros.hpp>
#include <memory>
#include <string>
#include <vector>
#include <any>
#include <optional>

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
    const std::string& name() const;

    Type type() const;
    bool isFolder() const;
    bool isItem() const;

    TzTreeNodePtr parent() const;
    int childCount() const;
    const std::vector<TzTreeNodePtr> &children() const;
    TzTreeNodePtr childAt(int index) const;

    int indexOf(const TzTreeNodePtr &child) const;

    bool addChild(const TzTreeNodePtr &child);
    bool insertChild(int index, const TzTreeNodePtr &child);
    bool removeChild(const TzTreeNodePtr &child);

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
