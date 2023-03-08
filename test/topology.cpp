#include "gtest/gtest.h"

#include "sys-sage.hpp"

TEST(Components, Node)
{
    Node node{42};
    EXPECT_EQ(42, node.GetId());
    EXPECT_EQ(SYS_SAGE_COMPONENT_NODE, node.GetComponentType());
    EXPECT_EQ("Node", node.GetComponentTypeStr());
    // TODO: Parent should be nullified
    // EXPECT_EQ(nullptr, node.GetParent());

    Node root{0};
    node.SetParent(&root);
    EXPECT_EQ(&root, node.GetParent());
}

TEST(Components, Topology)
{
    Topology node;
    EXPECT_EQ(0, node.GetId());
    EXPECT_EQ(SYS_SAGE_COMPONENT_TOPOLOGY, node.GetComponentType());
    EXPECT_EQ("Topology", node.GetComponentTypeStr());
}

TEST(Components, Thread)
{
    Node root{0};
    Thread node{&root, 42, "foo"};
    EXPECT_EQ(&root, node.GetParent());
    EXPECT_EQ(42, node.GetId());
    EXPECT_EQ("foo", node.GetName());
    EXPECT_EQ(SYS_SAGE_COMPONENT_THREAD, node.GetComponentType());
    EXPECT_EQ("HW_thread", node.GetComponentTypeStr());
}

TEST(Components, Core)
{
    Node root{0};
    Core node{&root, 42, "foo"};
    EXPECT_EQ(&root, node.GetParent());
    EXPECT_EQ(42, node.GetId());
    EXPECT_EQ("foo", node.GetName());
    EXPECT_EQ(SYS_SAGE_COMPONENT_CORE, node.GetComponentType());
    EXPECT_EQ("Core", node.GetComponentTypeStr());
}

TEST(Components, Cache)
{
    Node root{0};
    Cache node{&root, 42, "3", 32, 2, 16};
    EXPECT_EQ(&root, node.GetParent());
    EXPECT_EQ(42, node.GetId());
    EXPECT_EQ("Cache", node.GetName());
    EXPECT_EQ("3", node.GetCacheName());
    EXPECT_EQ(3, node.GetCacheLevel());
    EXPECT_EQ(32, node.GetCacheSize());
    EXPECT_EQ(2, node.GetCacheAssociativityWays());
    EXPECT_EQ(16, node.GetCacheLineSize());
    EXPECT_EQ(SYS_SAGE_COMPONENT_CACHE, node.GetComponentType());
    EXPECT_EQ("Cache", node.GetComponentTypeStr());

    node.SetCacheSize(16);
    EXPECT_EQ(16, node.GetCacheSize());

    node.SetCacheLineSize(8);
    EXPECT_EQ(8, node.GetCacheLineSize());
}

TEST(Components, Subdivision)
{
    Node root{0};
    Subdivision node{&root, 42, "foo"};
    EXPECT_EQ(&root, node.GetParent());
    EXPECT_EQ(42, node.GetId());
    EXPECT_EQ("foo", node.GetName());
    EXPECT_EQ(SYS_SAGE_COMPONENT_SUBDIVISION, node.GetComponentType());
    EXPECT_EQ("Subdivision", node.GetComponentTypeStr());

    node.SetSubdivisionType(3);
    EXPECT_EQ(3, node.GetSubdivisionType());
}

TEST(Components, Numa)
{
    Node root{0};
    Numa node{&root, 42, 64};
    EXPECT_EQ(&root, node.GetParent());
    EXPECT_EQ(42, node.GetId());
    EXPECT_EQ("Numa", node.GetName());
    EXPECT_EQ(SYS_SAGE_COMPONENT_NUMA, node.GetComponentType());
    EXPECT_EQ("NUMA", node.GetComponentTypeStr());

    node.SetSubdivisionType(3);
    EXPECT_EQ(3, node.GetSubdivisionType());
}

TEST(Components, Chip)
{
    Node root{0};
    Chip node{&root, 42, "foo", 5};
    EXPECT_EQ(&root, node.GetParent());
    EXPECT_EQ(42, node.GetId());
    EXPECT_EQ("foo", node.GetName());
    EXPECT_EQ(SYS_SAGE_COMPONENT_CHIP, node.GetComponentType());
    EXPECT_EQ("Chip", node.GetComponentTypeStr());
    EXPECT_EQ(5, node.GetChipType());

    node.SetModel("model");
    EXPECT_EQ("model", node.GetModel());

    node.SetVendor("vendor");
    EXPECT_EQ("vendor", node.GetVendor());

    node.SetChipType(6);
    EXPECT_EQ(6, node.GetChipType());
}

TEST(Components, Memory)
{
    Node root{0};
    Memory node{&root, "foo", 32};
    EXPECT_EQ(&root, node.GetParent());
    EXPECT_EQ(0, node.GetId());
    EXPECT_EQ("foo", node.GetName());
    EXPECT_EQ(32, node.GetSize());
    EXPECT_EQ(SYS_SAGE_COMPONENT_MEMORY, node.GetComponentType());
    EXPECT_EQ("Memory", node.GetComponentTypeStr());

    node.SetSize(64);
    EXPECT_EQ(64, node.GetSize());
}

