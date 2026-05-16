#include <gtest/gtest.h>
#include <models/tzbreadcrumbmodel.h>

// Builds this tree:
//   root (folder)
//   ├── folderA (folder)
//   │   ├── itemA1 (item)
//   │   └── itemA2 (item)
//   └── folderB (folder)
//       └── itemB1 (item)
struct SimpleTree {
    TzTreeNodePtr root;
    TzTreeNodePtr folderA;
    TzTreeNodePtr folderB;
    TzTreeNodePtr itemA1;
    TzTreeNodePtr itemA2;
    TzTreeNodePtr itemB1;

    SimpleTree()
        : root(TzTreeNode::createFolder("root"))
        , folderA(TzTreeNode::createFolder("folderA"))
        , folderB(TzTreeNode::createFolder("folderB"))
        , itemA1(TzTreeNode::createItem("itemA1"))
        , itemA2(TzTreeNode::createItem("itemA2"))
        , itemB1(TzTreeNode::createItem("itemB1"))
    {
        root->addChild(folderA);
        root->addChild(folderB);
        folderA->addChild(itemA1);
        folderA->addChild(itemA2);
        folderB->addChild(itemB1);
    }
};

TEST(TzBreadcrumbModel, InitialCurrentNodeIsRoot)
{
    SimpleTree t;
    TzBreadcrumbModel model(t.root);
    EXPECT_EQ(model.currentNode(), t.root);
}

TEST(TzBreadcrumbModel, RootAccessor)
{
    SimpleTree t;
    TzBreadcrumbModel model(t.root);
    EXPECT_EQ(model.root(), t.root);
}

TEST(TzBreadcrumbModel, PathAtRootHasSingleEntry)
{
    SimpleTree t;
    TzBreadcrumbModel model(t.root);
    auto p = model.path();
    ASSERT_EQ(p.size(), 1u);
    EXPECT_EQ(p[0], t.root);
}

TEST(TzBreadcrumbModel, PathAfterNavigatingToChild)
{
    SimpleTree t;
    TzBreadcrumbModel model(t.root);
    model.navigateTo(t.folderA);

    auto p = model.path();
    ASSERT_EQ(p.size(), 2u);
    EXPECT_EQ(p[0], t.root);
    EXPECT_EQ(p[1], t.folderA);
}

TEST(TzBreadcrumbModel, PathAfterNavigatingToGrandchild)
{
    SimpleTree t;
    TzBreadcrumbModel model(t.root);
    model.navigateTo(t.itemA1);

    auto p = model.path();
    ASSERT_EQ(p.size(), 3u);
    EXPECT_EQ(p[0], t.root);
    EXPECT_EQ(p[1], t.folderA);
    EXPECT_EQ(p[2], t.itemA1);
}

TEST(TzBreadcrumbModel, NavigateToValidNode)
{
    SimpleTree t;
    TzBreadcrumbModel model(t.root);
    EXPECT_TRUE(model.navigateTo(t.folderB));
    EXPECT_EQ(model.currentNode(), t.folderB);
}

TEST(TzBreadcrumbModel, NavigateToNullReturnsFalse)
{
    SimpleTree t;
    TzBreadcrumbModel model(t.root);
    EXPECT_FALSE(model.navigateTo(nullptr));
    EXPECT_EQ(model.currentNode(), t.root);
}

TEST(TzBreadcrumbModel, NavigateToNodeNotInTreeReturnsFalse)
{
    SimpleTree t;
    TzBreadcrumbModel model(t.root);
    auto orphan = TzTreeNode::createItem("orphan");
    EXPECT_FALSE(model.navigateTo(orphan));
    EXPECT_EQ(model.currentNode(), t.root);
}

TEST(TzBreadcrumbModel, NavigateToEmitsCurrentNodeChanged)
{
    SimpleTree t;
    TzBreadcrumbModel model(t.root);

    TzTreeNodePtr gotNew, gotOld;
    model.events().on("currentNodeChanged",
        [&](TzTreeNodePtr newNode, TzTreeNodePtr oldNode) {
            gotNew = newNode;
            gotOld = oldNode;
        });

    model.navigateTo(t.itemA2);
    EXPECT_EQ(gotNew, t.itemA2);
    EXPECT_EQ(gotOld, t.root);
}

