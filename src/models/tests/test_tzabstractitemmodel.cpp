#include <loom/tzabstractitemmodel.hpp>
#include <loom/tzabstractlistmodel.hpp>
#include <gtest/gtest.h>
#include <any>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Minimal concrete model backed by a vector<string>
// ---------------------------------------------------------------------------

class StringListModel : public TzAbstractListModel
{
public:
    explicit StringListModel(std::vector<std::string> data = {})
        : m_data(std::move(data))
    {}

    int rowCount(const TzModelIndex &parent = TzModelIndex()) const override
    {
        if (parent.isValid())
            return 0;
        return static_cast<int>(m_data.size());
    }

    std::any data(const TzModelIndex &index, int role = DisplayRole) const override
    {
        if (!checkIndex(index, CheckIndexOption::IndexIsValid))
            return {};
        if (role == DisplayRole || role == EditRole)
            return m_data[index.row()];
        return {};
    }

    bool setData(const TzModelIndex &index, const std::any &value, int role = EditRole) override
    {
        if (!checkIndex(index, CheckIndexOption::IndexIsValid))
            return false;
        if (role != EditRole)
            return false;
        m_data[index.row()] = std::any_cast<std::string>(value);
        dataChanged(index, index, {role});
        return true;
    }

    bool insertRows(int row, int count, const TzModelIndex &parent = TzModelIndex()) override
    {
        if (parent.isValid() || row < 0 || row > rowCount())
            return false;
        beginInsertRows(parent, row, row + count - 1);
        m_data.insert(m_data.begin() + row, count, std::string{});
        endInsertRows();
        return true;
    }

    bool removeRows(int row, int count, const TzModelIndex &parent = TzModelIndex()) override
    {
        if (parent.isValid() || row < 0 || row + count > rowCount())
            return false;
        beginRemoveRows(parent, row, row + count - 1);
        m_data.erase(m_data.begin() + row, m_data.begin() + row + count);
        endRemoveRows();
        return true;
    }

    void reset()
    {
        beginResetModel();
        endResetModel();
    }

    const std::vector<std::string> &strings() const { return m_data; }

private:
    std::vector<std::string> m_data;
};

// ---------------------------------------------------------------------------
// TzModelIndex tests
// ---------------------------------------------------------------------------

TEST(TzModelIndex, DefaultIsInvalid)
{
    TzModelIndex idx;
    EXPECT_FALSE(idx.isValid());
    EXPECT_EQ(idx.row(), -1);
    EXPECT_EQ(idx.column(), -1);
    EXPECT_EQ(idx.model(), nullptr);
}

TEST(TzModelIndex, EqualityOnDefault)
{
    TzModelIndex a, b;
    EXPECT_EQ(a, b);
}

// ---------------------------------------------------------------------------
// rowCount / columnCount
// ---------------------------------------------------------------------------

TEST(StringListModel, EmptyModel)
{
    StringListModel m;
    EXPECT_EQ(m.rowCount(), 0);
    EXPECT_EQ(m.columnCount(), 1);
}

TEST(StringListModel, InitialData)
{
    StringListModel m({"alpha", "beta", "gamma"});
    EXPECT_EQ(m.rowCount(), 3);
    EXPECT_EQ(m.columnCount(), 1);
}

// ---------------------------------------------------------------------------
// index() and data()
// ---------------------------------------------------------------------------

TEST(StringListModel, ValidIndex)
{
    StringListModel m({"hello", "world"});
    TzModelIndex idx = m.index(0, 0);
    EXPECT_TRUE(idx.isValid());
    EXPECT_EQ(idx.row(), 0);
    EXPECT_EQ(idx.column(), 0);
    EXPECT_EQ(idx.model(), &m);
}

TEST(StringListModel, InvalidIndexOutOfRange)
{
    StringListModel m({"hello"});
    TzModelIndex idx = m.index(5, 0);
    EXPECT_FALSE(idx.isValid());
}

TEST(StringListModel, DataDisplayRole)
{
    StringListModel m({"apple", "banana"});
    auto v = m.data(m.index(1, 0), DisplayRole);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(std::any_cast<std::string>(v), "banana");
}

TEST(StringListModel, DataInvalidIndexReturnsEmpty)
{
    StringListModel m({"a"});
    auto v = m.data(TzModelIndex{});
    EXPECT_FALSE(v.has_value());
}

