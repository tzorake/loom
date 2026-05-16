#include <gtest/gtest.h>
#include <models/tztreenode.h>
#include <models/tztreemodel.h>
#include <string>
#include <any>

TEST(TzTreeNode, CreateItem)
{
    auto node = TzTreeNode::createItem("file.txt");
    EXPECT_EQ(node->name(), "file.txt");
    EXPECT_TRUE(node->isItem());
    EXPECT_FALSE(node->isFolder());
    EXPECT_EQ(node->type(), TzTreeNode::Type::Item);
}

TEST(TzTreeNode, CreateFolder)
{
    auto folder = TzTreeNode::createFolder("docs");
    EXPECT_EQ(folder->name(), "docs");
    EXPECT_TRUE(folder->isFolder());
    EXPECT_FALSE(folder->isItem());
    EXPECT_EQ(folder->type(), TzTreeNode::Type::Folder);
}

TEST(TzTreeNode, SetName)
{
    auto node = TzTreeNode::createItem("old");
    EXPECT_TRUE(node->setName("new"));
    EXPECT_EQ(node->name(), "new");
    EXPECT_FALSE(node->setName("new")); // same name — no change
}

TEST(TzTreeNode, SetNameEmitsEvent)
{
    auto node = TzTreeNode::createItem("a");
    std::string received;
    node->events().on("nameChanged", [&](const std::string& name) { received = name; });
    node->setName("b");
    EXPECT_EQ(received, "b");
}

TEST(TzTreeNode, AddChild)
{
    auto parent = TzTreeNode::createFolder("root");
    auto child  = TzTreeNode::createItem("child");

    EXPECT_EQ(parent->childCount(), 0);
    EXPECT_TRUE(parent->addChild(child));
    EXPECT_EQ(parent->childCount(), 1);
    EXPECT_EQ(parent->childAt(0), child);
    EXPECT_EQ(child->parent(), parent);
}

TEST(TzTreeNode, AddNullChildReturnsFalse)
{
    auto parent = TzTreeNode::createFolder("root");
    EXPECT_FALSE(parent->addChild(nullptr));
    EXPECT_EQ(parent->childCount(), 0);
}

TEST(TzTreeNode, InsertChild)
{
    auto parent = TzTreeNode::createFolder("root");
    auto a = TzTreeNode::createItem("a");
    auto b = TzTreeNode::createItem("b");
    auto c = TzTreeNode::createItem("c");

    parent->addChild(a);
    parent->addChild(c);
    EXPECT_TRUE(parent->insertChild(1, b));

    EXPECT_EQ(parent->childCount(), 3);
    EXPECT_EQ(parent->childAt(0), a);
    EXPECT_EQ(parent->childAt(1), b);
    EXPECT_EQ(parent->childAt(2), c);
}

TEST(TzTreeNode, InsertChildOutOfBoundsReturnsFalse)
{
    auto parent = TzTreeNode::createFolder("root");
    auto child  = TzTreeNode::createItem("x");
    EXPECT_FALSE(parent->insertChild(5, child));
    EXPECT_FALSE(parent->insertChild(-1, child));
}

TEST(TzTreeNode, RemoveChild)
{
    auto parent = TzTreeNode::createFolder("root");
    auto child  = TzTreeNode::createItem("child");

    parent->addChild(child);
    EXPECT_EQ(parent->childCount(), 1);

    EXPECT_TRUE(parent->removeChild(child));
    EXPECT_EQ(parent->childCount(), 0);
    EXPECT_EQ(child->parent(), nullptr);
}

TEST(TzTreeNode, RemoveChildNotPresentReturnsFalse)
{
    auto parent  = TzTreeNode::createFolder("root");
    auto missing = TzTreeNode::createItem("x");
    EXPECT_FALSE(parent->removeChild(missing));
}

TEST(TzTreeNode, IndexOf)
{
    auto parent = TzTreeNode::createFolder("root");
    auto a = TzTreeNode::createItem("a");
    auto b = TzTreeNode::createItem("b");

    parent->addChild(a);
    parent->addChild(b);

    EXPECT_EQ(parent->indexOf(a), 0);
    EXPECT_EQ(parent->indexOf(b), 1);
    EXPECT_EQ(parent->indexOf(TzTreeNode::createItem("missing")), -1);
}

