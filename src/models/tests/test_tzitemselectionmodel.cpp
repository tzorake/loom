#include <gtest/gtest.h>
#include <loom/tzitemselectionmodel.hpp>
#include <loom/tzscopedeventlistener.hpp>

class SelTestModel : public TzAbstractListModel
{
public:
    explicit SelTestModel(int rows = 0) : m_rows(rows) {}

    int rowCount(const TzModelIndex &parent = TzModelIndex()) const override
    { return parent.isValid() ? 0 : m_rows; }

    std::any data(const TzModelIndex &index, int /*role*/) const override
    { return index.isValid() && index.row() < m_rows ? std::any{index.row()} : std::any{}; }

    void appendRows(int n)
    {
        beginInsertRows(TzModelIndex(), m_rows, m_rows + n - 1);
        m_rows += n;
        endInsertRows();
    }

    void removeRows(int first, int last)
    {
        beginRemoveRows(TzModelIndex(), first, last);
        m_rows -= (last - first + 1);
        endRemoveRows();
    }

private:
    int m_rows;
};

TEST(TzItemSelectionModel, DefaultConstruction)
{
    TzItemSelectionModel sm;
    EXPECT_EQ(sm.model(), nullptr);
    EXPECT_FALSE(sm.hasSelection());
    EXPECT_FALSE(sm.currentIndex().isValid());
}

TEST(TzItemSelectionModel, ConstructionWithModel)
{
    SelTestModel source(4);
    TzItemSelectionModel sm(&source);
    EXPECT_EQ(sm.model(), &source);
    EXPECT_FALSE(sm.hasSelection());
}

TEST(TzItemSelectionModel, SetModel)
{
    SelTestModel s1(3), s2(5);
    TzItemSelectionModel sm(&s1);

    sm.setModel(&s2);
    EXPECT_EQ(sm.model(), &s2);
}

TEST(TzItemSelectionModel, SetSameModelNoSignal)
{
    SelTestModel source(3);
    TzItemSelectionModel sm(&source);

    int fired = 0;
    sm.events().on("modelChanged", [&](TzAbstractItemModel *) { ++fired; });

    sm.setModel(&source);
    EXPECT_EQ(fired, 0);
}

TEST(TzItemSelectionModel, SetModelNull)
{
    SelTestModel source(3);
    TzItemSelectionModel sm(&source);

    sm.setModel(nullptr);
    EXPECT_EQ(sm.model(), nullptr);
}

TEST(TzItemSelectionModel, SignalModelChanged)
{
    SelTestModel s1(1), s2(1);
    TzItemSelectionModel sm(&s1);

    int fired = 0;
    TzAbstractItemModel *received = nullptr;
    sm.events().on("modelChanged", [&](TzAbstractItemModel *m) {
        ++fired;
        received = m;
    });

    sm.setModel(&s2);
    EXPECT_EQ(fired, 1);
    EXPECT_EQ(received, &s2);
}

TEST(TzItemSelectionModel, SelectSingleIndex)
{
    SelTestModel source(5);
    TzItemSelectionModel sm(&source);

    TzModelIndex idx = source.index(2, 0);
    sm.select(idx, TzItemSelectionModel::Select);

    EXPECT_TRUE(sm.isSelected(idx));
    EXPECT_TRUE(sm.hasSelection());
    EXPECT_FALSE(sm.isSelected(source.index(0, 0)));
    EXPECT_FALSE(sm.isSelected(source.index(1, 0)));
}

TEST(TzItemSelectionModel, SelectMultipleIndexes)
{
    SelTestModel source(5);
    TzItemSelectionModel sm(&source);

    sm.select(source.index(0, 0), TzItemSelectionModel::Select);
    sm.select(source.index(3, 0), TzItemSelectionModel::Select);

    TzModelIndexList sel = sm.selectedIndexes();
    EXPECT_EQ(sel.size(), 2u);
}

TEST(TzItemSelectionModel, ClearAndSelectReplacesPrevious)
{
    SelTestModel source(5);
    TzItemSelectionModel sm(&source);

    sm.select(source.index(0, 0), TzItemSelectionModel::Select);
    sm.select(source.index(3, 0), TzItemSelectionModel::ClearAndSelect);

    EXPECT_FALSE(sm.isSelected(source.index(0, 0)));
    EXPECT_TRUE(sm.isSelected(source.index(3, 0)));
}

