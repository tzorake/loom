#pragma once
#include <loom/tzabstractitemmodel.h>
#include <loom/tzscopedeventlistener.h>
#include <loom/tzclasshelpermacros.hpp>
#include <loom/tztreenode.h>

class TzTreeModelPrivate;

class TzTreeModel : public TzAbstractItemModel
{
	TZ_DECLARE_PRIVATE_D(d_ptr, TzTreeModel)
public:
	TzTreeModel(const TzTreeNodePtr& node);
	TzTreeModel(const std::vector<TzTreeNodePtr>& nodes);
	virtual ~TzTreeModel() override;

	TzTreeNodePtr invisibleRoot() const;

	virtual TzModelIndex index(int row, int column, const TzModelIndex& parent = TzModelIndex()) const override;
	virtual TzModelIndex parent(const TzModelIndex& child) const override;
	virtual int rowCount(const TzModelIndex& parent = TzModelIndex()) const override;
	virtual int columnCount(const TzModelIndex& parent = TzModelIndex()) const override;

	bool setData(const TzModelIndex& index, const std::any& value, int role = TzItemDataRole::Edit) override;
	std::any data(const TzModelIndex& index, int role = TzItemDataRole::Display) const override;

	virtual TzTreeNodePtr nodeForIndex(const TzModelIndex& index) const;
	virtual TzModelIndex indexForNode(const TzTreeNodePtr& node, int column = 0) const;

protected:
	TzTreeModel(TzTreeModelPrivate& dd);

private:
	void connectNode(const TzTreeNodePtr& node);
	void disconnectNode(const TzTreeNodePtr& node);
	void connectRecursively(const TzTreeNodePtr& node);
	void disconnectRecursively(const TzTreeNodePtr& node);
};
