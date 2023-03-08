#include "gtest/gtest.h"

#include "sys-sage.hpp"

TEST(Hwloc, parseHwlocOutput)
{
    Topology topo;
    Node node{&topo};
    ASSERT_EQ(0, parseHwlocOutput(&node, SYS_SAGE_TEST_RESOURCE_DIR "/skylake_hwloc.xml"));

    {
        std::vector<Component *> components;
        topo.GetSubcomponentsByType(&components, SYS_SAGE_COMPONENT_CHIP);
        EXPECT_EQ(2, components.size());
    }
    {
        std::vector<Component *> components;
        topo.GetSubcomponentsByType(&components, SYS_SAGE_COMPONENT_NUMA);
        EXPECT_EQ(4, components.size());
    }
    {
        std::vector<Component *> components;
        topo.GetSubcomponentsByType(&components, SYS_SAGE_COMPONENT_CACHE);
        EXPECT_EQ(50, components.size());
    }
    {
        std::vector<Component *> components;
        topo.GetSubcomponentsByType(&components, SYS_SAGE_COMPONENT_CORE);
        EXPECT_EQ(24, components.size());
    }
    {
        std::vector<Component *> components;
        topo.GetSubcomponentsByType(&components, SYS_SAGE_COMPONENT_THREAD);
        EXPECT_EQ(24, components.size());
    }

    auto chip = dynamic_cast<Chip *>(node.GetChildByType(SYS_SAGE_COMPONENT_CHIP));
    ASSERT_NE(nullptr, chip);
    EXPECT_EQ("GenuineIntel", chip->GetVendor());
    EXPECT_EQ("Intel(R) Xeon(R) Silver 4116 CPU @ 2.10GHz", chip->GetModel());

    auto cacheL3 = dynamic_cast<Cache *>(chip->GetChildByType(SYS_SAGE_COMPONENT_CACHE));
    ASSERT_NE(nullptr, cacheL3);
    EXPECT_EQ(3, cacheL3->GetCacheLevel());
    EXPECT_EQ(17301504, cacheL3->GetCacheSize());
    EXPECT_EQ(11, cacheL3->GetCacheAssociativityWays());
    EXPECT_EQ(64, cacheL3->GetCacheLineSize());

    auto numa = dynamic_cast<Numa *>(cacheL3->GetChildByType(SYS_SAGE_COMPONENT_NUMA));
    ASSERT_NE(nullptr, numa);

    auto cacheL2 = dynamic_cast<Cache *>(numa->GetChildByType(SYS_SAGE_COMPONENT_CACHE));
    ASSERT_NE(nullptr, cacheL2);
    EXPECT_EQ(2, cacheL2->GetCacheLevel());
    EXPECT_EQ(1048576, cacheL2->GetCacheSize());
    EXPECT_EQ(16, cacheL2->GetCacheAssociativityWays());
    EXPECT_EQ(64, cacheL2->GetCacheLineSize());

    auto cacheL1 = dynamic_cast<Cache *>(cacheL2->GetChildByType(SYS_SAGE_COMPONENT_CACHE));
    ASSERT_NE(nullptr, cacheL1);
    EXPECT_EQ(1, cacheL1->GetCacheLevel());
    EXPECT_EQ(32768, cacheL1->GetCacheSize());
    EXPECT_EQ(8, cacheL1->GetCacheAssociativityWays());
    EXPECT_EQ(64, cacheL1->GetCacheLineSize());

    auto core = dynamic_cast<Core *>(cacheL1->GetChildByType(SYS_SAGE_COMPONENT_CORE));
    ASSERT_NE(nullptr, core);

    auto thread = dynamic_cast<Thread *>(core->GetChildByType(SYS_SAGE_COMPONENT_THREAD));
    ASSERT_NE(nullptr, thread);
}