TEST(TzItemSelectionModel, Deselect)
{
    SelTestModel source(5);
    TzItemSelectionModel sm(&source);

    sm.select(source.index(1, 0), TzItemSelectionModel::Select);
    EXPECT_TRUE(sm.isSelected(source.index(1, 0)));

    sm.select(source.index(1, 0), TzItemSelectionModel::Deselect);
    EXPECT_FALSE(sm.isSelected(source.index(1, 0)));
    EXPECT_FALSE(sm.hasSelection());
}

TEST(TzItemSelectionModel, Toggle)
{
    SelTestModel source(5);
    TzItemSelectionModel sm(&source);

    TzModelIndex idx = source.index(2, 0);
    sm.select(idx, TzItemSelectionModel::Toggle);
    EXPECT_TRUE(sm.isSelected(idx));

    sm.select(idx, TzItemSelectionModel::Toggle);
    EXPECT_FALSE(sm.isSelected(idx));
}

TEST(TzItemSelectionModel, SelectRange)
{
    SelTestModel source(5);
    TzItemSelectionModel sm(&source);

    TzItemSelection sel(source.index(1, 0), source.index(3, 0));
    sm.select(sel, TzItemSelectionModel::Select);

    EXPECT_FALSE(sm.isSelected(source.index(0, 0)));
    EXPECT_TRUE(sm.isSelected(source.index(1, 0)));
    EXPECT_TRUE(sm.isSelected(source.index(2, 0)));
    EXPECT_TRUE(sm.isSelected(source.index(3, 0)));
    EXPECT_FALSE(sm.isSelected(source.index(4, 0)));
}

TEST(TzItemSelectionModel, ClearSelection)
{
    SelTestModel source(3);
    TzItemSelectionModel sm(&source);

    sm.select(source.index(0, 0), TzItemSelectionModel::Select);
    sm.select(source.index(1, 0), TzItemSelectionModel::Select);
    sm.clearSelection();

    EXPECT_FALSE(sm.hasSelection());
    EXPECT_EQ(sm.selectedIndexes().size(), 0u);
}

TEST(TzItemSelectionModel, Clear)
{
    SelTestModel source(3);
    TzItemSelectionModel sm(&source);

    sm.select(source.index(0, 0), TzItemSelectionModel::Select);
    sm.setCurrentIndex(source.index(1, 0), TzItemSelectionModel::NoUpdate);
    sm.clear();

    EXPECT_FALSE(sm.hasSelection());
    EXPECT_FALSE(sm.currentIndex().isValid());
}

TEST(TzItemSelectionModel, Reset)
{
    SelTestModel source(3);
    TzItemSelectionModel sm(&source);

    sm.select(source.index(0, 0), TzItemSelectionModel::Select);
    sm.reset();

    EXPECT_FALSE(sm.hasSelection());
}

TEST(TzItemSelectionModel, SetCurrentIndex)
{
    SelTestModel source(5);
    TzItemSelectionModel sm(&source);

    TzModelIndex idx = source.index(2, 0);
    sm.setCurrentIndex(idx, TzItemSelectionModel::NoUpdate);

    EXPECT_EQ(sm.currentIndex(), idx);
    EXPECT_FALSE(sm.isSelected(idx)); // NoUpdate → no selection change
}

TEST(TzItemSelectionModel, SetCurrentIndexWithSelect)
{
    SelTestModel source(5);
    TzItemSelectionModel sm(&source);

    TzModelIndex idx = source.index(2, 0);
    sm.setCurrentIndex(idx, TzItemSelectionModel::Select);

    EXPECT_EQ(sm.currentIndex(), idx);
    EXPECT_TRUE(sm.isSelected(idx));
}

TEST(TzItemSelectionModel, ClearCurrentIndex)
{
    SelTestModel source(3);
    TzItemSelectionModel sm(&source);

    sm.setCurrentIndex(source.index(0, 0), TzItemSelectionModel::NoUpdate);
    EXPECT_TRUE(sm.currentIndex().isValid());

    sm.clearCurrentIndex();
    EXPECT_FALSE(sm.currentIndex().isValid());
}

