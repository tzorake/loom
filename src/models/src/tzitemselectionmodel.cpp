#include <loom/tzitemselectionmodel.hpp>
#include <tzitemselectionmodel_p.hpp>

#include <algorithm>
#include <functional>
#include <unordered_set>

// ── Helpers ──────────────────────────────────────────────────────────────────

static inline bool isSelectableAndEnabled(TzItemFlags flags)
{
    return flags.testFlag(TzItemFlag::ItemIsSelectable)
        && flags.testFlag(TzItemFlag::ItemIsEnabled);
}

template<typename ModelIndexContainer>
static void indexesFromRange(const TzItemSelectionRange &range, ModelIndexContainer &result)
{
    if (range.isValid() && range.model()) {
        const TzModelIndex topLeft = range.topLeft();
        const int bottom = range.bottom();
        const int right  = range.right();
        for (int row = topLeft.row(); row <= bottom; ++row) {
            const TzModelIndex columnLeader = topLeft.sibling(row, topLeft.column());
            for (int column = topLeft.column(); column <= right; ++column) {
                TzModelIndex index = columnLeader.sibling(row, column);
                if (isSelectableAndEnabled(range.model()->flags(index)))
                    result.push_back(index);
            }
        }
    }
}

template<typename ModelIndexContainer>
static ModelIndexContainer tzSelectionIndexes(const TzItemSelection &selection)
{
    ModelIndexContainer result;
    for (const auto &range : selection)
        indexesFromRange(range, result);
    return result;
}

static void rowLengthsFromRange(const TzItemSelectionRange &range,
                                std::vector<std::pair<TzPersistentModelIndex, unsigned int>> &result)
{
    if (range.isValid() && range.model()) {
        const TzModelIndex topLeft = range.topLeft();
        const int bottom = range.bottom();
        const unsigned int width = range.width();
        const int column = topLeft.column();
        for (int row = topLeft.row(); row <= bottom; ++row)
            result.emplace_back(topLeft.sibling(row, column), width);
    }
}

static std::vector<std::pair<TzPersistentModelIndex, unsigned int>>
tzSelectionPersistentRowLengths(const TzItemSelection &sel)
{
    std::vector<std::pair<TzPersistentModelIndex, unsigned int>> result;
    for (const TzItemSelectionRange &range : sel)
        rowLengthsFromRange(range, result);
    return result;
}

// ── TzItemSelectionRange ─────────────────────────────────────────────────────

bool TzItemSelectionRange::intersects(const TzItemSelectionRange &other) const
{
    if (model() != other.model())
        return false;
    if (top() > other.bottom() || bottom() < other.top())
        return false;
    if (left() > other.right() || right() < other.left())
        return false;
    if (parent() != other.parent())
        return false;
    return isValid() && other.isValid();
}

TzItemSelectionRange TzItemSelectionRange::intersected(const TzItemSelectionRange &other) const
{
    if (model() == other.model() && parent() == other.parent()) {
        TzModelIndex topLeft = model()->index(std::max(top(), other.top()),
                                              std::max(left(), other.left()),
                                              other.parent());
        TzModelIndex bottomRight = model()->index(std::min(bottom(), other.bottom()),
                                                  std::min(right(), other.right()),
                                                  other.parent());
        return TzItemSelectionRange(topLeft, bottomRight);
    }
    return TzItemSelectionRange();
}

bool TzItemSelectionRange::isEmpty() const
{
    if (!isValid() || !model())
        return true;
    for (int column = left(); column <= right(); ++column) {
        for (int row = top(); row <= bottom(); ++row) {
            TzModelIndex idx = model()->index(row, column, parent());
            if (isSelectableAndEnabled(model()->flags(idx)))
                return false;
        }
    }
    return true;
}

TzModelIndexList TzItemSelectionRange::indexes() const
{
    TzModelIndexList result;
    indexesFromRange(*this, result);
    return result;
}

// ── TzItemSelection ───────────────────────────────────────────────────────────

TzItemSelection::TzItemSelection(const TzModelIndex &topLeft, const TzModelIndex &bottomRight)
{
    select(topLeft, bottomRight);
}

void TzItemSelection::select(const TzModelIndex &topLeft, const TzModelIndex &bottomRight)
{
    if (!topLeft.isValid() || !bottomRight.isValid())
        return;
    if (topLeft.model() != bottomRight.model() || topLeft.parent() != bottomRight.parent()) {
        tzWarning("TzItemSelection: can't select indexes from different models or parents");
        return;
    }
    if (topLeft.row() > bottomRight.row() || topLeft.column() > bottomRight.column()) {
        int top    = std::min(topLeft.row(),    bottomRight.row());
        int bottom = std::max(topLeft.row(),    bottomRight.row());
        int left   = std::min(topLeft.column(), bottomRight.column());
        int right  = std::max(topLeft.column(), bottomRight.column());
        push_back(TzItemSelectionRange(topLeft.sibling(top, left),
                                       bottomRight.sibling(bottom, right)));
        return;
    }
    push_back(TzItemSelectionRange(topLeft, bottomRight));
}