TEST(StringListModel, DataUnknownRoleReturnsEmpty)
{
    StringListModel m({"a"});
    auto v = m.data(m.index(0, 0), 999);
    EXPECT_FALSE(v.has_value());
}

// ---------------------------------------------------------------------------
// setData
// ---------------------------------------------------------------------------

TEST(StringListModel, SetData)
{
    StringListModel m({"old"});
    bool ok = m.setData(m.index(0, 0), std::string{"new"}, EditRole);
    EXPECT_TRUE(ok);
    auto v = m.data(m.index(0, 0));
    EXPECT_EQ(std::any_cast<std::string>(v), "new");
}

TEST(StringListModel, SetDataWrongRole)
{
    StringListModel m({"x"});
    bool ok = m.setData(m.index(0, 0), std::string{"y"}, DisplayRole);
    EXPECT_FALSE(ok);
    EXPECT_EQ(std::any_cast<std::string>(m.data(m.index(0, 0))), "x");
}

// ---------------------------------------------------------------------------
// insertRows
// ---------------------------------------------------------------------------

TEST(StringListModel, InsertRowsAtEnd)
{
    StringListModel m({"a", "b"});
    bool ok = m.insertRows(2, 1);
    EXPECT_TRUE(ok);
    EXPECT_EQ(m.rowCount(), 3);
    EXPECT_EQ(std::any_cast<std::string>(m.data(m.index(2, 0))), "");
}

TEST(StringListModel, InsertRowsAtBeginning)
{
    StringListModel m({"a", "b"});
    bool ok = m.insertRows(0, 2);
    EXPECT_TRUE(ok);
    EXPECT_EQ(m.rowCount(), 4);
    EXPECT_EQ(std::any_cast<std::string>(m.data(m.index(2, 0))), "a");
}

TEST(StringListModel, InsertRowsInMiddle)
{
    StringListModel m({"a", "c"});
    m.insertRows(1, 1);
    m.setData(m.index(1, 0), std::string{"b"});
    EXPECT_EQ(m.rowCount(), 3);
    EXPECT_EQ(std::any_cast<std::string>(m.data(m.index(0, 0))), "a");
    EXPECT_EQ(std::any_cast<std::string>(m.data(m.index(1, 0))), "b");
    EXPECT_EQ(std::any_cast<std::string>(m.data(m.index(2, 0))), "c");
}

TEST(StringListModel, InsertRowsWithValidParentFails)
{
    StringListModel m({"a"});
    bool ok = m.insertRows(0, 1, m.index(0, 0));
    EXPECT_FALSE(ok);
}

// ---------------------------------------------------------------------------
// removeRows
// ---------------------------------------------------------------------------

TEST(StringListModel, RemoveFirstRow)
{
    StringListModel m({"a", "b", "c"});
    bool ok = m.removeRows(0, 1);
    EXPECT_TRUE(ok);
    EXPECT_EQ(m.rowCount(), 2);
    EXPECT_EQ(std::any_cast<std::string>(m.data(m.index(0, 0))), "b");
}

TEST(StringListModel, RemoveLastRow)
{
    StringListModel m({"a", "b", "c"});
    bool ok = m.removeRows(2, 1);
    EXPECT_TRUE(ok);
    EXPECT_EQ(m.rowCount(), 2);
    EXPECT_EQ(std::any_cast<std::string>(m.data(m.index(1, 0))), "b");
}

TEST(StringListModel, RemoveAllRows)
{
    StringListModel m({"a", "b", "c"});
    bool ok = m.removeRows(0, 3);
    EXPECT_TRUE(ok);
    EXPECT_EQ(m.rowCount(), 0);
}

TEST(StringListModel, RemoveRowsOutOfRangeFails)
{
    StringListModel m({"a"});
    bool ok = m.removeRows(0, 5);
    EXPECT_FALSE(ok);
    EXPECT_EQ(m.rowCount(), 1);
}

// ---------------------------------------------------------------------------
// Convenience single-row helpers
// ---------------------------------------------------------------------------

TEST(StringListModel, InsertRow)
{
    StringListModel m({"a"});
    bool ok = m.insertRow(1);
    EXPECT_TRUE(ok);
    EXPECT_EQ(m.rowCount(), 2);
}

