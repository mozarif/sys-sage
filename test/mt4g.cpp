#include <boost/ut.hpp>
#include <string_view>

#include "sys-sage.hpp"

using namespace boost::ut;
using namespace sys_sage;
using namespace std::string_view_literals;

static suite<"mt4g"> _ = []
{
    Topology topo;
    Chip gpu{&topo};
    expect(that % (0 == parseMt4gTopo(&gpu, SYS_SAGE_TEST_RESOURCE_DIR "/pascal_gpu_topo.csv")) >> fatal);

    for (const auto &[type, count] : std::vector{
             std::tuple{ComponentType::Memory, 31},
             std::tuple{ComponentType::Subdivision, 30},
             std::tuple{ComponentType::Cache, 121},
             std::tuple{ComponentType::Thread, 3840},
         })
    {
        std::vector<Component *> components;
        topo.GetSubcomponentsByType(&components, type);
        expect(that % _u(count) == components.size());
    }

    expect(that % "Nvidia"sv == gpu.GetVendor());
    expect(that % "Quadro P6000"sv == gpu.GetModel());

    auto memory = dynamic_cast<Memory *>(gpu.GetChildByType(ComponentType::Memory));
    expect(that % (nullptr != memory) >> fatal);
    expect(that % 25637224578 == memory->GetSize());
    expect(that % 3840_u == memory->GetAllDataPaths(DataPathType::Any, DataPathDirection::Outgoing).size());

    auto cacheL2 = dynamic_cast<Cache *>(memory->GetChildByType(ComponentType::Cache));
    expect(that % (nullptr != cacheL2) >> fatal);
    expect(that % 3145728 == cacheL2->GetCacheSize());
    expect(that % 32 == cacheL2->GetCacheLineSize());

    auto subdivision = dynamic_cast<Subdivision *>(cacheL2->GetChildByType(ComponentType::Subdivision));
    expect(that % (nullptr != subdivision) >> fatal);
    expect(that % SubdivisionType::GpuSM == subdivision->GetSubdivisionType());

    auto cacheL1 = dynamic_cast<Cache *>(subdivision->GetChildByType(ComponentType::Cache));
    expect(that % (nullptr != cacheL1) >> fatal);
    expect(that % 24588 == cacheL1->GetCacheSize());
    expect(that % 32 == cacheL1->GetCacheLineSize());

    auto thread = dynamic_cast<Thread *>(cacheL1->GetChildByType(ComponentType::Thread));
    expect(that % (nullptr != thread) >> fatal);
    //topo.Delete(true);
};
