#include <gtest/gtest.h>
#include <loom/tzabstractlistmodel.hpp>
#include <loom/tzscopedeventlistener.hpp>

class TestListModel : public TzAbstractListModel
{
public:
    explicit TestListModel(int rows = 0) : m_rows(rows) {}

    int rowCount(const TzModelIndex &parent = TzModelIndex()) const override
    {
        return parent.isValid() ? 0 : m_rows;
    }

    std::any data(const TzModelIndex &index, int /*role*/) const override
    {
        if (!index.isValid() || index.row() >= m_rows)
            return std::any{};
        return m_data.count(index.row()) ? m_data.at(index.row()) : std::any{};
    }

    void publicBeginInsertRows(int first, int last)
    {
        beginInsertRows(TzModelIndex(), first, last);
        m_rows += (last - first + 1);
    }
    void publicEndInsertRows() { endInsertRows(); }

    void publicBeginRemoveRows(int first, int last)
    {
        beginRemoveRows(TzModelIndex(), first, last);
        m_rows -= (last - first + 1);
    }
    void publicEndRemoveRows() { endRemoveRows(); }

    void publicBeginResetModel() { beginResetModel(); m_rows = 0; m_data.clear(); }
    void publicEndResetModel()   { endResetModel(); }

    void publicDataChanged(int row)
    {
        auto idx = index(row, 0);
        dataChanged(idx, idx);
    }

    void setRow(int row, std::any value) { m_data[row] = std::move(value); }

private:
    int m_rows;
    std::unordered_map<int, std::any> m_data;
};

TEST(TzAbstractListModel, DefaultConstruction)
{
    TestListModel model;
    EXPECT_EQ(model.rowCount(), 0);
    EXPECT_FALSE(model.hasIndex(0, 0)); // no rows → no valid indices
}

TEST(TzAbstractListModel, RowCount)
{
    TestListModel model(5);
    EXPECT_EQ(model.rowCount(), 5);
    EXPECT_TRUE(model.hasIndex(0, 0));
    EXPECT_FALSE(model.hasIndex(5, 0)); // out of range
}

TEST(TzAbstractListModel, ValidIndex)
{
    TestListModel model(3);
    TzModelIndex idx = model.index(0, 0);
    EXPECT_TRUE(idx.isValid());
    EXPECT_EQ(idx.row(), 0);
    EXPECT_EQ(idx.column(), 0);
}

TEST(TzAbstractListModel, InvalidIndexOutOfRange)
{
    TestListModel model(3);
    EXPECT_FALSE(model.index(3, 0).isValid());
    EXPECT_FALSE(model.index(-1, 0).isValid());
    EXPECT_FALSE(model.index(0, 1).isValid());
}

TEST(TzAbstractListModel, ParentAlwaysInvalid)
{
    TestListModel model(3);
    TzModelIndex idx = model.index(1, 0);
    EXPECT_FALSE(idx.parent().isValid());
}

TEST(TzAbstractListModel, SignalRowsInserted)
{
    TestListModel model;

    int aboutFired = 0, insertedFired = 0;
    int capturedFirst = -1, capturedLast = -1;

    auto l1 = model.events().on("rowsAboutToBeInserted",
        [&](const TzModelIndex & /*parent*/, int first, int last) {
            ++aboutFired;
            capturedFirst = first;
            capturedLast  = last;
        });

    auto l2 = model.events().on("rowsInserted",
        [&](const TzModelIndex & /*parent*/, int /*first*/, int /*last*/) {
            ++insertedFired;
        });

    model.publicBeginInsertRows(0, 2);
    model.publicEndInsertRows();

    EXPECT_EQ(aboutFired,    1);
    EXPECT_EQ(insertedFired, 1);
    EXPECT_EQ(capturedFirst, 0);
    EXPECT_EQ(capturedLast,  2);
    EXPECT_EQ(model.rowCount(), 3);
}

TEST(TzAbstractListModel, SignalRowsRemoved)
{
    TestListModel model(5);

    int removedFired = 0;
    auto l = model.events().on("rowsRemoved",
        [&](const TzModelIndex & /*parent*/, int /*first*/, int /*last*/) {
            ++removedFired;
        });

    model.publicBeginRemoveRows(1, 3);
    model.publicEndRemoveRows();

    EXPECT_EQ(removedFired, 1);
    EXPECT_EQ(model.rowCount(), 2);
}

