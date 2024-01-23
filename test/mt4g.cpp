#include <boost/ut.hpp>

#include "sys-sage.hpp"

using namespace boost::ut;

static suite<"mt4g"> _ = []
{
    Topology topo;
    Chip gpu{&topo};
    expect(that % (0 == parseMt4gTopo(&gpu, SYS_SAGE_TEST_RESOURCE_DIR "/pascal_gpu_topo.csv")) >> fatal);

    for (const auto &[type, count] : std::vector{
             std::tuple{SYS_SAGE_COMPONENT_MEMORY, 31},
             std::tuple{SYS_SAGE_COMPONENT_SUBDIVISION, 30},
             std::tuple{SYS_SAGE_COMPONENT_CACHE, 121},
             std::tuple{SYS_SAGE_COMPONENT_THREAD, 3840},
         })
    {
        std::vector<Component *> components;
        topo.GetSubcomponentsByType(&components, type);
        expect(that % _u(count) == components.size());
    }

    expect(that % "Nvidia"sv == gpu.GetVendor());
    expect(that % "Quadro P6000"sv == gpu.GetModel());

    auto memory = dynamic_cast<Memory *>(gpu.GetChildByType(SYS_SAGE_COMPONENT_MEMORY));
    expect(that % (nullptr != memory) >> fatal);
    expect(that % 25637224578 == memory->GetSize());
    expect(that % 3840_u == memory->GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING)->size());

    auto cacheL2 = dynamic_cast<Cache *>(memory->GetChildByType(SYS_SAGE_COMPONENT_CACHE));
    expect(that % (nullptr != cacheL2) >> fatal);
    expect(that % 3145728 == cacheL2->GetCacheSize());
    expect(that % 32 == cacheL2->GetCacheLineSize());

    auto subdivision = dynamic_cast<Subdivision *>(cacheL2->GetChildByType(SYS_SAGE_COMPONENT_SUBDIVISION));
    expect(that % (nullptr != subdivision) >> fatal);
    expect(that % SYS_SAGE_SUBDIVISION_TYPE_GPU_SM == subdivision->GetSubdivisionType());

    auto cacheL1 = dynamic_cast<Cache *>(subdivision->GetChildByType(SYS_SAGE_COMPONENT_CACHE));
    expect(that % (nullptr != cacheL1) >> fatal);
    expect(that % 24588 == cacheL1->GetCacheSize());
    expect(that % 32 == cacheL1->GetCacheLineSize());

    auto thread = dynamic_cast<Thread *>(cacheL1->GetChildByType(SYS_SAGE_COMPONENT_THREAD));
    expect(that % (nullptr != thread) >> fatal);
    //topo.Delete(true);
};