TEST(TzItemSelectionModel, IsRowSelected)
{
    SelTestModel source(5);
    TzItemSelectionModel sm(&source);

    // List model has only column 0 → selecting (row, 0) selects the entire row
    sm.select(source.index(2, 0), TzItemSelectionModel::Select);
    EXPECT_TRUE(sm.isRowSelected(2));
    EXPECT_FALSE(sm.isRowSelected(0));
    EXPECT_FALSE(sm.isRowSelected(1));
}

TEST(TzItemSelectionModel, IsColumnSelected)
{
    SelTestModel source(3);
    TzItemSelectionModel sm(&source);

    // Select all rows in column 0 → the whole column is selected
    TzItemSelection sel(source.index(0, 0), source.index(2, 0));
    sm.select(sel, TzItemSelectionModel::Select);
    EXPECT_TRUE(sm.isColumnSelected(0));
}

TEST(TzItemSelectionModel, RowIntersectsSelection)
{
    SelTestModel source(5);
    TzItemSelectionModel sm(&source);

    sm.select(source.index(3, 0), TzItemSelectionModel::Select);
    EXPECT_TRUE(sm.rowIntersectsSelection(3));
    EXPECT_FALSE(sm.rowIntersectsSelection(2));
}

TEST(TzItemSelectionModel, ColumnIntersectsSelection)
{
    SelTestModel source(3);
    TzItemSelectionModel sm(&source);

    sm.select(source.index(1, 0), TzItemSelectionModel::Select);
    EXPECT_TRUE(sm.columnIntersectsSelection(0));
}

TEST(TzItemSelectionModel, SelectedIndexes)
{
    SelTestModel source(5);
    TzItemSelectionModel sm(&source);

    TzItemSelection sel(source.index(1, 0), source.index(3, 0));
    sm.select(sel, TzItemSelectionModel::Select);

    TzModelIndexList idx = sm.selectedIndexes();
    ASSERT_EQ(idx.size(), 3u);
    EXPECT_EQ(idx[0].row(), 1);
    EXPECT_EQ(idx[1].row(), 2);
    EXPECT_EQ(idx[2].row(), 3);
}

TEST(TzItemSelectionModel, SelectedRows)
{
    SelTestModel source(5);
    TzItemSelectionModel sm(&source);

    sm.select(source.index(0, 0), TzItemSelectionModel::Select);
    sm.select(source.index(2, 0), TzItemSelectionModel::Select);

    TzModelIndexList rows = sm.selectedRows(0);
    ASSERT_EQ(rows.size(), 2u);
    EXPECT_EQ(rows[0].row(), 0);
    EXPECT_EQ(rows[1].row(), 2);
}

TEST(TzItemSelectionModel, SignalSelectionChanged)
{
    SelTestModel source(5);
    TzItemSelectionModel sm(&source);

    int fired = 0;
    sm.events().on("selectionChanged",
        [&](const TzItemSelection & /*selected*/, const TzItemSelection & /*deselected*/) {
            ++fired;
        });

    sm.select(source.index(1, 0), TzItemSelectionModel::Select);
    EXPECT_EQ(fired, 1);

    sm.select(source.index(1, 0), TzItemSelectionModel::Deselect);
    EXPECT_EQ(fired, 2);
}

TEST(TzItemSelectionModel, SignalCurrentChanged)
{
    SelTestModel source(5);
    TzItemSelectionModel sm(&source);

    int fired = 0;
    TzModelIndex capturedCurrent, capturedPrevious;
    sm.events().on("currentChanged",
        [&](const TzModelIndex &current, const TzModelIndex &previous) {
            ++fired;
            capturedCurrent  = current;
            capturedPrevious = previous;
        });

    TzModelIndex first = source.index(1, 0);
    sm.setCurrentIndex(first, TzItemSelectionModel::NoUpdate);
    EXPECT_EQ(fired, 1);
    EXPECT_EQ(capturedCurrent.row(), 1);
    EXPECT_FALSE(capturedPrevious.isValid());

    TzModelIndex second = source.index(3, 0);
    sm.setCurrentIndex(second, TzItemSelectionModel::NoUpdate);
    EXPECT_EQ(fired, 2);
    EXPECT_EQ(capturedCurrent.row(), 3);
    EXPECT_EQ(capturedPrevious.row(), 1);
}

