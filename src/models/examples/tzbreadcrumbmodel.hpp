#pragma once
#include <loom/tztreenode.h>
#include <loom/tzeventemitter.h>
#include <vector>

class TzBreadcrumbModel
{
public:
    explicit TzBreadcrumbModel(const TzTreeNodePtr& root);

    TzTreeNodePtr root() const;
    TzTreeNodePtr currentNode() const;

    std::vector<TzTreeNodePtr> path() const;
    std::vector<TzTreeNodePtr> siblingsOf(const TzTreeNodePtr& node) const;

    bool navigateTo(const TzTreeNodePtr& node);
    bool navigateUp();
    void navigateToRoot();

    TzEventEmitter& events();
    const TzEventEmitter& events() const;

private:
    bool isReachable(const TzTreeNodePtr& node) const;

    TzTreeNodePtr  m_root;
    TzTreeNodePtr  m_current;
    TzEventEmitter m_events;
};
