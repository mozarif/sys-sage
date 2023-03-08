#include "gtest/gtest.h"

#include "sys-sage.hpp"

TEST(GpuTopo, parseGpuTopo)
{
    Topology topo;
    Chip gpu{&topo};
    ASSERT_EQ(0, parseGpuTopo(&gpu, SYS_SAGE_TEST_RESOURCE_DIR "/pascal_gpu_topo.csv"));

    {
        std::vector<Component *> components;
        topo.GetSubcomponentsByType(&components, SYS_SAGE_COMPONENT_MEMORY);
        EXPECT_EQ(1, components.size());
    }
    {
        std::vector<Component *> components;
        topo.GetSubcomponentsByType(&components, SYS_SAGE_COMPONENT_SUBDIVISION);
        EXPECT_EQ(30, components.size());
    }
    {
        std::vector<Component *> components;
        topo.GetSubcomponentsByType(&components, SYS_SAGE_COMPONENT_CACHE);
        EXPECT_EQ(151, components.size());
    }
    {
        std::vector<Component *> components;
        topo.GetSubcomponentsByType(&components, SYS_SAGE_COMPONENT_THREAD);
        EXPECT_EQ(3840, components.size());
    }

    EXPECT_EQ("Nvidia", gpu.GetVendor());
    EXPECT_EQ("Quadro P6000", gpu.GetModel());

    auto memory = dynamic_cast<Memory *>(gpu.GetChildByType(SYS_SAGE_COMPONENT_MEMORY));
    ASSERT_NE(nullptr, memory);
    EXPECT_EQ(25637224578, memory->GetSize());
    EXPECT_EQ(3840, memory->GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING)->size());

    auto cacheL2 = dynamic_cast<Cache *>(memory->GetChildByType(SYS_SAGE_COMPONENT_CACHE));
    ASSERT_NE(nullptr, cacheL2);
    EXPECT_EQ(3145728, cacheL2->GetCacheSize());
    EXPECT_EQ(32, cacheL2->GetCacheLineSize());

    auto subdivision = dynamic_cast<Subdivision *>(cacheL2->GetChildByType(SYS_SAGE_COMPONENT_SUBDIVISION));
    ASSERT_NE(nullptr, subdivision);
    EXPECT_EQ(SYS_SAGE_SUBDIVISION_TYPE_GPU_SM, subdivision->GetSubdivisionType());

    auto cacheL1 = dynamic_cast<Cache *>(subdivision->GetChildByType(SYS_SAGE_COMPONENT_CACHE));
    ASSERT_NE(nullptr, cacheL1);
    EXPECT_EQ(24588, cacheL1->GetCacheSize());
    EXPECT_EQ(32, cacheL1->GetCacheLineSize());

    auto thread = dynamic_cast<Thread *>(cacheL1->GetChildByType(SYS_SAGE_COMPONENT_THREAD));
    ASSERT_NE(nullptr, thread);
}