TEST(TzAbstractListModel, SignalDataChanged)
{
    TestListModel model(3);

    int fired = 0;
    TzModelIndex capturedTL, capturedBR;

    auto l = model.events().on("dataChanged",
        [&](const TzModelIndex &tl, const TzModelIndex &br, const std::vector<int> & /*roles*/) {
            ++fired;
            capturedTL = tl;
            capturedBR = br;
        });

    model.publicDataChanged(1);

    EXPECT_EQ(fired, 1);
    EXPECT_EQ(capturedTL.row(), 1);
    EXPECT_EQ(capturedBR.row(), 1);
}

TEST(TzAbstractListModel, SignalModelReset)
{
    TestListModel model(4);

    int aboutFired = 0, resetFired = 0;
    model.events().on("modelAboutToBeReset", [&]() { ++aboutFired; });
    model.events().on("modelReset",          [&]() { ++resetFired; });

    model.publicBeginResetModel();
    model.publicEndResetModel();

    EXPECT_EQ(aboutFired, 1);
    EXPECT_EQ(resetFired, 1);
    EXPECT_EQ(model.rowCount(), 0);
}

TEST(TzAbstractListModel, MultipleListeners)
{
    TestListModel model;

    int a = 0, b = 0;
    model.events().on("rowsInserted",
        [&](const TzModelIndex &, int, int) { ++a; });
    model.events().on("rowsInserted",
        [&](const TzModelIndex &, int, int) { ++b; });

    model.publicBeginInsertRows(0, 0);
    model.publicEndInsertRows();

    EXPECT_EQ(a, 1);
    EXPECT_EQ(b, 1);
}

TEST(TzAbstractListModel, DisconnectListener)
{
    TestListModel model;

    int fired = 0;
    TzEventListener listener = model.events().on("rowsInserted",
        [&](const TzModelIndex &, int, int) { ++fired; });

    model.publicBeginInsertRows(0, 0);
    model.publicEndInsertRows();
    EXPECT_EQ(fired, 1);

    listener.disconnect();

    model.publicBeginInsertRows(1, 1);
    model.publicEndInsertRows();
    EXPECT_EQ(fired, 1); // should not have incremented
}

TEST(TzAbstractListModel, ScopedListenerAutoDisconnects)
{
    TestListModel model(2);

    int fired = 0;
    {
        TzScopedEventListener scoped{
            model.events().on("rowsRemoved",
                [&](const TzModelIndex &, int, int) { ++fired; })
        };

        model.publicBeginRemoveRows(0, 0);
        model.publicEndRemoveRows();
        EXPECT_EQ(fired, 1);
    } // scoped destructs here

    model.publicBeginRemoveRows(0, 0);
    model.publicEndRemoveRows();
    EXPECT_EQ(fired, 1); // should not fire after disconnect
}

TEST(TzAbstractListModel, CheckIndexValid)
{
    TestListModel model(3);
    EXPECT_TRUE(model.checkIndex(model.index(0, 0)));
    EXPECT_TRUE(model.checkIndex(model.index(2, 0)));
}

TEST(TzAbstractListModel, CheckIndexInvalid)
{
    TestListModel model(3);

    // List model: valid index always has an invalid parent → both flags satisfied
    EXPECT_TRUE(model.checkIndex(model.index(0, 0),
        TzAbstractItemModel::CheckIndexOption::ParentIsInvalid
        | TzAbstractItemModel::CheckIndexOption::IndexIsValid));

    // An invalid TzModelIndex without IndexIsValid → returns true (invalid is acceptable)
    EXPECT_TRUE(model.checkIndex(TzModelIndex()));

    // An invalid TzModelIndex WITH IndexIsValid → returns false
    EXPECT_FALSE(model.checkIndex(TzModelIndex(),
        TzAbstractItemModel::CheckIndexOption::IndexIsValid));
}

TEST(TzAbstractListModel, FlagsDefault)
{
    TestListModel model(1);
    TzModelIndex idx = model.index(0, 0);
    TzItemFlags f = model.flags(idx);
    EXPECT_TRUE(f.testFlag(TzItemFlag::ItemIsSelectable));
    EXPECT_TRUE(f.testFlag(TzItemFlag::ItemIsEnabled));
    EXPECT_TRUE(f.testFlag(TzItemFlag::ItemNeverHasChildren));
}

TEST(TzAbstractListModel, DefaultRoleNames)
{
    TestListModel model;
    auto names = model.roleNames();
    EXPECT_EQ(names.at(TzItemDataRole::DisplayRole), "display");
    EXPECT_EQ(names.at(TzItemDataRole::EditRole),    "edit");
}

TEST(TzAbstractListModel, SubmitRevert)
{
    TestListModel model;
    EXPECT_TRUE(model.submit());
    model.revert(); // no-op, just verify it doesn't crash
}
