#ifndef CAPS_NUMA_BENCHMARK
#define CAPS_NUMA_BENCHMARK

#include <sys-sage/Component.hpp>
#include <sys-sage/DataPath.hpp>


namespace sys_sage {
    int parseCapsNumaBenchmark(Component* rootComponent, const std::string &benchmarkPath, const std::string &delim = ";");

    class CSVReader
    {
        std::string benchmarkPath;
        std::string delimiter;
    public:
        CSVReader(std::string benchmarkPath, std::string delm = ";") : benchmarkPath(benchmarkPath), delimiter(delm) { }
        // Function to fetch data from a CSV File
        int getData(std::vector<std::vector<std::string> >*);
    };

} //namespace sys_sage
#endif