bool TzItemSelection::contains(const TzModelIndex &index) const
{
    if (isSelectableAndEnabled(index.flags()))
        return std::any_of(begin(), end(), [&](const auto &range) { return range.contains(index); });
    return false;
}

TzModelIndexList TzItemSelection::indexes() const
{
    return tzSelectionIndexes<TzModelIndexList>(*this);
}

void TzItemSelection::merge(const TzItemSelection &other, TzItemSelectionModel::SelectionFlags command)
{
    if (other.empty() ||
        !(command & TzItemSelectionModel::Select ||
          command & TzItemSelectionModel::Deselect ||
          command & TzItemSelectionModel::Toggle))
        return;

    TzItemSelection newSelection;
    newSelection.reserve(other.size());
    TzItemSelection intersections;

    for (const auto &range : other) {
        if (!range.isValid())
            continue;
        newSelection.push_back(range);
        for (int t = 0; t < (int)size(); ++t) {
            if (range.intersects(at(t)))
                intersections.push_back(at(t).intersected(range));
        }
    }

    for (int i = 0; i < (int)intersections.size(); ++i) {
        for (int t = 0; t < (int)size();) {
            if (at(t).intersects(intersections.at(i))) {
                split(at(t), intersections.at(i), this);
                erase(begin() + t);
            } else {
                ++t;
            }
        }
        for (int n = 0; (command & TzItemSelectionModel::Toggle) && n < (int)newSelection.size();) {
            if (newSelection.at(n).intersects(intersections.at(i))) {
                split(newSelection.at(n), intersections.at(i), &newSelection);
                newSelection.erase(newSelection.begin() + n);
            } else {
                ++n;
            }
        }
    }

    if (!(command & TzItemSelectionModel::Deselect))
        insert(end(), newSelection.begin(), newSelection.end());
}

void TzItemSelection::split(const TzItemSelectionRange &range,
                             const TzItemSelectionRange &other,
                             TzItemSelection *result)
{
    if (range.parent() != other.parent() || range.model() != other.model())
        return;

    TzModelIndex parent = other.parent();
    int top         = range.top();
    int left        = range.left();
    int bottom      = range.bottom();
    int right       = range.right();
    int other_top   = other.top();
    int other_left  = other.left();
    int other_bottom = other.bottom();
    int other_right  = other.right();
    const TzAbstractItemModel *model = range.model();

    if (other_top > top) {
        result->push_back(TzItemSelectionRange(model->index(top, left, parent),
                                               model->index(other_top - 1, right, parent)));
        top = other_top;
    }
    if (other_bottom < bottom) {
        result->push_back(TzItemSelectionRange(model->index(other_bottom + 1, left, parent),
                                               model->index(bottom, right, parent)));
        bottom = other_bottom;
    }
    if (other_left > left) {
        result->push_back(TzItemSelectionRange(model->index(top, left, parent),
                                               model->index(bottom, other_left - 1, parent)));
        left = other_left;
    }
    if (other_right < right) {
        result->push_back(TzItemSelectionRange(model->index(top, other_right + 1, parent),
                                               model->index(bottom, right, parent)));
    }
}

// ── TzItemSelectionModelPrivate ───────────────────────────────────────────────

void TzItemSelectionModelPrivate::initModel(TzAbstractItemModel *m)
{
    TZ_Q(TzItemSelectionModel);
    if (model == m)
        return;

    if (model) {
        q->reset();
        disconnectModel();
    }

    model = m;

    if (m) {
        connections.push_back(m->events().on("rowsAboutToBeRemoved",
            [this](const TzModelIndex &parent, int start, int end) {
                rowsAboutToBeRemoved(parent, start, end);
            }));
        connections.push_back(m->events().on("columnsAboutToBeRemoved",
            [this](const TzModelIndex &parent, int start, int end) {
                columnsAboutToBeRemoved(parent, start, end);
            }));
        connections.push_back(m->events().on("rowsAboutToBeInserted",
            [this](const TzModelIndex &parent, int start, int end) {
                rowsAboutToBeInserted(parent, start, end);
            }));
        connections.push_back(m->events().on("columnsAboutToBeInserted",
            [this](const TzModelIndex &parent, int start, int end) {
                columnsAboutToBeInserted(parent, start, end);
            }));
        connections.push_back(m->events().on("rowsAboutToBeMoved",
            [this](const TzModelIndex &, int, int, const TzModelIndex &, int) {
                triggerLayoutToBeChanged();
            }));
        connections.push_back(m->events().on("columnsAboutToBeMoved",
            [this](const TzModelIndex &, int, int, const TzModelIndex &, int) {
                triggerLayoutToBeChanged();
            }));
        connections.push_back(m->events().on("rowsMoved",
            [this](const TzModelIndex &, int, int, const TzModelIndex &, int) {
                triggerLayoutChanged();
            }));
        connections.push_back(m->events().on("columnsMoved",
            [this](const TzModelIndex &, int, int, const TzModelIndex &, int) {
                triggerLayoutChanged();
            }));
        connections.push_back(m->events().on("layoutAboutToBeChanged",
            [this](const std::vector<TzPersistentModelIndex> &parents,
                   TzAbstractItemModel::LayoutChangeHint hint) {
                layoutAboutToBeChanged(parents, hint);
            }));
        connections.push_back(m->events().on("layoutChanged",
            [this](const std::vector<TzPersistentModelIndex> &parents,
                   TzAbstractItemModel::LayoutChangeHint hint) {
                layoutChanged(parents, hint);
            }));
        connections.push_back(m->events().on("modelReset",
            [q]() { q->reset(); }));
    }

    q->modelChanged(m);
}