TEST(StringListModel, RemoveRow)
{
    StringListModel m({"a", "b"});
    bool ok = m.removeRow(0);
    EXPECT_TRUE(ok);
    EXPECT_EQ(m.rowCount(), 1);
    EXPECT_EQ(std::any_cast<std::string>(m.data(m.index(0, 0))), "b");
}

// ---------------------------------------------------------------------------
// hasIndex / hasChildren
// ---------------------------------------------------------------------------

TEST(StringListModel, HasIndex)
{
    StringListModel m({"a", "b"});
    EXPECT_TRUE(m.hasIndex(0, 0));
    EXPECT_TRUE(m.hasIndex(1, 0));
    EXPECT_FALSE(m.hasIndex(2, 0));
    EXPECT_FALSE(m.hasIndex(0, 1));
}

TEST(StringListModel, HasChildrenRoot)
{
    StringListModel m({"a"});
    EXPECT_TRUE(m.hasChildren());
}

TEST(StringListModel, HasChildrenEmptyModel)
{
    StringListModel m;
    EXPECT_FALSE(m.hasChildren());
}

// ---------------------------------------------------------------------------
// Signal emission via TzEventEmitter
// ---------------------------------------------------------------------------

TEST(StringListModel, DataChangedSignalEmitted)
{
    StringListModel m({"x"});
    int count = 0;
    m.on("dataChanged", [&](const TzModelIndex &, const TzModelIndex &,
                             const std::vector<int> &) { ++count; });

    m.setData(m.index(0, 0), std::string{"y"});
    EXPECT_EQ(count, 1);
}

TEST(StringListModel, RowsInsertedSignalEmitted)
{
    StringListModel m;
    int first = -1, last = -1;
    m.on("rowsInserted", [&](const TzModelIndex &, int f, int l) {
        first = f;
        last  = l;
    });

    m.insertRows(0, 3);
    EXPECT_EQ(first, 0);
    EXPECT_EQ(last,  2);
}

TEST(StringListModel, RowsRemovedSignalEmitted)
{
    StringListModel m({"a", "b", "c"});
    int first = -1, last = -1;
    m.on("rowsRemoved", [&](const TzModelIndex &, int f, int l) {
        first = f;
        last  = l;
    });

    m.removeRows(1, 2);
    EXPECT_EQ(first, 1);
    EXPECT_EQ(last,  2);
}

TEST(StringListModel, RowsAboutToBeInsertedBeforeChange)
{
    StringListModel m({"a"});
    int rowCountAtSignal = -1;
    m.on("rowsAboutToBeInserted", [&](const TzModelIndex &, int, int) {
        rowCountAtSignal = m.rowCount();
    });

    m.insertRows(1, 1);
    // The signal fires before the actual insertion
    EXPECT_EQ(rowCountAtSignal, 1);
}

TEST(StringListModel, RowsAboutToBeRemovedBeforeChange)
{
    StringListModel m({"a", "b"});
    int rowCountAtSignal = -1;
    m.on("rowsAboutToBeRemoved", [&](const TzModelIndex &, int, int) {
        rowCountAtSignal = m.rowCount();
    });

    m.removeRows(0, 1);
    EXPECT_EQ(rowCountAtSignal, 2);
}

// ---------------------------------------------------------------------------
// TzPersistentModelIndex
// ---------------------------------------------------------------------------

TEST(TzPersistentModelIndex, DefaultIsInvalid)
{
    TzPersistentModelIndex pi;
    EXPECT_FALSE(pi.isValid());
}

TEST(TzPersistentModelIndex, ConstructFromValidIndex)
{
    StringListModel m({"a", "b"});
    TzPersistentModelIndex pi(m.index(1, 0));
    EXPECT_TRUE(pi.isValid());
    EXPECT_EQ(pi.row(), 1);
    EXPECT_EQ(pi.column(), 0);
}

TEST(TzPersistentModelIndex, StaysValidAfterInsertBefore)
{
    StringListModel m({"a", "b", "c"});
    TzPersistentModelIndex pi(m.index(2, 0)); // points at "c"
    EXPECT_EQ(pi.row(), 2);

    m.insertRows(0, 1); // insert at top → "c" is now row 3
    EXPECT_TRUE(pi.isValid());
    EXPECT_EQ(pi.row(), 3);
    EXPECT_EQ(std::any_cast<std::string>(m.data(pi)), "c");
}