TEST(Components, Storage)
{
    Node root{0};
    Storage node{&root};
    EXPECT_EQ(&root, node.GetParent());
    EXPECT_EQ(SYS_SAGE_COMPONENT_STORAGE, node.GetComponentType());
    EXPECT_EQ("Storage", node.GetComponentTypeStr());

    node.SetSize(64);
    EXPECT_EQ(64, node.GetSize());
}

TEST(Components, ChildrenInsertionAndRemoval)
{
    Node a;
    Node b;
    Node c;
    Node d;

    a.InsertChild(&b);
    a.InsertChild(&c);
    a.InsertChild(&d);
    ASSERT_EQ(3, a.GetChildren()->size());

    ASSERT_EQ(1, a.RemoveChild(&b));
    ASSERT_EQ(2, a.GetChildren()->size());
    ASSERT_EQ(std::find(a.GetChildren()->begin(), a.GetChildren()->end(), &b), a.GetChildren()->end());
    ASSERT_NE(std::find(a.GetChildren()->begin(), a.GetChildren()->end(), &c), a.GetChildren()->end());
    ASSERT_NE(std::find(a.GetChildren()->begin(), a.GetChildren()->end(), &d), a.GetChildren()->end());
}

TEST(Components, GetChild)
{
    Node a{1};
    Node b{2};
    Node c{3};
    Node d{4};

    a.InsertChild(&b);
    a.InsertChild(&c);
    b.InsertChild(&d);

    EXPECT_EQ(a.GetChild(2), &b);
    EXPECT_EQ(a.GetChild(3), &c);
    EXPECT_EQ(a.GetChild(4), nullptr);
}

TEST(Components, GetChildByType)
{
    Node a;
    Memory b;
    Memory c;
    Chip d;

    a.InsertChild(&b);
    a.InsertChild(&c);
    a.InsertChild(&d);

    EXPECT_EQ(a.GetChildByType(SYS_SAGE_COMPONENT_MEMORY), &b);
    EXPECT_EQ(a.GetChildByType(SYS_SAGE_COMPONENT_CHIP), &d);
    EXPECT_EQ(a.GetAllChildrenByType(SYS_SAGE_COMPONENT_MEMORY), (std::vector<Component *>{&b, &c}));
}

TEST(Components, CheckComponentTreeConsistency)
{
    {
        Node a;
        Node b;
        Node c;
        a.InsertChild(&b);
        b.InsertChild(&c);
        EXPECT_EQ(0, a.CheckComponentTreeConsistency());
    }
    {
        Node a;
        Node b;
        Node c;
        a.InsertChild(&b);
        b.InsertChild(&c);
        c.SetParent(&a);
        EXPECT_EQ(1, a.CheckComponentTreeConsistency());
    }
}

TEST(Components, GetComponentsNLevelsDeeper)
{
    Node a;
    Node b;
    Node c;
    Node d;

    a.InsertChild(&b);
    a.InsertChild(&c);
    c.InsertChild(&d);

    std::vector<Component *> array;
    a.GetComponentsNLevelsDeeper(&array, 1);
    EXPECT_EQ(2, array.size());
}

TEST(Components, GetSubcomponentsByType)
{
    Node a;
    Chip b;
    Memory c;
    Chip d;

    a.InsertChild(&b);
    a.InsertChild(&c);
    c.InsertChild(&d);

    std::vector<Component *> array;
    a.GetSubcomponentsByType(&array, SYS_SAGE_COMPONENT_CHIP);
    EXPECT_EQ(2, array.size());
}

TEST(Components, GetNumThreads)
{
    Node a;
    Thread b;
    Thread c;
    Node d;
    Thread e;
    Node f;

    a.InsertChild(&b);
    a.InsertChild(&c);
    a.InsertChild(&d);
    d.InsertChild(&e);
    d.InsertChild(&f);

    EXPECT_EQ(3, a.GetNumThreads());
}

TEST(Components, GetSubtreeNodeList)
{
    Node a;
    Node b;
    Node c;
    Node d;

    a.InsertChild(&b);
    b.InsertChild(&d);
    a.InsertChild(&c);

    std::vector<Component *> array;
    a.GetSubtreeNodeList(&array);
    EXPECT_EQ(array, (std::vector<Component *>{&a, &b, &d, &c}));
}

TEST(Components, GetTopoTreeDepth)
{
    Node a;
    Node b;
    Node c;
    Node d;
    Node e;
    Node f;
    Node g;

    a.InsertChild(&b);
    b.InsertChild(&c);
    a.InsertChild(&d);
    d.InsertChild(&e);
    e.InsertChild(&f);
    a.InsertChild(&g);

    EXPECT_EQ(3, a.GetTopoTreeDepth());
}