void TzItemSelectionModelPrivate::disconnectModel()
{
    for (auto &c : connections)
        c.disconnect();
    connections.clear();
}

TzItemSelection TzItemSelectionModelPrivate::expandSelection(
    const TzItemSelection &selection, TzItemSelectionModel::SelectionFlags command) const
{
    if (selection.empty() && !((command & TzItemSelectionModel::Rows) ||
                                (command & TzItemSelectionModel::Columns)))
        return selection;

    TzItemSelection expanded;
    if (command & TzItemSelectionModel::Rows) {
        for (int i = 0; i < (int)selection.size(); ++i) {
            TzModelIndex parent = selection.at(i).parent();
            int colCount = model->columnCount(parent);
            TzModelIndex tl = model->index(selection.at(i).top(), 0, parent);
            TzModelIndex br = model->index(selection.at(i).bottom(), colCount - 1, parent);
            expanded.merge(TzItemSelection(tl, br), TzItemSelectionModel::Select);
        }
    }
    if (command & TzItemSelectionModel::Columns) {
        for (int i = 0; i < (int)selection.size(); ++i) {
            TzModelIndex parent = selection.at(i).parent();
            int rowCount = model->rowCount(parent);
            TzModelIndex tl = model->index(0, selection.at(i).left(), parent);
            TzModelIndex br = model->index(rowCount - 1, selection.at(i).right(), parent);
            expanded.merge(TzItemSelection(tl, br), TzItemSelectionModel::Select);
        }
    }
    return expanded;
}

void TzItemSelectionModelPrivate::rowsAboutToBeRemoved(const TzModelIndex &parent, int start, int end)
{
    TZ_Q(TzItemSelectionModel);
    finalize();

    // update current index
    if (currentIndex.isValid() && parent == currentIndex.parent()
        && currentIndex.row() >= start && currentIndex.row() <= end) {
        TzModelIndex old = currentIndex;
        if (start > 0) {
            currentIndex = model->index(start - 1, old.column(), parent);
        } else if (model && end < model->rowCount(parent) - 1) {
            currentIndex = model->index(end + 1, old.column(), parent);
        } else {
            currentIndex = TzModelIndex();
        }
        q->currentChanged(currentIndex, old);
        q->currentRowChanged(currentIndex, old);
        if (currentIndex.column() != old.column())
            q->currentColumnChanged(currentIndex, old);
    }

    TzItemSelection deselected;
    TzItemSelection newParts;
    bool indexesOfSelectionChanged = false;
    auto it = ranges.begin();
    while (it != ranges.end()) {
        if (it->topLeft().parent() != parent) {
            TzModelIndex itParent = it->topLeft().parent();
            while (itParent.isValid() && itParent.parent() != parent)
                itParent = itParent.parent();
            if (itParent.isValid() && start <= itParent.row() && itParent.row() <= end) {
                deselected.push_back(*it);
                it = ranges.erase(it);
            } else {
                if (itParent.isValid() && end < itParent.row())
                    indexesOfSelectionChanged = true;
                ++it;
            }
        } else if (start <= it->bottom() && it->bottom() <= end
                   && start <= it->top() && it->top() <= end) {
            deselected.push_back(*it);
            it = ranges.erase(it);
        } else if (start <= it->top() && it->top() <= end) {
            deselected.push_back(TzItemSelectionRange(it->topLeft(),
                                   model->index(end, it->right(), it->parent())));
            *it = TzItemSelectionRange(model->index(end + 1, it->left(), it->parent()),
                                        it->bottomRight());
            ++it;
        } else if (start <= it->bottom() && it->bottom() <= end) {
            deselected.push_back(TzItemSelectionRange(
                model->index(start, it->left(), it->parent()), it->bottomRight()));
            *it = TzItemSelectionRange(it->topLeft(),
                                        model->index(start - 1, it->right(), it->parent()));
            ++it;
        } else if (it->top() < start && end < it->bottom()) {
            const TzItemSelectionRange removedRange(
                model->index(start, it->left(), it->parent()),
                model->index(end,   it->right(), it->parent()));
            deselected.push_back(removedRange);
            TzItemSelection::split(*it, removedRange, &newParts);
            it = ranges.erase(it);
        } else if (end < it->top()) {
            indexesOfSelectionChanged = true;
            ++it;
        } else {
            ++it;
        }
    }
    ranges.insert(ranges.end(), newParts.begin(), newParts.end());

    if (!deselected.empty() || indexesOfSelectionChanged)
        q->selectionChanged(TzItemSelection(), deselected);
}

