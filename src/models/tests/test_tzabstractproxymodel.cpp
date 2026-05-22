#include <gtest/gtest.h>
#include <loom/tzabstractproxymodel.hpp>
#include <loom/tzitemselectionmodel.hpp>
#include <loom/tzscopedeventlistener.hpp>

class ProxyTestSourceModel : public TzAbstractListModel
{
public:
    explicit ProxyTestSourceModel(int rows = 0) : m_rows(rows) {}

    int rowCount(const TzModelIndex &parent = TzModelIndex()) const override
    { return parent.isValid() ? 0 : m_rows; }

    std::any data(const TzModelIndex &index, int /*role*/) const override
    {
        if (!index.isValid() || index.row() >= m_rows) return {};
        auto it = m_data.find(index.row());
        return it != m_data.end() ? it->second : std::any{};
    }

    bool setData(const TzModelIndex &index, const std::any &value, int /*role*/) override
    {
        if (!index.isValid() || index.row() >= m_rows) return false;
        m_data[index.row()] = value;
        dataChanged(index, index);
        return true;
    }

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

    void reset(int newRows = 0)
    {
        beginResetModel();
        m_rows = newRows;
        m_data.clear();
        endResetModel();
    }

private:
    int m_rows;
    std::unordered_map<int, std::any> m_data;
};

class IdentityProxy : public TzAbstractProxyModel
{
public:
    IdentityProxy() = default;

    TzModelIndex mapToSource(const TzModelIndex &proxyIndex) const override
    {
        if (!proxyIndex.isValid() || !sourceModel())
            return {};
        return sourceModel()->index(proxyIndex.row(), proxyIndex.column());
    }

    TzModelIndex mapFromSource(const TzModelIndex &sourceIndex) const override
    {
        if (!sourceIndex.isValid())
            return {};
        return createIndex(sourceIndex.row(), sourceIndex.column());
    }

    TzModelIndex index(int row, int column,
                       const TzModelIndex &parent = TzModelIndex()) const override
    {
        if (!sourceModel() || !hasIndex(row, column, parent))
            return {};
        return createIndex(row, column);
    }

    TzModelIndex parent(const TzModelIndex &) const override { return {}; }

    int rowCount(const TzModelIndex &parent = TzModelIndex()) const override
    { return sourceModel() ? sourceModel()->rowCount(mapToSource(parent)) : 0; }

    int columnCount(const TzModelIndex &parent = TzModelIndex()) const override
    { return sourceModel() ? sourceModel()->columnCount(mapToSource(parent)) : 0; }
};

TEST(TzAbstractProxyModel, DefaultConstruction)
{
    IdentityProxy proxy;
    // No source set yet → sourceModel() is nullptr
    EXPECT_EQ(proxy.sourceModel(), nullptr);
    EXPECT_EQ(proxy.rowCount(), 0);
}

TEST(TzAbstractProxyModel, SetSourceModel)
{
    ProxyTestSourceModel source(4);
    IdentityProxy proxy;

    proxy.setSourceModel(&source);
    EXPECT_EQ(proxy.sourceModel(), &source);
    EXPECT_EQ(proxy.rowCount(), 4);
}

TEST(TzAbstractProxyModel, SetSourceModelNull)
{
    ProxyTestSourceModel source(3);
    IdentityProxy proxy;
    proxy.setSourceModel(&source);

    proxy.setSourceModel(nullptr);
    EXPECT_EQ(proxy.sourceModel(), nullptr);
    EXPECT_EQ(proxy.rowCount(), 0);
}

TEST(TzAbstractProxyModel, ReplaceSourceModel)
{
    ProxyTestSourceModel s1(2), s2(7);
    IdentityProxy proxy;

    proxy.setSourceModel(&s1);
    EXPECT_EQ(proxy.rowCount(), 2);

    proxy.setSourceModel(&s2);
    EXPECT_EQ(proxy.sourceModel(), &s2);
    EXPECT_EQ(proxy.rowCount(), 7);
}

TEST(TzAbstractProxyModel, SignalSourceModelChanged)
{
    ProxyTestSourceModel source(1);
    IdentityProxy proxy;

    int fired = 0;
    proxy.events().on("sourceModelChanged", [&]() { ++fired; });

    proxy.setSourceModel(&source);
    EXPECT_EQ(fired, 1);

    proxy.setSourceModel(nullptr);
    EXPECT_EQ(fired, 2);
}

TEST(TzAbstractProxyModel, SetSameSourceModelNoSignal)
{
    ProxyTestSourceModel source(1);
    IdentityProxy proxy;
    proxy.setSourceModel(&source);

    int fired = 0;
    proxy.events().on("sourceModelChanged", [&]() { ++fired; });

    // Setting the same model again must not fire
    proxy.setSourceModel(&source);
    EXPECT_EQ(fired, 0);
}

TEST(TzAbstractProxyModel, DataDelegation)
{
    ProxyTestSourceModel source(3);
    source.setData(source.index(1, 0), std::any{42}, TzItemDataRole::DisplayRole);
    IdentityProxy proxy;
    proxy.setSourceModel(&source);

    std::any v = proxy.data(proxy.index(1, 0));
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(std::any_cast<int>(v), 42);
}

TEST(TzAbstractProxyModel, SetDataDelegation)
{
    ProxyTestSourceModel source(3);
    IdentityProxy proxy;
    proxy.setSourceModel(&source);

    EXPECT_TRUE(proxy.setData(proxy.index(0, 0), std::any{99}));

    std::any v = source.data(source.index(0, 0), TzItemDataRole::DisplayRole);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(std::any_cast<int>(v), 99);
}