TEST(TzTreeNode, ChildAt)
{
    auto parent = TzTreeNode::createFolder("root");
    auto child  = TzTreeNode::createItem("only");
    parent->addChild(child);

    EXPECT_EQ(parent->childAt(0), child);
    EXPECT_EQ(parent->childAt(99), nullptr);
}

TEST(TzTreeNode, SetAndGetData)
{
    auto node = TzTreeNode::createItem("node");
    node->setData(std::string("hello"));

    auto val = node->unwrap<std::string>();
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, "hello");
}

TEST(TzTreeNode, UnwrapWrongType)
{
    auto node = TzTreeNode::createItem("node");
    node->setData(42);
    EXPECT_FALSE(node->unwrap<std::string>().has_value());
    EXPECT_TRUE(node->unwrap<int>().has_value());
}

TEST(TzTreeNode, DataChangedEvent)
{
    auto node = TzTreeNode::createItem("node");
    int fired = 0;
    node->events().on("dataChanged", [&] { ++fired; });
    node->setData(std::string("x"));
    EXPECT_EQ(fired, 1);
}

TEST(TzTreeNode, AddChildEmitsEvent)
{
    auto parent = TzTreeNode::createFolder("root");
    TzTreeNodePtr received;
    int receivedIndex = -1;
    parent->events().on("childAdded", [&](TzTreeNodePtr child, int index) {
        received = child;
        receivedIndex = index;
    });

    auto child = TzTreeNode::createItem("child");
    parent->addChild(child);

    EXPECT_EQ(received, child);
    EXPECT_EQ(receivedIndex, 0);
}

TEST(TzTreeNode, RemoveChildEmitsEvent)
{
    auto parent = TzTreeNode::createFolder("root");
    auto child  = TzTreeNode::createItem("child");
    parent->addChild(child);

    TzTreeNodePtr removed;
    int removedIndex = -1;
    parent->events().on("childAboutToBeRemoved", [&](TzTreeNodePtr c, int index) {
        removed = c;
        removedIndex = index;
    });

    parent->removeChild(child);
    EXPECT_EQ(removed, child);
    EXPECT_EQ(removedIndex, 0);
}

TEST(TzTreeModel, ConstructWithSingleNode)
{
    auto node  = TzTreeNode::createItem("item");
    TzTreeModel model(node);

    EXPECT_EQ(model.rowCount(), 1);
    EXPECT_EQ(model.columnCount(), 1);
}

TEST(TzTreeModel, ConstructWithMultipleNodes)
{
    auto a = TzTreeNode::createItem("a");
    auto b = TzTreeNode::createItem("b");
    auto c = TzTreeNode::createItem("c");
    TzTreeModel model({a, b, c});

    EXPECT_EQ(model.rowCount(), 3);
}

TEST(TzTreeModel, IndexValid)
{
    auto node = TzTreeNode::createItem("item");
    TzTreeModel model(node);

    TzModelIndex idx = model.index(0, 0);
    EXPECT_TRUE(idx.isValid());
    EXPECT_EQ(idx.row(), 0);
    EXPECT_EQ(idx.column(), 0);
}

TEST(TzTreeModel, IndexOutOfRange)
{
    auto node = TzTreeNode::createItem("item");
    TzTreeModel model(node);

    EXPECT_FALSE(model.index(1, 0).isValid());
    EXPECT_FALSE(model.index(0, 1).isValid());
    EXPECT_FALSE(model.index(-1, 0).isValid());
}

TEST(TzTreeModel, DataReturnsName)
{
    auto node = TzTreeNode::createItem("hello");
    TzTreeModel model(node);

    TzModelIndex idx = model.index(0, 0);
    auto val = model.data(idx, TzItemDataRole::Display);
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(std::any_cast<std::string>(val), "hello");
}

TEST(TzTreeModel, DataInvalidIndexReturnsEmpty)
{
    auto node = TzTreeNode::createItem("x");
    TzTreeModel model(node);

    EXPECT_FALSE(model.data(TzModelIndex()).has_value());
}