void TzItemSelectionModelPrivate::columnsAboutToBeRemoved(const TzModelIndex &parent, int start, int end)
{
    TZ_Q(TzItemSelectionModel);

    if (currentIndex.isValid() && parent == currentIndex.parent()
        && currentIndex.column() >= start && currentIndex.column() <= end) {
        TzModelIndex old = currentIndex;
        if (start > 0) {
            currentIndex = model->index(old.row(), start - 1, parent);
        } else if (model && end < model->columnCount() - 1) {
            currentIndex = model->index(old.row(), end + 1, parent);
        } else {
            currentIndex = TzModelIndex();
        }
        q->currentChanged(currentIndex, old);
        if (currentIndex.row() != old.row())
            q->currentRowChanged(currentIndex, old);
        q->currentColumnChanged(currentIndex, old);
    }

    TzModelIndex tl = model->index(0, start, parent);
    TzModelIndex br = model->index(model->rowCount(parent) - 1, end, parent);
    q->select(TzItemSelection(tl, br), TzItemSelectionModel::Deselect);
    finalize();
}

void TzItemSelectionModelPrivate::columnsAboutToBeInserted(const TzModelIndex &parent, int start, int /*end*/)
{
    finalize();
    std::vector<TzItemSelectionRange> splitRanges;
    auto it = ranges.begin();
    for (; it != ranges.end(); ) {
        const TzModelIndex &itParent = it->parent();
        if (it->isValid() && itParent == parent
            && it->left() < start && it->right() >= start) {
            TzModelIndex bottomMiddle = model->index(it->bottom(), start - 1, itParent);
            TzItemSelectionRange leftPart(it->topLeft(), bottomMiddle);
            TzModelIndex topMiddle = model->index(it->top(), start, itParent);
            TzItemSelectionRange rightPart(topMiddle, it->bottomRight());
            it = ranges.erase(it);
            splitRanges.push_back(leftPart);
            splitRanges.push_back(rightPart);
        } else {
            ++it;
        }
    }
    ranges.insert(ranges.end(), splitRanges.begin(), splitRanges.end());
}

void TzItemSelectionModelPrivate::rowsAboutToBeInserted(const TzModelIndex &parent, int start, int /*end*/)
{
    TZ_Q(TzItemSelectionModel);
    finalize();
    std::vector<TzItemSelectionRange> splitRanges;
    bool indexesOfSelectionChanged = false;
    auto it = ranges.begin();
    for (; it != ranges.end(); ) {
        const TzModelIndex &itParent = it->parent();
        if (it->isValid() && itParent == parent
            && it->top() < start && it->bottom() >= start) {
            TzModelIndex middleRight = model->index(start - 1, it->right(), itParent);
            TzItemSelectionRange topPart(it->topLeft(), middleRight);
            TzModelIndex middleLeft = model->index(start, it->left(), itParent);
            TzItemSelectionRange bottomPart(middleLeft, it->bottomRight());
            it = ranges.erase(it);
            splitRanges.push_back(topPart);
            splitRanges.push_back(bottomPart);
        } else if (it->isValid() && itParent == parent && it->top() >= start) {
            indexesOfSelectionChanged = true;
            ++it;
        } else {
            ++it;
        }
    }
    ranges.insert(ranges.end(), splitRanges.begin(), splitRanges.end());

    if (indexesOfSelectionChanged)
        q->selectionChanged(TzItemSelection(), TzItemSelection());
}

void TzItemSelectionModelPrivate::layoutAboutToBeChanged(
    const std::vector<TzPersistentModelIndex> &,
    TzAbstractItemModel::LayoutChangeHint hint)
{
    savedPersistentIndexes.clear();
    savedPersistentCurrentIndexes.clear();
    savedPersistentRowLengths.clear();
    savedPersistentCurrentRowLengths.clear();

    // optimization for when all indexes are selected
    if (ranges.empty() && currentSelection.size() == 1) {
        const TzItemSelectionRange &range = currentSelection.front();
        TzModelIndex parent = range.parent();
        tableRowCount = model->rowCount(parent);
        tableColCount = model->columnCount(parent);
        if (tableRowCount * tableColCount > 1000
            && range.top() == 0 && range.left() == 0
            && range.bottom() == tableRowCount - 1
            && range.right()  == tableColCount - 1) {
            tableSelected = true;
            tableParent = parent;
            return;
        }
    }
    tableSelected = false;

    if (hint == TzAbstractItemModel::LayoutChangeHint::VerticalSortHint) {
        savedPersistentRowLengths = tzSelectionPersistentRowLengths(ranges);
        savedPersistentCurrentRowLengths = tzSelectionPersistentRowLengths(currentSelection);
    } else {
        savedPersistentIndexes =
            tzSelectionIndexes<std::vector<TzPersistentModelIndex>>(ranges);
        savedPersistentCurrentIndexes =
            tzSelectionIndexes<std::vector<TzPersistentModelIndex>>(currentSelection);
    }
}

static bool persistentIndexLessThan(const TzPersistentModelIndex &i1,
                                    const TzPersistentModelIndex &i2)
{
    const TzModelIndex p1 = i1.parent();
    const TzModelIndex p2 = i2.parent();
    return p1 == p2 ? i1 < i2 : p1 < p2;
}