TEST(TzItemSelectionModel, SignalCurrentRowChanged)
{
    SelTestModel source(5);
    TzItemSelectionModel sm(&source);

    int fired = 0;
    sm.events().on("currentRowChanged",
        [&](const TzModelIndex &, const TzModelIndex &) { ++fired; });

    sm.setCurrentIndex(source.index(0, 0), TzItemSelectionModel::NoUpdate);
    EXPECT_EQ(fired, 1);

    sm.setCurrentIndex(source.index(2, 0), TzItemSelectionModel::NoUpdate);
    EXPECT_EQ(fired, 2);
}

TEST(TzItemSelectionModel, SignalCurrentChangedNotFiredForSameIndex)
{
    SelTestModel source(3);
    TzItemSelectionModel sm(&source);

    TzModelIndex idx = source.index(1, 0);
    sm.setCurrentIndex(idx, TzItemSelectionModel::NoUpdate);

    int fired = 0;
    sm.events().on("currentChanged",
        [&](const TzModelIndex &, const TzModelIndex &) { ++fired; });

    // Setting the same index with NoUpdate → no signal
    sm.setCurrentIndex(idx, TzItemSelectionModel::NoUpdate);
    EXPECT_EQ(fired, 0);
}

TEST(TzItemSelectionModel, RowsRemovedInvalidatesSelection)
{
    SelTestModel source(5);
    TzItemSelectionModel sm(&source);

    // Select rows 2 and 3
    sm.select(source.index(2, 0), TzItemSelectionModel::Select);
    sm.select(source.index(3, 0), TzItemSelectionModel::Select);
    EXPECT_EQ(sm.selectedIndexes().size(), 2u);

    // Remove rows 2-3
    source.removeRows(2, 3);
    EXPECT_FALSE(sm.hasSelection());
}

TEST(TzItemSelectionModel, RowsInsertedPreservesSelection)
{
    SelTestModel source(5);
    TzItemSelectionModel sm(&source);

    sm.select(source.index(0, 0), TzItemSelectionModel::Select);
    EXPECT_TRUE(sm.isSelected(source.index(0, 0)));

    source.appendRows(3);
    // Row 0 selection should survive the insertion of rows at the end
    EXPECT_TRUE(sm.isSelected(source.index(0, 0)));
}

TEST(TzItemSelectionModel, ScopedListenerAutoDisconnects)
{
    SelTestModel source(3);
    TzItemSelectionModel sm(&source);

    int fired = 0;
    {
        TzScopedEventListener scoped{
            sm.events().on("selectionChanged",
                [&](const TzItemSelection &, const TzItemSelection &) { ++fired; })
        };
        sm.select(source.index(0, 0), TzItemSelectionModel::Select);
        EXPECT_EQ(fired, 1);
    } // auto-disconnect

    sm.select(source.index(1, 0), TzItemSelectionModel::ClearAndSelect);
    EXPECT_EQ(fired, 1); // must not increment
}

TEST(TzItemSelectionRange, Validity)
{
    SelTestModel source(4);
    TzItemSelectionRange valid(source.index(0, 0), source.index(2, 0));
    EXPECT_TRUE(valid.isValid());
    EXPECT_EQ(valid.top(), 0);
    EXPECT_EQ(valid.bottom(), 2);

    TzItemSelectionRange def;
    EXPECT_FALSE(def.isValid());
}

TEST(TzItemSelectionRange, Intersects)
{
    SelTestModel source(6);
    TzItemSelectionRange r1(source.index(0, 0), source.index(2, 0));
    TzItemSelectionRange r2(source.index(1, 0), source.index(4, 0));
    TzItemSelectionRange r3(source.index(3, 0), source.index(5, 0));

    EXPECT_TRUE(r1.intersects(r2));
    EXPECT_FALSE(r1.intersects(r3));
}

TEST(TzItemSelectionRange, Intersected)
{
    SelTestModel source(6);
    TzItemSelectionRange r1(source.index(0, 0), source.index(3, 0));
    TzItemSelectionRange r2(source.index(2, 0), source.index(5, 0));

    TzItemSelectionRange intersection = r1.intersected(r2);
    EXPECT_TRUE(intersection.isValid());
    EXPECT_EQ(intersection.top(), 2);
    EXPECT_EQ(intersection.bottom(), 3);
}