TEST(TzBreadcrumbModel, NavigateToDoesNotEmitOnFailure)
{
    SimpleTree t;
    TzBreadcrumbModel model(t.root);

    int fired = 0;
    model.events().on("currentNodeChanged",
        [&](TzTreeNodePtr, TzTreeNodePtr) { ++fired; });

    model.navigateTo(nullptr);
    EXPECT_EQ(fired, 0);
}

TEST(TzBreadcrumbModel, NavigateUpFromGrandchild)
{
    SimpleTree t;
    TzBreadcrumbModel model(t.root);
    model.navigateTo(t.itemA1);

    EXPECT_TRUE(model.navigateUp());
    EXPECT_EQ(model.currentNode(), t.folderA);

    EXPECT_TRUE(model.navigateUp());
    EXPECT_EQ(model.currentNode(), t.root);
}

TEST(TzBreadcrumbModel, NavigateUpAtRootReturnsFalse)
{
    SimpleTree t;
    TzBreadcrumbModel model(t.root);
    EXPECT_FALSE(model.navigateUp());
    EXPECT_EQ(model.currentNode(), t.root);
}

TEST(TzBreadcrumbModel, NavigateToRootFromDeep)
{
    SimpleTree t;
    TzBreadcrumbModel model(t.root);
    model.navigateTo(t.itemB1);

    model.navigateToRoot();
    EXPECT_EQ(model.currentNode(), t.root);
    EXPECT_EQ(model.path().size(), 1u);
}

TEST(TzBreadcrumbModel, NavigateToRootWhenAlreadyAtRootIsNoop)
{
    SimpleTree t;
    TzBreadcrumbModel model(t.root);

    int fired = 0;
    model.events().on("currentNodeChanged",
        [&](TzTreeNodePtr, TzTreeNodePtr) { ++fired; });

    model.navigateToRoot();
    EXPECT_EQ(fired, 0);
    EXPECT_EQ(model.currentNode(), t.root);
}

TEST(TzBreadcrumbModel, SiblingsOfRootIsEmpty)
{
    SimpleTree t;
    TzBreadcrumbModel model(t.root);
    EXPECT_TRUE(model.siblingsOf(t.root).empty());
}

TEST(TzBreadcrumbModel, SiblingsOfChildIncludesAllChildren)
{
    SimpleTree t;
    TzBreadcrumbModel model(t.root);

    auto siblings = model.siblingsOf(t.folderA);
    ASSERT_EQ(siblings.size(), 2u);
    EXPECT_EQ(siblings[0], t.folderA);
    EXPECT_EQ(siblings[1], t.folderB);
}

TEST(TzBreadcrumbModel, SiblingsOfGrandchild)
{
    SimpleTree t;
    TzBreadcrumbModel model(t.root);

    auto siblings = model.siblingsOf(t.itemA2);
    ASSERT_EQ(siblings.size(), 2u);
    EXPECT_EQ(siblings[0], t.itemA1);
    EXPECT_EQ(siblings[1], t.itemA2);
}

TEST(TzBreadcrumbModel, SiblingsOfNullIsEmpty)
{
    SimpleTree t;
    TzBreadcrumbModel model(t.root);
    EXPECT_TRUE(model.siblingsOf(nullptr).empty());
}

TEST(TzBreadcrumbModel, NavigateBetweenBranches)
{
    SimpleTree t;
    TzBreadcrumbModel model(t.root);

    model.navigateTo(t.itemA1);
    EXPECT_EQ(model.currentNode(), t.itemA1);

    model.navigateTo(t.itemB1);
    EXPECT_EQ(model.currentNode(), t.itemB1);

    auto p = model.path();
    ASSERT_EQ(p.size(), 3u);
    EXPECT_EQ(p[0], t.root);
    EXPECT_EQ(p[1], t.folderB);
    EXPECT_EQ(p[2], t.itemB1);
}

TEST(TzBreadcrumbModel, EventFiredForEachNavigation)
{
    SimpleTree t;
    TzBreadcrumbModel model(t.root);

    int count = 0;
    model.events().on("currentNodeChanged",
        [&](TzTreeNodePtr, TzTreeNodePtr) { ++count; });

    model.navigateTo(t.folderA);
    model.navigateTo(t.itemA1);
    model.navigateUp();
    model.navigateToRoot();
    EXPECT_EQ(count, 4);
}