static TzItemSelection mergeIndexes(const std::vector<TzPersistentModelIndex> &indexes)
{
    TzItemSelection colSpans;
    int i = 0;
    while (i < (int)indexes.size()) {
        const TzPersistentModelIndex &tl = indexes.at(i);
        if (!tl.isValid()) { ++i; continue; }
        TzPersistentModelIndex br = tl;
        TzModelIndex brParent = br.parent();
        int brRow = br.row(), brColumn = br.column();
        while (++i < (int)indexes.size()) {
            const TzPersistentModelIndex &next = indexes.at(i);
            if (!next.isValid()) continue;
            const TzModelIndex nextParent = next.parent();
            if (nextParent == brParent && next.row() == brRow && next.column() == brColumn + 1) {
                br = next; brParent = nextParent; brRow = next.row(); brColumn = next.column();
            } else break;
        }
        colSpans.push_back(TzItemSelectionRange(tl, br));
    }
    TzItemSelection rowSpans;
    i = 0;
    while (i < (int)colSpans.size()) {
        TzModelIndex tl = colSpans.at(i).topLeft();
        TzModelIndex br = colSpans.at(i).bottomRight();
        TzModelIndex prevTl = tl;
        while (++i < (int)colSpans.size()) {
            TzModelIndex nextTl = colSpans.at(i).topLeft();
            TzModelIndex nextBr = colSpans.at(i).bottomRight();
            if (nextTl.parent() != tl.parent()) break;
            if (nextTl.column() == prevTl.column() && nextBr.column() == br.column()
                && nextTl.row() == prevTl.row() + 1 && nextBr.row() == br.row() + 1) {
                br = nextBr; prevTl = nextTl;
            } else break;
        }
        rowSpans.push_back(TzItemSelectionRange(tl, br));
    }
    return rowSpans;
}

static TzItemSelection mergeRowLengths(
    const std::vector<std::pair<TzPersistentModelIndex, unsigned int>> &rowLengths)
{
    if (rowLengths.empty())
        return TzItemSelection();
    TzItemSelection result;
    int i = 0;
    while (i < (int)rowLengths.size()) {
        const TzPersistentModelIndex &tl = rowLengths.at(i).first;
        if (!tl.isValid()) { ++i; continue; }
        TzPersistentModelIndex br = tl;
        const unsigned int length = rowLengths.at(i).second;
        while (++i < (int)rowLengths.size()) {
            const TzPersistentModelIndex &next = rowLengths.at(i).first;
            if (!next.isValid()) continue;
            const unsigned int nextLength = rowLengths.at(i).second;
            if (nextLength == length && next.row() == br.row() + 1
                && next.column() == br.column() && next.parent() == br.parent()) {
                br = next;
            } else break;
        }
        result.push_back(TzItemSelectionRange(tl, br.sibling(br.row(), br.column() + length - 1)));
    }
    return result;
}

void TzItemSelectionModelPrivate::layoutChanged(
    const std::vector<TzPersistentModelIndex> &,
    TzAbstractItemModel::LayoutChangeHint hint)
{
    if (tableSelected && tableColCount == model->columnCount(tableParent)
        && tableRowCount == model->rowCount(tableParent)) {
        ranges.clear();
        currentSelection.clear();
        TzModelIndex tl = model->index(0, 0, tableParent);
        TzModelIndex br = model->index(tableRowCount - 1, tableColCount - 1, tableParent);
        currentSelection.push_back(TzItemSelectionRange(tl, br));
        tableParent = TzModelIndex();
        tableSelected = false;
        return;
    }

    if ((hint != TzAbstractItemModel::LayoutChangeHint::VerticalSortHint
         && savedPersistentCurrentIndexes.empty() && savedPersistentIndexes.empty())
     || (hint == TzAbstractItemModel::LayoutChangeHint::VerticalSortHint
         && savedPersistentRowLengths.empty() && savedPersistentCurrentRowLengths.empty())) {
        return;
    }

    ranges.clear();
    currentSelection.clear();

    if (hint != TzAbstractItemModel::LayoutChangeHint::VerticalSortHint) {
        std::stable_sort(savedPersistentIndexes.begin(), savedPersistentIndexes.end(),
                         persistentIndexLessThan);
        std::stable_sort(savedPersistentCurrentIndexes.begin(), savedPersistentCurrentIndexes.end(),
                         persistentIndexLessThan);
        ranges = mergeIndexes(savedPersistentIndexes);
        currentSelection = mergeIndexes(savedPersistentCurrentIndexes);
        savedPersistentIndexes.clear();
        savedPersistentCurrentIndexes.clear();
    } else {
        std::stable_sort(savedPersistentRowLengths.begin(), savedPersistentRowLengths.end());
        std::stable_sort(savedPersistentCurrentRowLengths.begin(), savedPersistentCurrentRowLengths.end());
        ranges = mergeRowLengths(savedPersistentRowLengths);
        currentSelection = mergeRowLengths(savedPersistentCurrentRowLengths);
        savedPersistentRowLengths.clear();
        savedPersistentCurrentRowLengths.clear();
    }
}