TEST(TzAbstractProxyModel, FlagsDelegation)
{
    ProxyTestSourceModel source(2);
    IdentityProxy proxy;
    proxy.setSourceModel(&source);

    TzItemFlags f = proxy.flags(proxy.index(0, 0));
    EXPECT_TRUE(f.testFlag(TzItemFlag::ItemIsSelectable));
    EXPECT_TRUE(f.testFlag(TzItemFlag::ItemIsEnabled));
}

TEST(TzAbstractProxyModel, RoleNamesDelegation)
{
    ProxyTestSourceModel source;
    IdentityProxy proxy;
    proxy.setSourceModel(&source);

    auto names = proxy.roleNames();
    EXPECT_EQ(names.at(TzItemDataRole::DisplayRole), "display");
}

TEST(TzAbstractProxyModel, SubmitRevert)
{
    ProxyTestSourceModel source(1);
    IdentityProxy proxy;
    proxy.setSourceModel(&source);

    EXPECT_TRUE(proxy.submit());
    proxy.revert(); // no-op, just verify no crash
}

TEST(TzAbstractProxyModel, ForwardsRowsInserted)
{
    ProxyTestSourceModel source(2);
    IdentityProxy proxy;
    proxy.setSourceModel(&source);

    // The proxy does NOT automatically re-emit source signals in this minimal
    // identity implementation, but the source signals are still reachable via
    // the source model's.events().  More importantly, the private slots that
    // track header-update state are exercised here.
    int rowCount = proxy.rowCount();
    source.appendRows(3);
    EXPECT_EQ(proxy.rowCount(), rowCount + 3);
}

TEST(TzAbstractProxyModel, ForwardsRowsRemoved)
{
    ProxyTestSourceModel source(5);
    IdentityProxy proxy;
    proxy.setSourceModel(&source);

    source.removeRows(1, 3);
    EXPECT_EQ(proxy.rowCount(), 2);
}

TEST(TzAbstractProxyModel, OldSourceListenersDisconnected)
{
    ProxyTestSourceModel s1(4), s2(0);
    IdentityProxy proxy;
    proxy.setSourceModel(&s1);
    proxy.setSourceModel(&s2); // replaces s1

    // s1's.events() should have no listeners for the proxy's private slots
    // (they were disconnected).  The easiest observable effect: rowCount
    // reflects s2, not s1.
    s1.appendRows(10); // old source grows, but proxy is bound to s2
    EXPECT_EQ(proxy.rowCount(), 0); // still s2's count
    EXPECT_EQ(proxy.sourceModel(), &s2);
}

TEST(TzAbstractProxyModel, MapRoundTrip)
{
    ProxyTestSourceModel source(5);
    IdentityProxy proxy;
    proxy.setSourceModel(&source);

    for (int r = 0; r < 5; ++r) {
        TzModelIndex proxyIdx  = proxy.index(r, 0);
        TzModelIndex sourceIdx = proxy.mapToSource(proxyIdx);
        TzModelIndex backIdx   = proxy.mapFromSource(sourceIdx);

        EXPECT_EQ(sourceIdx.row(), r);
        EXPECT_EQ(backIdx.row(),   r);
    }
}

TEST(TzAbstractProxyModel, MapSelectionToSource)
{
    ProxyTestSourceModel source(4);
    IdentityProxy proxy;
    proxy.setSourceModel(&source);

    TzItemSelection proxySel;
    proxySel << TzItemSelectionRange(proxy.index(1, 0))
             << TzItemSelectionRange(proxy.index(3, 0));

    TzItemSelection sourceSel = proxy.mapSelectionToSource(proxySel);
    TzModelIndexList srcIdx = sourceSel.indexes();

    ASSERT_EQ(srcIdx.size(), 2u);
    EXPECT_EQ(srcIdx[0].row(), 1);
    EXPECT_EQ(srcIdx[1].row(), 3);
}

TEST(TzAbstractProxyModel, MapSelectionFromSource)
{
    ProxyTestSourceModel source(4);
    IdentityProxy proxy;
    proxy.setSourceModel(&source);

    TzItemSelection sourceSel;
    sourceSel << TzItemSelectionRange(source.index(0, 0))
              << TzItemSelectionRange(source.index(2, 0));

    TzItemSelection proxySel = proxy.mapSelectionFromSource(sourceSel);
    TzModelIndexList proxyIdx = proxySel.indexes();

    ASSERT_EQ(proxyIdx.size(), 2u);
    EXPECT_EQ(proxyIdx[0].row(), 0);
    EXPECT_EQ(proxyIdx[1].row(), 2);
}

TEST(TzAbstractProxyModel, CanFetchMoreDelegation)
{
    ProxyTestSourceModel source(2);
    IdentityProxy proxy;
    proxy.setSourceModel(&source);
    // Default implementation always returns false
    EXPECT_FALSE(proxy.canFetchMore(TzModelIndex()));
}

TEST(TzAbstractProxyModel, BuddyDelegation)
{
    ProxyTestSourceModel source(3);
    IdentityProxy proxy;
    proxy.setSourceModel(&source);

    TzModelIndex idx    = proxy.index(1, 0);
    TzModelIndex buddy  = proxy.buddy(idx);
    // Default buddy() returns same index mapped back
    EXPECT_EQ(buddy.row(), 1);
}

TEST(TzAbstractProxyModel, ScopedListenerOnProxy)
{
    ProxyTestSourceModel s1(1), s2(1);
    IdentityProxy proxy;

    int fired = 0;
    {
        TzScopedEventListener scoped{
            proxy.events().on("sourceModelChanged", [&]() { ++fired; })
        };
        proxy.setSourceModel(&s1);
        EXPECT_EQ(fired, 1);
    } // scoped destructs → auto-disconnect

    proxy.setSourceModel(&s2);
    EXPECT_EQ(fired, 1); // must not increment after disconnect
}
