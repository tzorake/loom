#pragma once
#include "tztreemodel.hpp"
#include <tzabstractitemmodel_p.hpp>
#include <loom/tzscopedeventlistener.hpp>

class TzTreeModelPrivate : public TzAbstractItemModelPrivate
{
public:
	TzTreeModelPrivate();

	TzTreeNodePtr root;

	struct NodeConnections {
		TzScopedEventListener onChildAdded;
		TzScopedEventListener onChildRemoved;
		TzScopedEventListener onDataChanged;
		TzScopedEventListener onNameChanged;
	};
	std::unordered_map<TzTreeNode*, NodeConnections> connections;
};
