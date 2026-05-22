#ifndef TZITEMSELECTIONMODEL_P_HPP
#define TZITEMSELECTIONMODEL_P_HPP

#include <loom/tzitemselectionmodel.hpp>
#include <loom/tzeventlistener.hpp>
#include <vector>

class TzItemSelectionModelPrivate
{
    TZ_DECLARE_PUBLIC(TzItemSelectionModel)
public:
    TzItemSelectionModelPrivate()
        : q_ptr(nullptr)
        , model(nullptr)
        , currentCommand(TzItemSelectionModel::NoUpdate)
        , tableSelected(false)
        , tableColCount(0)
        , tableRowCount(0)
    {}
    ~TzItemSelectionModelPrivate() = default;

    TzItemSelectionModel *q_ptr;
    TzAbstractItemModel *model;

    TzItemSelection expandSelection(const TzItemSelection &selection,
                                    TzItemSelectionModel::SelectionFlags command) const;

    void initModel(TzAbstractItemModel *m);
    void disconnectModel();

    void rowsAboutToBeRemoved(const TzModelIndex &parent, int start, int end);
    void columnsAboutToBeRemoved(const TzModelIndex &parent, int start, int end);
    void rowsAboutToBeInserted(const TzModelIndex &parent, int start, int end);
    void columnsAboutToBeInserted(const TzModelIndex &parent, int start, int end);
    void layoutAboutToBeChanged(const std::vector<TzPersistentModelIndex> &parents,
                                TzAbstractItemModel::LayoutChangeHint hint);
    void triggerLayoutToBeChanged()
    {
        layoutAboutToBeChanged({}, TzAbstractItemModel::LayoutChangeHint::NoLayoutChangeHint);
    }
    void layoutChanged(const std::vector<TzPersistentModelIndex> &parents,
                       TzAbstractItemModel::LayoutChangeHint hint);
    void triggerLayoutChanged()
    {
        layoutChanged({}, TzAbstractItemModel::LayoutChangeHint::NoLayoutChangeHint);
    }
    void modelDestroyed();

    inline void remove(const std::vector<TzItemSelectionRange> &r)
    {
        for (const auto &item : r) {
            auto it = std::find(ranges.begin(), ranges.end(), item);
            if (it != ranges.end())
                ranges.erase(it);
        }
    }

    inline void finalize()
    {
        ranges.merge(currentSelection, currentCommand);
        if (!currentSelection.empty())
            currentSelection.clear();
    }

    void setModel(TzAbstractItemModel *mod) { q_func()->setModel(mod); }
    void modelChangedForwarder(TzAbstractItemModel *mod) { q_func()->modelChanged(mod); }

    TzEventEmitter events;

    TzItemSelection ranges;
    TzItemSelection currentSelection;
    TzPersistentModelIndex currentIndex;
    TzItemSelectionModel::SelectionFlags currentCommand;
    std::vector<TzPersistentModelIndex> savedPersistentIndexes;
    std::vector<TzPersistentModelIndex> savedPersistentCurrentIndexes;
    std::vector<std::pair<TzPersistentModelIndex, unsigned int>> savedPersistentRowLengths;
    std::vector<std::pair<TzPersistentModelIndex, unsigned int>> savedPersistentCurrentRowLengths;
    // optimization when all indexes are selected
    bool tableSelected;
    TzPersistentModelIndex tableParent;
    int tableColCount, tableRowCount;
    std::vector<TzEventListener> connections;
};

#endif // TZITEMSELECTIONMODEL_P_HPP