void TzItemSelectionModelPrivate::modelDestroyed()
{
    model = nullptr;
    connections.clear();
}

TzItemSelectionModel::TzItemSelectionModel(TzAbstractItemModel *model)
    : d_ptr(new TzItemSelectionModelPrivate)
{
    d_ptr->q_ptr = this;
    d_func()->initModel(model);
}

TzItemSelectionModel::TzItemSelectionModel(TzItemSelectionModelPrivate &dd, TzAbstractItemModel *model)
    : d_ptr(&dd)
{
    d_ptr->q_ptr = this;
    dd.initModel(model);
}

TzItemSelectionModel::~TzItemSelectionModel()
{
    TZ_D(TzItemSelectionModel);
    d->disconnectModel();
    delete d_ptr;
}

void TzItemSelectionModel::selectionChanged(const TzItemSelection &selected, const TzItemSelection &deselected)
{
    TZ_D(TzItemSelectionModel);
    d->events.emit("selectionChanged", selected, deselected);
}

void TzItemSelectionModel::currentChanged(const TzModelIndex &current, const TzModelIndex &previous)
{
    TZ_D(TzItemSelectionModel);
    d->events.emit("currentChanged", current, previous);
}

void TzItemSelectionModel::currentRowChanged(const TzModelIndex &current, const TzModelIndex &previous)
{
    TZ_D(TzItemSelectionModel);
    d->events.emit("currentRowChanged", current, previous);
}

void TzItemSelectionModel::currentColumnChanged(const TzModelIndex &current, const TzModelIndex &previous)
{
    TZ_D(TzItemSelectionModel);
    d->events.emit("currentColumnChanged", current, previous);
}

void TzItemSelectionModel::modelChanged(TzAbstractItemModel *model)
{
    TZ_D(TzItemSelectionModel);
    d->events.emit("modelChanged", model);
}

void TzItemSelectionModel::select(const TzModelIndex &index, TzItemSelectionModel::SelectionFlags command)
{
    select(TzItemSelection(index, index), command);
}

void TzItemSelectionModel::select(const TzItemSelection &selection, TzItemSelectionModel::SelectionFlags command)
{
    TZ_D(TzItemSelectionModel);
    if (!d->model) {
        tzWarning("TzItemSelectionModel: no model set");
        return;
    }
    if (command == NoUpdate)
        return;

    // Remove invalid ranges
    std::erase_if(d->ranges, [](const TzItemSelectionRange &r) { return !r.isValid(); });

    TzItemSelection sel = selection;
    TzItemSelection old = d->ranges;
    old.merge(d->currentSelection, d->currentCommand);

    if (command & TzItemSelectionModel::Rows || command & Columns)
        sel = d->expandSelection(sel, command);

    if (command & Clear) {
        d->ranges.clear();
        d->currentSelection.clear();
    }
    if (!(command & Current))
        d->finalize();

    if (command & Toggle || command & Select || command & Deselect) {
        d->currentCommand = command;
        d->currentSelection = sel;
    }

    TzItemSelection newSelection = d->ranges;
    newSelection.merge(d->currentSelection, d->currentCommand);
    emitSelectionChanged(newSelection, old);
}

void TzItemSelectionModel::clear()
{
    clearSelection();
    clearCurrentIndex();
}

void TzItemSelectionModel::clearCurrentIndex()
{
    TZ_D(TzItemSelectionModel);
    TzModelIndex previous = d->currentIndex;
    d->currentIndex = TzModelIndex();
    if (previous.isValid()) {
        currentChanged(d->currentIndex, previous);
        currentRowChanged(d->currentIndex, previous);
        currentColumnChanged(d->currentIndex, previous);
    }
}

void TzItemSelectionModel::reset()
{
    clear();
}

void TzItemSelectionModel::clearSelection()
{
    TZ_D(TzItemSelectionModel);
    if (d->ranges.empty() && d->currentSelection.empty())
        return;
    select(TzItemSelection(), Clear);
}

void TzItemSelectionModel::setCurrentIndex(const TzModelIndex &index,
                                            TzItemSelectionModel::SelectionFlags command)
{
    TZ_D(TzItemSelectionModel);
    if (!d->model) {
        tzWarning("TzItemSelectionModel: no model set");
        return;
    }
    if (index == d->currentIndex) {
        if (command != NoUpdate)
            select(index, command);
        return;
    }
    TzPersistentModelIndex previous = d->currentIndex;
    d->currentIndex = index;
    if (command != NoUpdate)
        select(d->currentIndex, command);
    currentChanged(d->currentIndex, previous);
    if (d->currentIndex.row() != previous.row() || d->currentIndex.parent() != previous.parent())
        currentRowChanged(d->currentIndex, previous);
    if (d->currentIndex.column() != previous.column() || d->currentIndex.parent() != previous.parent())
        currentColumnChanged(d->currentIndex, previous);
}

TzModelIndex TzItemSelectionModel::currentIndex() const
{
    return static_cast<TzModelIndex>(d_func()->currentIndex);
}