TEST(TzTreeModel, SetDataRenamesNode)
{
    auto node = TzTreeNode::createItem("old");
    TzTreeModel model(node);

    TzModelIndex idx = model.index(0, 0);
    EXPECT_TRUE(model.setData(idx, std::string("new"), TzItemDataRole::Edit));
    EXPECT_EQ(node->name(), "new");
}

TEST(TzTreeModel, NodeForIndex)
{
    auto node = TzTreeNode::createItem("x");
    TzTreeModel model(node);

    TzModelIndex idx = model.index(0, 0);
    EXPECT_EQ(model.nodeForIndex(idx), node);
}

TEST(TzTreeModel, IndexForNode)
{
    auto node = TzTreeNode::createItem("x");
    TzTreeModel model(node);

    TzModelIndex idx = model.indexForNode(node);
    EXPECT_TRUE(idx.isValid());
    EXPECT_EQ(idx.row(), 0);
}

TEST(TzTreeModel, IndexForInvisibleRootIsInvalid)
{
    auto node = TzTreeNode::createItem("x");
    TzTreeModel model(node);

    EXPECT_FALSE(model.indexForNode(model.invisibleRoot()).isValid());
}

TEST(TzTreeModel, ChildRowCount)
{
    auto folder = TzTreeNode::createFolder("folder");
    folder->addChild(TzTreeNode::createItem("a"));
    folder->addChild(TzTreeNode::createItem("b"));

    TzTreeModel model(folder);

    TzModelIndex folderIdx = model.index(0, 0);
    EXPECT_EQ(model.rowCount(folderIdx), 2);
}

TEST(TzTreeModel, ParentIndex)
{
    auto folder = TzTreeNode::createFolder("folder");
    auto child  = TzTreeNode::createItem("child");
    folder->addChild(child);

    TzTreeModel model(folder);

    TzModelIndex folderIdx = model.index(0, 0);
    TzModelIndex childIdx = model.index(0, 0, folderIdx);
    EXPECT_TRUE(childIdx.isValid());

    TzModelIndex parentIdx = model.parent(childIdx);
    EXPECT_TRUE(parentIdx.isValid());
    EXPECT_EQ(parentIdx.row(), folderIdx.row());
}

TEST(TzTreeModel, AddChildUpdatesRowCount)
{
    auto folder = TzTreeNode::createFolder("folder");
    TzTreeModel model(folder);

    TzModelIndex folderIdx = model.index(0, 0);
    EXPECT_EQ(model.rowCount(folderIdx), 0);

    folder->addChild(TzTreeNode::createItem("new"));
    EXPECT_EQ(model.rowCount(folderIdx), 1);
}

TEST(TzTreeModel, RemoveChildUpdatesRowCount)
{
    auto folder = TzTreeNode::createFolder("folder");
    auto child  = TzTreeNode::createItem("child");
    folder->addChild(child);

    TzTreeModel model(folder);
    TzModelIndex folderIdx = model.index(0, 0);
    EXPECT_EQ(model.rowCount(folderIdx), 1);

    folder->removeChild(child);
    EXPECT_EQ(model.rowCount(folderIdx), 0);
}

TEST(TzTreeModel, AddChildEmitsRowsInserted)
{
    auto folder = TzTreeNode::createFolder("folder");
    TzTreeModel model(folder);

    int insertedFirst = -1, insertedLast = -1;
    model.events().on("rowsInserted", [&](TzModelIndex, int first, int last) {
        insertedFirst = first;
        insertedLast = last;
    });

    folder->addChild(TzTreeNode::createItem("new"));
    EXPECT_EQ(insertedFirst, 0);
    EXPECT_EQ(insertedLast, 0);
}

TEST(TzTreeModel, RemoveChildEmitsRowsRemoved)
{
    auto folder = TzTreeNode::createFolder("folder");
    auto child  = TzTreeNode::createItem("child");
    folder->addChild(child);

    TzTreeModel model(folder);

    int removedFirst = -1, removedLast = -1;
    model.events().on("rowsRemoved", [&](TzModelIndex, int first, int last) {
        removedFirst = first;
        removedLast = last;
    });

    folder->removeChild(child);
    EXPECT_EQ(removedFirst, 0);
    EXPECT_EQ(removedLast, 0);
}
