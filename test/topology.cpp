#include "gtest/gtest.h"

#include "sys-sage.hpp"

TEST(Components, Node)
{
    Node node{42};
    EXPECT_EQ(42, node.GetId());
    EXPECT_EQ(SYS_SAGE_COMPONENT_NODE, node.GetComponentType());
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
}

TEST(Components, Thread)
{
    Node root{0};
    Thread node{&root, 42, "foo"};
    EXPECT_EQ(&root, node.GetParent());
    EXPECT_EQ(42, node.GetId());
    EXPECT_EQ("foo", node.GetName());
    EXPECT_EQ(SYS_SAGE_COMPONENT_THREAD, node.GetComponentType());
}

TEST(Components, Core)
{
    Node root{0};
    Core node{&root, 42, "foo"};
    EXPECT_EQ(&root, node.GetParent());
    EXPECT_EQ(42, node.GetId());
    EXPECT_EQ("foo", node.GetName());
    EXPECT_EQ(SYS_SAGE_COMPONENT_CORE, node.GetComponentType());
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

    node.SetSize(64);
    EXPECT_EQ(64, node.GetSize());
}

TEST(Components, Storage)
{
    Node root{0};
    Storage node{&root};
    EXPECT_EQ(&root, node.GetParent());
    EXPECT_EQ(SYS_SAGE_COMPONENT_STORAGE, node.GetComponentType());

    // TODO: Enable when implementation is available
    // node.SetSize(64);
    // EXPECT_EQ(64, node.GetSize());
}
