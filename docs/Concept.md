# Architectural Concept and Design

## Workflow

The typical workflow is described in the figure below.

<p align="middle">
    <img alt="workflow under sys-sage" src="images/workflow.drawio.png" width="80%">
</p>

We use the term "user" to refer to any resource manager, daemon, user-side application, or any other entities that store or retrieve data from the library.

1. As part of the pre-run system discovery, the user measures and collects relevant topological information from available data sources
2. At application startup, the static configuration of the target architecture as well as the supplementary data from the previous step are imported into the library to materialize an initial system state.
3. During application runtime, topological information can be queried/updated/extended through the library's unified interface
4. Before the application terminates, the user may capture a snapshot of the system's state

## Conceptual Design

An overview of _sys-sage_'s design is given in the figure below.

<p align="middle">
    <img alt="conceptual design of sys-sage" src="images/integration-architecture.drawio.png" width="50%">
</p>

### Internal Representation

The hardware topology is modeled by an internal representation that aims at representing both the static, hierarchical and physical decomposition of hardware components as well as their dynamic, often non-hierarchical and logical interconnections.
It builds the foundation for the library by combining all data together into one central interface.

The physical hardware components are represented by the [Component](class_component.html) data structure, whose hierarchical layout form a **Component Tree** that naturaly describes the topologies basis.
Every piece of static or dynamic information attaches to or references Components within this tree, therefore being the primary access point to query topological information.
Although the design is inspired by [hwloc](https://www.open-mpi.org/projects/hwloc/)’s CPU-centric approach, _sys-sage_ significantly generalizes the concept to support a much broader and heterogeneous spectrum of hardware and system resources, thus extending far beyond CPU-centric modelling.
Users can interconnect Components arbitrarily, thus providing a high degree of freedom in expressing custom system configurations and layouts.
Furthermore, _sys-sage_ defines several ComponentTypes derived from distinct parts of computing hardware, with each being tailored towards holding the relevant characteristics and behaviors of its domain.
These types form a class inheritance hierarchy.
More information on the specific classes and their members can be found [here](class_component.html).

In contrast, a [Relation](class_relation.html) is a complementary data structure that reflect different relationships and interactions between Components.
The full collection of Relations forms a **Relations Graph**, which captures information orthogonal to the Component Tree.
Relations are typically referenced through the participating Components, and accommodate a broad spectrum of information types including data-transfer characteristics, performance indicators, power consumption, application-specific metrics, or quantum-specific properties.
Moreover, a Component can participate in an arbitrary number of Relations, and multiple Relations can connect the same subset of Components to represent different dependencies or properties.
This open design provides high flexibility for diverse user-defined use-case scenarios.
Similar to Components, each Relation belongs to a specific RelationType, which allows for more specialized functionality targeted at a specific usage scenario.
Additionally, the RelationCategory provides semantic tagging of the specific Relation.
More information can be found [here](class_relation.html).

To illustrate the distinction, Components may represent individual caches along with their size, associativity, or observed hit rates from hardware counters.
Components will also be used to store a CPU core’s register file size and current frequency.
Relations extend this view by capturing relational properties between an arbitrary number of (often two: source-target) Components — for instance, cache-to-core load latencies across multiple cache levels or the fraction of cross-NUMA memory accesses between different NUMA domains.
The figures below showcases this distinction.

<p align="middle">
    <img alt="sys-sage's Component Tree data structure" src="images/component-tree-dis.drawio.png" width="40%">
    <br clear="all">
    <img alt="sys-sage's Relations Graph data structure" src="images/relations-dis.drawio.png" width="40%">
</p>

The one at the top shows the Component Tree consisting of different ComponentTypes marked by different colors, whereas the figure at the bottom demonstrates relations carrying different information as indicated by the different colors.

Refer to this [tutorial](../examples/tutorials/Tutorial_01.md) to get hands-on experience with _sys-sage_'s Component Tree and Relations Graph.

### Data Sources and Data Parsers

As mentioned before, _sys-sage_ collects topological information from external Data Sources to construct a full system representation.
The Data Sources referred to here constitute the data integration path stemming from pre-startup system discovery.
The raw data is stored within a file that can be the output of applications, benchmarks, scripts, user-defined configuration files, and other inputs.
These files are then uploaded to _sys-sage_ during startup, usually providing the base hardware topology and other information that does not change.
Furthermore, Data Sources can broadly be divided into two categories: Default Data Sources and Custom Data Sources.
Regarding the former, _sys-sage_ offers out-of-the-box integration for commonly used information.
The latter includes user-provided measurements, observations, or findings, which lie outside of the core of _sys-sage_ but can be integrated during the data upload process.

To upload a Data Source to _sys-sage_, a dedicated Data Parser is needed.
They interpret and process the given information and convert it to structures compatible with the library’s Internal Representation.
Similar to Data Sources, Data Parsers are divided into Default and Custom Data Parsers.
On one hand, Default Parsers are part of the library and can directly be used to upload the corresponding Default Data Sources.
On the other hand, are extensions implemented by users to parse their Custom Data Sources.
These are typically implemented by a single function that reads the data and merges it into the Internal Representation.

A list of Default Data Sources for which a Default Data Parser exists is given below.

| Default Data Source | used for |
| ------------------- | -------- |
| **hwloc** | CPU static topology |
| **MT4G** | GPU static and dynamic topology information |
| **IQM** | tatic topological information on IQM QPUs |
| **cccbench** | CPU core-to-core communication latency benchmark |
| **caps-numa-benchmark** | load latencies of CPU to NUMA and cross-NUMA memory accesses |
| **MUSA** | static CPU topology in MUSA simulator input format |
| **FIRESTARTER2**, **BabelSTREAM**, **NAS Parallel Benchmarks**, **High Performance Linpack**, **LULESH** | various performance benchmarks (parsed through MPPV parser) |

More details about the specific information these data sources provide can be found in the API documentation of their respective parsers.

Refer to this [tutorial](../examples/tutorials/Tutorial_02.md) to learn how to upload data sources and how to write your own parsers.

### 3rd party extensions

### User-specific Attributes