bool TzItemSelectionModel::isSelected(const TzModelIndex &index) const
{
    TZ_D(const TzItemSelectionModel);
    if (d->model != index.model() || !index.isValid())
        return false;

    bool selected = std::any_of(d->ranges.begin(), d->ranges.end(),
                                [&](const auto &range) { return range.contains(index); });
    if (d->currentSelection.size()) {
        if ((d->currentCommand & Deselect) && selected)
            selected = !d->currentSelection.contains(index);
        else if (d->currentCommand & Toggle)
            selected ^= d->currentSelection.contains(index);
        else if ((d->currentCommand & Select) && !selected)
            selected = d->currentSelection.contains(index);
    }
    if (selected)
        return isSelectableAndEnabled(d->model->flags(index));
    return false;
}

bool TzItemSelectionModel::isRowSelected(int row, const TzModelIndex &parent) const
{
    TZ_D(const TzItemSelectionModel);
    if (!d->model)
        return false;
    if (parent.isValid() && d->model != parent.model())
        return false;

    if (d->currentCommand & Deselect) {
        for (const auto &sel : d->currentSelection) {
            if (row >= sel.top() && row <= sel.bottom() && parent == sel.parent())
                return false;
        }
    }
    if (d->currentCommand & Toggle) {
        for (const auto &sel : d->currentSelection) {
            if (row >= sel.top() && row <= sel.bottom()) {
                for (const auto &range : d->ranges) {
                    if (row >= range.top() && row <= range.bottom()
                        && sel.intersected(range).isValid())
                        return false;
                }
            }
        }
    }

    const int colCount = d->model->columnCount(parent);
    int unselectable = 0;
    for (int column = 0; column < colCount; ++column) {
        if (!isSelectableAndEnabled(d->model->index(row, column, parent).flags())) {
            ++unselectable;
            continue;
        }
        bool found = false;
        for (const auto &range : d->currentSelection) {
            if (range.contains(row, column, parent)) { column = range.right(); found = true; break; }
        }
        if (!found) {
            for (const auto &range : d->ranges) {
                if (range.contains(row, column, parent)) { column = range.right(); found = true; break; }
            }
        }
        if (!found)
            return false;
    }
    return unselectable < colCount;
}

bool TzItemSelectionModel::isColumnSelected(int column, const TzModelIndex &parent) const
{
    TZ_D(const TzItemSelectionModel);
    if (!d->model)
        return false;
    if (parent.isValid() && d->model != parent.model())
        return false;

    if (d->currentCommand & Deselect) {
        for (const auto &sel : d->currentSelection) {
            if (column >= sel.left() && column <= sel.right() && parent == sel.parent())
                return false;
        }
    }
    if (d->currentCommand & Toggle) {
        for (const auto &sel : d->currentSelection) {
            if (column >= sel.left() && column <= sel.right()) {
                for (const auto &range : d->ranges) {
                    if (column >= range.left() && column <= range.right()
                        && sel.intersected(range).isValid())
                        return false;
                }
            }
        }
    }

    const int rowCount = d->model->rowCount(parent);
    int unselectable = 0;
    for (int row = 0; row < rowCount; ++row) {
        if (!isSelectableAndEnabled(d->model->index(row, column, parent).flags())) {
            ++unselectable;
            continue;
        }
        bool found = false;
        for (const auto &range : d->currentSelection) {
            if (range.contains(row, column, parent)) { row = range.bottom(); found = true; break; }
        }
        if (!found) {
            for (const auto &range : d->ranges) {
                if (range.contains(row, column, parent)) { row = range.bottom(); found = true; break; }
            }
        }
        if (!found)
            return false;
    }
    return unselectable < rowCount;
}

bool TzItemSelectionModel::rowIntersectsSelection(int row, const TzModelIndex &parent) const
{
    TZ_D(const TzItemSelectionModel);
    if (!d->model)
        return false;
    if (parent.isValid() && d->model != parent.model())
        return false;

    TzItemSelection sel = d->ranges;
    sel.merge(d->currentSelection, d->currentCommand);
    if (sel.empty())
        return false;

    for (const TzItemSelectionRange &range : sel) {
        if (range.top() <= row && range.bottom() >= row) {
            for (int j = range.left(); j <= range.right(); ++j) {
                if (isSelectableAndEnabled(d->model->index(row, j, parent).flags()))
                    return true;
            }
        }
    }
    return false;
}

bool TzItemSelectionModel::columnIntersectsSelection(int column, const TzModelIndex &parent) const
{
    TZ_D(const TzItemSelectionModel);
    if (!d->model)
        return false;
    if (parent.isValid() && d->model != parent.model())
        return false;

    TzItemSelection sel = d->ranges;
    sel.merge(d->currentSelection, d->currentCommand);
    if (sel.empty())
        return false;

    for (const TzItemSelectionRange &range : sel) {
        if (range.left() <= column && range.right() >= column) {
            for (int j = range.top(); j <= range.bottom(); ++j) {
                if (isSelectableAndEnabled(d->model->index(j, column, parent).flags()))
                    return true;
            }
        }
    }
    return false;
}

