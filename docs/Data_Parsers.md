# sys-sage Data Parsers Documentation

## Available Parsers
- [hwloc](#hwloc) (CPU topology)
- [mt4g](#mt4g) (GPU topology)

<a id="hwloc"></a>
### hwloc (CPU topology)
//TODO



<a id="mt4g"></a>
### mt4g (GPU topology)
Parser of mt4g ( https://github.com/caps-tum/mt4g ) project. This project captures the memory topology of Nvidia GPUs, specifically all GPUs since the Kepler microarchitecture. It is a set of microbenchmarks, which uncover the hidden structure and attributes of modern GPUs, and present them to the user for further processing.

#### General

With mt4g, one can generate a .csv output file, which contains the GPU topology information and attributes regarding the GPU. This .csv is a sys-sage Data Source, which is parsed by the mt4g Data Parser.

#### Parsing Logic
The mt4g Parser creates a new GPU topology representation, starting at the GPU level (as Chip component of type SYS_SAGE_CHIP_TYPE_GPU).

The topology is created with the following hierarchy:
- ```[1   ]``` **GPU** (component Chip, chip type SYS_SAGE_CHIP_TYPE_GPU)
    - ```[1..n]``` **Global memory**(component Memory ; provided MAIN_MEMORY is Shared_On GPU-level; otherwise error)
        - ```[1..n]``` **L2 cache** (component Cache; provided L2_DATA_CACHE is Shared_On GPU-level)
            - ```[1..n]``` *other caches -- L1 cache, Texture cache, Read-only cache* (provided they are Shared_On GPU-level)
                - ```[1..n]``` **SM** (component Subdivision, subdivisionType SYS_SAGE_SUBDIVISION_TYPE_GPU_SM)
                    - ```[1..n]``` *L2 cache (provided L2_DATA_CACHE is Shared_On SM-level)*
                        - ```[1..n]``` **other caches -- L1 cache, Texture cache, Read-only cache** (provided they are Shared_On SM-level) - Either as one object, if they are physically shared, or as separate objects if not.
                            - ```[1..n]``` **GPU Core** (component HW_Thread) -- child of L1 cache
                        - ```[1..n]``` **L1.5 Constant cache** (component Cache)
                            - ```[1   ]``` **L1 Constant cache** (component Cache)
                        - ```[1   ]``` **Shared memory** (component Memory; provided Shared_On SM-level; otherwise error)

- The **GPU (Chip component)** contains the following information (if found in the CSV, line GPU_INFORMATION, line COMPUTE_RESOURCE_INFORMATION, line ADDITIONAL_INFORMATION ):
    - vendor
    - model 
    - name = "GPU" (if new Chip is being created)
    - ```attrib``` (key; value): "CUDA_compute_capability"; string* to the value
    - ```attrib``` (key; value): "Number_of_streaming_multiprocessors"; int*
    - ```attrib``` (key; value): "Number_of_cores_in_GPU"; int*
    - ```attrib``` (key; value): "Number_of_cores_per_SM"; int*
    - ```attrib``` (key; value): "GPU_Clock_Rate"; double* (clock rate in Hz)

Each GPU has one Global memory child.

- The **Global memory (Memory component)** contains the following information (if found in line ADDITIONAL_INFORMATION, line MAIN_MEMORY)
    - size
    - name = "GPU main memory"
    - ```attrib``` (key; value): "Clock_Frequency", double* (clock rate in Hz, from field Memory_Clock_Frequency)
    - ```attrib``` (key; value): "Bus_Width", int* (in bit, from field Memory_Bus_Width)

The Global memory has usually an L2 cache child/children. Alternatively, SMs can be children of Global memory, if the L2 cache is Shared_On SM_level.

- The **L2 cache (Cache component)**. There may be multiple L2 cache segments, if these are detected in mt4g benchmarks (Caches_Per_GPU). It contains the following information (if found in line L2_DATA_CACHE)
    - cache_type = "L2"
    - id = 0
    - cache_size
    - cache_line_size

The L2 would usually have the SMs as children (Shared_On = GPU-level) but can also be the other 

- **SM -- Streaming Multiprocessor (Subdivision component)**. Subdivision of type SYS_SAGE_SUBDIVISION_TYPE_GPU_SM. One SM gets created for each SM the GPU has (as in COMPUTE_RESOURCE_INFORMATION - Number_of_streaming_multiprocessors). It contains the following information
    - Name = "SM (Streaming Multiprocessor)"
    - id - goes from 0 to n-1
    - subdivision_type = SYS_SAGE_SUBDIVISION_TYPE_GPU_SM

SMs usually have multiple types of caches and Shared memory as children.

//TODO what if caches are Shared_On GPU-level?

- **L1 cache (Cache component)**. There are as many L1 caches created as specified in Caches_Per_SM (line L1_DATA_CACHE). The L1, Texture, ReadOnly , and Constant L1 caches may be shared on one physical chip -- if this is the case, they will also be represented as one Cache component in sys-sage. If L1 cache is shared with others, the whole group takes over the values from L1_DATA_CACHE. If no L1 but a Texture cache is present, the group takes over values from TEXTURE_CACHE line. If neither L1 nor Texture is present but ReadOnly is, the group takes over the information from the READ-ONLY_CACHE line. The sharing is distinguisned by the "cache_type" attribute. The possible options are "L1", "L1+Texture", "L1+ReadOnly", "L1+Constant_L1", "L1+Texture+ReadOnly", "L1+Texture+Constant_L1", "L1+ReadOnly+Constant_L1", "L1+Texture+ReadOnly+Constant_L1", "Texture", "Texture+ReadOnly", "Texture+Constant_L1", "Texture+ReadOnly+Constant_L1", "ReadOnly", "ReadOnly+Constant_L1", "Constant_L1". It contains the following information (if found in line L1_DATA_CACHE)
    - cache_type
    - id = 0
    - Name = "Cache"
    - cache_size
    - cache_line_size

The L1 cache (shared with others or not) has the GPU cores (of the whole SM or a respective portion based on Caches_Per_SM )as children.

- **Texture cache (Cache component)**. There are as many Texture caches created as specified in Caches_Per_SM (line TEXTURE_CACHE). The L1, Texture, ReadOnly , and Constant L1 caches may be shared on one physical chip -- if this is the case, they will also be represented as one Cache component in sys-sage. If L1 cache is shared with others, the whole group takes over the values from L1_DATA_CACHE. If no L1 but a Texture cache is present, the group takes over values from TEXTURE_CACHE line. If neither L1 nor Texture is present but ReadOnly is, the group takes over the information from the READ-ONLY_CACHE line. The sharing is distinguisned by the "cache_type" attribute. The possible options are "L1", "L1+Texture", "L1+ReadOnly", "L1+Constant_L1", "L1+Texture+ReadOnly", "L1+Texture+Constant_L1", "L1+ReadOnly+Constant_L1", "L1+Texture+ReadOnly+Constant_L1", "Texture", "Texture+ReadOnly", "Texture+Constant_L1", "Texture+ReadOnly+Constant_L1", "ReadOnly", "ReadOnly+Constant_L1", "Constant_L1". It contains the following information (if found in line TEXTURE_CACHE)
    - cache_type
    - id = 0
    - Name = "Cache"
    - cache_size
    - cache_line_size

- **Read-Only cache (Cache component)**. There are as many Read-Only caches created as specified in Caches_Per_SM (line READ-ONLY_CACHE). The L1, Texture, ReadOnly , and Constant L1 caches may be shared on one physical chip -- if this is the case, they will also be represented as one Cache component in sys-sage. If L1 cache is shared with others, the whole group takes over the values from L1_DATA_CACHE. If no L1 but a Texture cache is present, the group takes over values from TEXTURE_CACHE line. If neither L1 nor Texture is present but ReadOnly is, the group takes over the information from the READ-ONLY_CACHE line. The sharing is distinguisned by the "cache_type" attribute. The possible options are "L1", "L1+Texture", "L1+ReadOnly", "L1+Constant_L1", "L1+Texture+ReadOnly", "L1+Texture+Constant_L1", "L1+ReadOnly+Constant_L1", "L1+Texture+ReadOnly+Constant_L1", "Texture", "Texture+ReadOnly", "Texture+Constant_L1", "Texture+ReadOnly+Constant_L1", "ReadOnly", "ReadOnly+Constant_L1", "Constant_L1". It contains the following information (if found in line READ-ONLY_CACHE)
    - cache_type
    - id = 0
    - Name = "Cache"
    - cache_size
    - cache_line_size

- **Constant L1.5 cache (Cache component)**. The Constant L1.5 cache is created as a child of the SM it belongs to, and is filled with informaiton parsed on line CONST_L1_5_CACHE.
    - cache_type = "Constant_L1.5"
    - id = 0
    - Name = "Cache"
    - cache_size
    - cache_line_size

Unless a Constant L1 cache is shared with L1 cache, it is a child of C_1.5 cache.

- **Constant L1 cache (Cache component)**. There is as many Constant L1 caches created as specified in Caches_Per_SM (line CONSTANT_L1_CACHE). The L1, Texture, ReadOnly , and Constant L1 caches may be shared on one physical chip -- if this is the case, they will also be represented as one Cache component in sys-sage. If L1 cache is shared with others, the whole group takes over the values from L1_DATA_CACHE. If no L1 but a Texture cache is present, the group takes over values from TEXTURE_CACHE line. If neither L1 nor Texture is present but ReadOnly is, the group takes over the information from the READ-ONLY_CACHE line. The sharing is distinguisned by the "cache_type" attribute. The possible options are "L1", "L1+Texture", "L1+ReadOnly", "L1+Constant_L1", "L1+Texture+ReadOnly", "L1+Texture+Constant_L1", "L1+ReadOnly+Constant_L1", "L1+Texture+ReadOnly+Constant_L1", "Texture", "Texture+ReadOnly", "Texture+Constant_L1", "Texture+ReadOnly+Constant_L1", "ReadOnly", "ReadOnly+Constant_L1", "Constant_L1". It contains the following information (if found in line READ-ONLY_CACHE)
    - cache_type
    - id = 0
    - Name = "Cache"
    - cache_size
    - cache_line_size

If Constant L1 cache is not shared with others, such as the L1 cache, it will be inserted as a child of Constant L1.5 cahce.

- **Shared memory (Memory component)** contains the following information (if found in line SHARED_MEMORY)
    - size
    - name = "Shared memory"

Shared memory is usually a child of an SM, unless L2 cache is shared on SM level (then it is L2 cache)

- **DataPath**
    - **Load Latencies** are measured between the cores and several memories/cahces. They are oriented, DataPath type SYS_SAGE_DATAPATH_TYPE_LOGICAL. It contains the "Load_Latency" value from the particular entry. GPU cycles value is used (bool latency_in_cycles cannot be set up now).
    The following DataPaths are created: 
        - **Global Memory --> each GPU core** (class Memory --> Thread)
        - **Shared memroy --> all GPU cores from the SM** (class Memory --> Thread)
        - **L2 cache --> each GPU core** (class Cache --> Thread)
        - **L1 cache --> all child GPU cores** (class Cache --> Thread)
        - **Texture cache --> all GPU cores from the SM** (class Cache --> Thread) -- if Texture cache is shared with L1, this DP does not get created (//TODO create anyways?)
        - **Read-only cache --> all GPU cores from the SM** (class Cache --> Thread) -- if Read-only cache is shared with L1 or Texture, this DP does not get created (//TODO create anyways?)
        - **Constant L1 cache --> all GPU cores from the SM** (class Cache --> Thread) -- if Constant L1 cache is shared with L1, Texture, or Read-only, this DP does not get created (//TODO create anyways?)
        - **Constant L1.5 cache --> all GPU cores from the SM** (class Cache --> Thread) 

Line "REGISTER_INFORMATION" of the output is not parsed. (//TODO parse as well?)


