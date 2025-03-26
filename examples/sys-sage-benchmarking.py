#TODO Fix this bullshit

import sys_sage as syge 
import sys 
import os  
import timeit


#this file benchmarks and prints out performance information about basic operations with sys-sage
if __name__ == "__main__":
    path_prefix = sys.argv[0]
    path_prefix = os.path.dirname(path_prefix) + "/"
    xmlPath = path_prefix + "example_data/skylake_hwloc.xml"
    bwPath = path_prefix + "example_data/skylake_caps_numa_benchmark.csv"
    mt4gPath = path_prefix + "example_data/pascal_gpu_topo.csv"

    t = syge.Topology()
    n = syge.Node(t, 1)

    #time create new component
    time_createNewComponent = min(timeit.repeat(lambda: syge.Node(t, 1), number=1000, repeat=3))
    print("time create new component: ",time_createNewComponent/1000)
    
    #time hwloc parsing
    
    time_hwloc = min(timeit.repeat(lambda: syge.parseHwlocOutput(n, xmlPath), number=100, repeat=3))
    print("time hwloc parsing: ",time_hwloc/100)
    
    #time get a vector with all components (of hwloc parsing)
    
    time_getAllComponents = min(timeit.repeat(lambda: t.GetComponentsInSubtree(), number=100, repeat=3))
    print("time for Get hwloc SubtreeNodeList:", time_getAllComponents/100)
    
    #time caps-numa-benchmark parsing
    
    time_parseCapsNumaBenchmark = min(timeit.repeat(lambda: syge.parseCapsNumaBenchmark(n, bwPath), number=100, repeat=3))
    print("time for parseCapsNumaBenchmark: ", time_parseCapsNumaBenchmark/100)
    
    syge.parseCapsNumaBenchmark(n, bwPath)
    def max_bw(n):
        max_bw = 0
        max_bw_comp = None
        
        dps = n.GetDataPaths(syge.DATAPATH_OUTGOING)
        for dp in dps:
            if dp.bandwith > max_bw:
                max_bw_comp = dp.target
            print(dp.bandwith)
        return max_bw_comp
    
    #time get numa max_bw
    time_numa_max_bw = min(timeit.repeat(lambda: max_bw(n), number = 100, repeat = 3 ))
    print("time for numa max bw: ", time_numa_max_bw/100)
    
    
    gpu = syge.Chip(n,100,"GPU")
    
    #time mt4g parsing
    time_mt4