#ifndef TZABSTRACTPROXYMODEL_HPP
#define TZABSTRACTPROXYMODEL_HPP

#include <loom/tzabstractitemmodel.hpp>
#include <loom/tzitemselection.hpp>

class TzAbstractProxyModelPrivate;

class TzAbstractProxyModel : public TzAbstractItemModel
{
public:
    TzAbstractProxyModel();
    ~TzAbstractProxyModel();

    virtual void setSourceModel(TzAbstractItemModel *sourceModel);
    TzAbstractItemModel *sourceModel() const;

    virtual TzModelIndex mapToSource(const TzModelIndex &proxyIndex) const = 0;
    virtual TzModelIndex mapFromSource(const TzModelIndex &sourceIndex) const = 0;

    virtual TzItemSelection mapSelectionToSource(const TzItemSelection &selection) const;
    virtual TzItemSelection mapSelectionFromSource(const TzItemSelection &selection) const;

    bool submit() override;
    void revert() override;

    std::any data(const TzModelIndex &proxyIndex, int role = TzItemDataRole::DisplayRole) const override;
    std::any headerData(int section, TzOrientation orientation, int role = TzItemDataRole::DisplayRole) const override;
    TzItemFlags flags(const TzModelIndex &index) const override;

    bool setData(const TzModelIndex &index, const std::any &value, int role = TzItemDataRole::EditRole) override;
    bool setHeaderData(int section, TzOrientation orientation, const std::any &value, int role = TzItemDataRole::EditRole) override;

    TzModelIndex buddy(const TzModelIndex &index) const override;
    bool canFetchMore(const TzModelIndex &parent) const override;
    void fetchMore(const TzModelIndex &parent) override;
    void sort(int column, TzSortOrder order = TzSortOrder::AscendingOrder) override;
    bool hasChildren(const TzModelIndex &parent = TzModelIndex()) const override;
    TzModelIndex sibling(int row, int column, const TzModelIndex &idx) const override;

    std::unordered_map<int, std::string> roleNames() const override;

// TZ_SIGNALS.begin — subscribe via proxy.emitter.on("sourceModelChanged", cb)
private:
    void sourceModelChanged();
// TZ_SIGNALS.end

protected:
    TzModelIndex createSourceIndex(int row, int col, void *internalPtr) const;
    TzAbstractProxyModel(TzAbstractProxyModelPrivate &dd);

private:
    TZ_DECLARE_PRIVATE(TzAbstractProxyModel)
    TZ_DISABLE_COPY(TzAbstractProxyModel)
};

#endif // TZABSTRACTPROXYMODEL_HPP
