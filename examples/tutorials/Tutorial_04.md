# Tutorial 04: Using 3rd Party Extensions and Querying Dynamic Topological Information

This tutorial will illustrate how you can make use of _sys-sage_'s 3rd party integrations to process dynamic system properties.
In this context, we'll consider the following (simplified) use-case scenario:

Assume multiple applications run on a single node.
The compute cores are partitioned such that each application is assigned a fixed number of cores.
Moreover, the node is set under a power cap and therefore only has access to a limited power budget that it needs to distribute among the applications.
Let's say the "application manager" of the node favors to increase the clock frequency of cores assigned to compute-intensive applications while decreasing the clock frequency of memory-bound applications.
For the sake of simplicity, we say a compute-intensive application has a high IPC.
Now, every application needs to monitor and report the respective performance counters and core frequency.