static inline bool selectionIsEmpty(const TzItemSelection &selection)
{
    return std::all_of(selection.begin(), selection.end(),
                       [](const TzItemSelectionRange &r) { return r.isEmpty(); });
}

bool TzItemSelectionModel::hasSelection() const
{
    TZ_D(const TzItemSelectionModel);
    if (d->currentCommand & (TzItemSelectionModel::Toggle | TzItemSelectionModel::Deselect)) {
        TzItemSelection sel = d->ranges;
        sel.merge(d->currentSelection, d->currentCommand);
        return !selectionIsEmpty(sel);
    }
    return !(selectionIsEmpty(d->ranges) && selectionIsEmpty(d->currentSelection));
}

TzModelIndexList TzItemSelectionModel::selectedIndexes() const
{
    TZ_D(const TzItemSelectionModel);
    TzItemSelection selected = d->ranges;
    selected.merge(d->currentSelection, d->currentCommand);
    return selected.indexes();
}

TzModelIndexList TzItemSelectionModel::selectedRows(int column) const
{
    TzModelIndexList indexes;
    std::unordered_set<int> rowsSeen;

    const TzItemSelection sel = selection();
    for (const auto &range : sel) {
        TzModelIndex parent = range.parent();
        for (int row = range.top(); row <= range.bottom(); ++row) {
            if (rowsSeen.insert(row).second && isRowSelected(row, parent))
                indexes.push_back(model()->index(row, column, parent));
        }
    }
    return indexes;
}

TzModelIndexList TzItemSelectionModel::selectedColumns(int row) const
{
    TzModelIndexList indexes;
    std::unordered_set<int> columnsSeen;

    const TzItemSelection sel = selection();
    for (const auto &range : sel) {
        TzModelIndex parent = range.parent();
        for (int column = range.left(); column <= range.right(); ++column) {
            if (columnsSeen.insert(column).second && isColumnSelected(column, parent))
                indexes.push_back(model()->index(row, column, parent));
        }
    }
    return indexes;
}

const TzItemSelection TzItemSelectionModel::selection() const
{
    TZ_D(const TzItemSelectionModel);
    TzItemSelection selected = d->ranges;
    selected.merge(d->currentSelection, d->currentCommand);
    std::erase_if(selected, [](const TzItemSelectionRange &r) { return !r.isValid(); });
    return selected;
}

TzEventEmitter &TzItemSelectionModel::events()
{
    TZ_D(TzItemSelectionModel);
    return d->events;
}

const TzEventEmitter &TzItemSelectionModel::events() const
{
    TZ_D(const TzItemSelectionModel);
    return d->events;
}

void TzItemSelectionModel::setModel(TzAbstractItemModel *model)
{
    TZ_D(TzItemSelectionModel);
    if (d->model == model)
        return;
    d->initModel(model);
}

TzAbstractItemModel *TzItemSelectionModel::model()
{
    TZ_D(TzItemSelectionModel);
    return d->model;
}

const TzAbstractItemModel *TzItemSelectionModel::model() const
{
    TZ_D(const TzItemSelectionModel);
    return d->model;
}

void TzItemSelectionModel::emitSelectionChanged(const TzItemSelection &newSelection, const TzItemSelection &oldSelection)
{
    if ((oldSelection.empty() && newSelection.empty()) || oldSelection == newSelection)
        return;
    if (oldSelection.empty() || newSelection.empty()) {
        selectionChanged(newSelection, oldSelection);
        return;
    }

    TzItemSelection deselected = oldSelection;
    TzItemSelection selected   = newSelection;

    // remove equal ranges
    for (int o = 0; o < (int)deselected.size(); ) {
        bool advance = true;
        for (int s = 0; s < (int)selected.size() && o < (int)deselected.size(); ) {
            if (deselected.at(o) == selected.at(s)) {
                deselected.erase(deselected.begin() + o);
                selected.erase(selected.begin() + s);
                advance = false;
            } else {
                ++s;
            }
        }
        if (advance)
            ++o;
    }

    TzItemSelection intersections;
    for (int o = 0; o < (int)deselected.size(); ++o)
        for (int s = 0; s < (int)selected.size(); ++s)
            if (deselected.at(o).intersects(selected.at(s)))
                intersections.push_back(deselected.at(o).intersected(selected.at(s)));

    for (int i = 0; i < (int)intersections.size(); ++i) {
        for (int o = 0; o < (int)deselected.size(); ) {
            if (deselected.at(o).intersects(intersections.at(i))) {
                TzItemSelection::split(deselected.at(o), intersections.at(i), &deselected);
                deselected.erase(deselected.begin() + o);
            } else {
                ++o;
            }
        }
        for (int s = 0; s < (int)selected.size(); ) {
            if (selected.at(s).intersects(intersections.at(i))) {
                TzItemSelection::split(selected.at(s), intersections.at(i), &selected);
                selected.erase(selected.begin() + s);
            } else {
                ++s;
            }
        }
    }

    if (!selected.empty() || !deselected.empty())
        selectionChanged(selected, deselected);
}
