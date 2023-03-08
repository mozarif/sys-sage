#include "gtest/gtest.h"

#include "sys-sage.hpp"

TEST(CapsNumaBenchmark, parseCapsNumaBenchmark)
{
    Topology topo;
    Node node{&topo};

    parseHwlocOutput(&node, SYS_SAGE_TEST_RESOURCE_DIR "/skylake_hwloc.xml");

    ASSERT_EQ(0, parseCapsNumaBenchmark(&node, SYS_SAGE_TEST_RESOURCE_DIR "/skylake_caps_numa_benchmark.csv"));

    std::vector<Component *> numas;
    node.GetSubcomponentsByType(&numas, SYS_SAGE_COMPONENT_NUMA);
    ASSERT_EQ(4, numas.size());

    for (const auto &numa : numas)
    {
        ASSERT_EQ(4, numa->GetDataPaths(SYS_SAGE_DATAPATH_INCOMING)->size());
        for (const auto &dp : *numa->GetDataPaths(SYS_SAGE_DATAPATH_INCOMING))
        {
            EXPECT_EQ(SYS_SAGE_DATAPATH_TYPE_PHYSICAL, dp->GetDpType());
            EXPECT_EQ(SYS_SAGE_DATAPATH_ORIENTED, dp->GetOriented());
        }

        ASSERT_EQ(4, numa->GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING)->size());
        for (const auto &dp : *numa->GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING))
        {
            EXPECT_EQ(SYS_SAGE_DATAPATH_TYPE_PHYSICAL, dp->GetDpType());
            EXPECT_EQ(SYS_SAGE_DATAPATH_ORIENTED, dp->GetOriented());
        }
    }

    for (size_t i = 0; i < 4; ++i)
    {
        for (size_t k = 0; k < 4; ++k)
        {
            auto dp1 = (*numas[i]->GetDataPaths(SYS_SAGE_DATAPATH_INCOMING))[k];
            auto dp2 = (*numas[k]->GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING))[i];
            EXPECT_EQ(dp1->GetBw(), dp2->GetBw());
            EXPECT_EQ(dp1->GetLatency(), dp2->GetLatency());
        }
    }

    auto dp = [&numas](size_t i, size_t k)
    {
        return (*numas[i]->GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING))[k];
    };

    EXPECT_EQ(8621, dp(0, 0)->GetBw());
    EXPECT_EQ(8502, dp(1, 0)->GetBw());
    EXPECT_EQ(6467, dp(2, 0)->GetBw());
    EXPECT_EQ(6476, dp(3, 0)->GetBw());
    EXPECT_EQ(244, dp(0, 0)->GetLatency());
    EXPECT_EQ(195, dp(1, 0)->GetLatency());
    EXPECT_EQ(307, dp(2, 0)->GetLatency());
    EXPECT_EQ(313, dp(3, 0)->GetLatency());

    EXPECT_EQ(8237, dp(0, 1)->GetBw());
    EXPECT_EQ(8663, dp(1, 1)->GetBw());
    EXPECT_EQ(6291, dp(2, 1)->GetBw());
    EXPECT_EQ(6267, dp(3, 1)->GetBw());
    EXPECT_EQ(203, dp(0, 1)->GetLatency());
    EXPECT_EQ(237, dp(1, 1)->GetLatency());
    EXPECT_EQ(315, dp(2, 1)->GetLatency());
    EXPECT_EQ(319, dp(3, 1)->GetLatency());

    EXPECT_EQ(6439, dp(0, 2)->GetBw());
    EXPECT_EQ(6609, dp(1, 2)->GetBw());
    EXPECT_EQ(8539, dp(2, 2)->GetBw());
    EXPECT_EQ(8412, dp(3, 2)->GetBw());
    EXPECT_EQ(302, dp(0, 2)->GetLatency());
    EXPECT_EQ(313, dp(1, 2)->GetLatency());
    EXPECT_EQ(237, dp(2, 2)->GetLatency());
    EXPECT_EQ(210, dp(3, 2)->GetLatency());

    EXPECT_EQ(6315, dp(0, 3)->GetBw());
    EXPECT_EQ(6324, dp(1, 3)->GetBw());
    EXPECT_EQ(8177, dp(2, 3)->GetBw());
    EXPECT_EQ(8466, dp(3, 3)->GetBw());
    EXPECT_EQ(309, dp(0, 3)->GetLatency());
    EXPECT_EQ(316, dp(1, 3)->GetLatency());
    EXPECT_EQ(211, dp(2, 3)->GetLatency());
    EXPECT_EQ(246, dp(3, 3)->GetLatency());
}