TEST(TzPersistentModelIndex, InvalidatedAfterRemove)
{
    StringListModel m({"a", "b", "c"});
    TzPersistentModelIndex pi(m.index(1, 0)); // points at "b"

    m.removeRows(1, 1); // remove "b"
    EXPECT_FALSE(pi.isValid());
}

TEST(TzPersistentModelIndex, ShiftedAfterRemoveBefore)
{
    StringListModel m({"a", "b", "c"});
    TzPersistentModelIndex pi(m.index(2, 0)); // points at "c"

    m.removeRows(0, 1); // remove "a" → "c" shifts to row 1
    EXPECT_TRUE(pi.isValid());
    EXPECT_EQ(pi.row(), 1);
}

TEST(TzPersistentModelIndex, CopySharesData)
{
    StringListModel m({"x", "y"});
    TzPersistentModelIndex a(m.index(0, 0));
    TzPersistentModelIndex b = a;
    EXPECT_EQ(a.row(), b.row());
    EXPECT_TRUE(a == b);
}

TEST(TzPersistentModelIndex, MoveIsValid)
{
    StringListModel m({"x"});
    TzPersistentModelIndex a(m.index(0, 0));
    TzPersistentModelIndex b = std::move(a);
    EXPECT_TRUE(b.isValid());
    EXPECT_FALSE(a.isValid());
}

// ---------------------------------------------------------------------------
// flags / roleNames / buddy
// ---------------------------------------------------------------------------

TEST(StringListModel, FlagsOnValidIndex)
{
    StringListModel m({"a"});
    TzItemFlags f = m.flags(m.index(0, 0));
    EXPECT_TRUE(f.testFlag(TzItemFlag::ItemIsEnabled));
    EXPECT_TRUE(f.testFlag(TzItemFlag::ItemIsSelectable));
}

TEST(StringListModel, FlagsOnInvalidIndex)
{
    StringListModel m;
    TzItemFlags f = m.flags(TzModelIndex{});
    EXPECT_FALSE(static_cast<bool>(f));
}

TEST(StringListModel, BuddyReturnsSameIndex)
{
    StringListModel m({"a"});
    TzModelIndex idx = m.index(0, 0);
    EXPECT_EQ(m.buddy(idx), idx);
}

TEST(StringListModel, RoleNamesContainDisplay)
{
    StringListModel m;
    auto names = m.roleNames();
    ASSERT_TRUE(names.count(DisplayRole));
    EXPECT_EQ(names[DisplayRole], "display");
}

// ---------------------------------------------------------------------------
// checkIndex
// ---------------------------------------------------------------------------

TEST(StringListModel, CheckIndexDefaultOptions)
{
    StringListModel m({"a"});
    // Valid in-range index passes with default options
    EXPECT_TRUE(m.checkIndex(m.index(0, 0)));
    // Default (NoOption): invalid TzModelIndex is accepted (no IndexIsValid constraint)
    EXPECT_TRUE(m.checkIndex(TzModelIndex{}));
    // Index belonging to a different model is rejected
    StringListModel other({"x"});
    EXPECT_FALSE(m.checkIndex(other.index(0, 0)));
}

TEST(StringListModel, CheckIndexIndexIsValid)
{
    StringListModel m({"a"});
    EXPECT_TRUE(m.checkIndex(m.index(0, 0), TzAbstractItemModel::CheckIndexOption::IndexIsValid));
    EXPECT_FALSE(
        m.checkIndex(TzModelIndex{}, TzAbstractItemModel::CheckIndexOption::IndexIsValid));
}

// ---------------------------------------------------------------------------
// Model reset
// ---------------------------------------------------------------------------

TEST(StringListModel, ResetEmitsSignals)
{
    StringListModel m({"a", "b"});
    bool aboutToReset = false, didReset = false;
    m.on("modelAboutToBeReset", [&] { aboutToReset = true; });
    m.on("modelReset", [&] { didReset = true; });

    m.reset();

    EXPECT_TRUE(aboutToReset);
    EXPECT_TRUE(didReset);
}
