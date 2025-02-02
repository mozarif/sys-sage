#include "defines.hpp"

// #ifdef PYBIND

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/attr.h>
#include <string>
#include "Component.hpp"
#include "DataPath.hpp"
#include "sys-sage.hpp"



namespace py = pybind11;

auto NULL_CHECK = [](auto func) {
    return [func](auto self, auto... args) {
        py::object obj = py::cast(self);
        if (obj) {
            return (self->*func)(args...);
        } else {
            throw std::runtime_error("Null object");
        }
    };
};

py::dict syncAttributes(std::map<std::string, void*> &attributes, py::dict object_attributes) {
    py::dict dict;
    for (auto const& [key, value] : attributes) {
        if(!key.compare("CATcos") || !key.compare("CATL3mask")){
            uint64_t retval = *((uint64_t*)value); 
            dict[key.c_str()] = py::cast(retval);
        }
        else if(!key.compare("mig_size") )
        {
            dict[key.c_str()] = py::cast(*(long long*)value);
        }
        //value: int
        else if(!key.compare("Number_of_streaming_multiprocessors") || 
        !key.compare("Number_of_cores_in_GPU") || 
        !key.compare("Number_of_cores_per_SM")  || 
        !key.compare("Bus_Width_bit") )
        {
            dict[key.c_str()] = py::cast(*(int*)value);
        }
        //value: double
        else if(!key.compare("Clock_Frequency") )
        {
            dict[key.c_str()] = py::cast(*(double*)value);
        }
        //value: float
        else if(!key.compare("latency") ||
        !key.compare("latency_min") ||
        !key.compare("latency_max") )
        {
            dict[key.c_str()] = py::cast(*(float*)value);
        }   
        //value: string
        else if(!key.compare("CUDA_compute_capability") || 
        !key.compare("mig_uuid") )
        {
            dict[key.c_str()] = py::cast(*(string*)value);
        }
        else if(!key.compare("freq_history") ){
            std::vector<std::tuple<long long,double>>* val = (std::vector<std::tuple<long long,double>>*)value;
            py::dict freq_dict;
             for(auto [ ts,freq ] : *val){
                 freq_dict[py::cast(ts)] = py::cast(freq);
             }
             dict[key.c_str()] = freq_dict;
        }
        else if(!key.compare("GPU_Clock_Rate")){
            auto [ freq, unit ] = *(std::tuple<double, std::string>*)value;
            py::dict freq_dict;
            freq_dict[py::str("freq")] = py::cast(freq);
            freq_dict[py::str("unit")] = py::cast(unit);       
            dict[key.c_str()] = freq_dict;
        }
    }
    for (auto const& [key, value] : object_attributes) {
        //if value is int
        py::type value_type = py::type::of(value);
        if(value_type.is(py::type::of(py::int_()))){
            attributes[py::cast<std::string>(key)] = (void*) new int (py::cast<int>(value));
        }
        // //if value is long
        // else if(py::isinstance<py::long_(value)){
        //     attributes[py::cast<std::string>(key)] = (void*) new long (py::cast<long>(value));
        // }
        // //if value is float
        // else if(py::isinstance<py::float_(value)){
        //     attributes[py::cast<std::string>(key)] = (void*) new float (py::cast<float>(value));
        // }
        // //if value is double
        // else if(py::isinstance<py::double_(value)){
        //     attributes[py::cast<std::string>(key)] = (void*) new double (py::cast<double>(value));
        // }
        // //if value is bool
        // else if(py::isinstance<py::object>(value) && py::cast<bool>(value)){
        //     attributes[py::cast<std::string>(key)] = (void*) new bool (py::cast<bool>(value));
        // }
        // //else cast to string
        // else {
        //     attributes[py::cast<std::string>(key)] = (void*) new std::string (py::cast<std::string>(value));
        // }
    }
    return dict;
}


PYBIND11_MODULE(sys_sage, m) {

    m.attr("SYS_SAGE_COMPONENT_NONE") = SYS_SAGE_COMPONENT_NONE;
    m.attr("SYS_SAGE_COMPONENT_THREAD") = SYS_SAGE_COMPONENT_THREAD;
    m.attr("SYS_SAGE_COMPONENT_CORE") = SYS_SAGE_COMPONENT_CORE;
    m.attr("SYS_SAGE_COMPONENT_CACHE") = SYS_SAGE_COMPONENT_CACHE;
    m.attr("SYS_SAGE_COMPONENT_SUBDIVISION") = SYS_SAGE_COMPONENT_SUBDIVISION;
    m.attr("SYS_SAGE_COMPONENT_NUMA") = SYS_SAGE_COMPONENT_NUMA;
    m.attr("SYS_SAGE_COMPONENT_CHIP") = SYS_SAGE_COMPONENT_CHIP;
    m.attr("SYS_SAGE_COMPONENT_MEMORY") = SYS_SAGE_COMPONENT_MEMORY;
    m.attr("SYS_SAGE_COMPONENT_STORAGE") = SYS_SAGE_COMPONENT_STORAGE;
    m.attr("SYS_SAGE_COMPONENT_NODE") = SYS_SAGE_COMPONENT_NODE;
    m.attr("SYS_SAGE_COMPONENT_TOPOLOGY") = SYS_SAGE_COMPONENT_TOPOLOGY;

    m.attr("SYS_SAGE_SUBDIVISION_TYPE_NONE") = SYS_SAGE_SUBDIVISION_TYPE_NONE;
    m.attr("SYS_SAGE_SUBDIVISION_TYPE_GPU_SM") = SYS_SAGE_SUBDIVISION_TYPE_GPU_SM;

    m.attr("SYS_SAGE_CHIP_TYPE_NONE") = SYS_SAGE_CHIP_TYPE_NONE;
    m.attr("SYS_SAGE_CHIP_TYPE_CPU") = SYS_SAGE_CHIP_TYPE_CPU;
    m.attr("SYS_SAGE_CHIP_TYPE_CPU_SOCKET") = SYS_SAGE_CHIP_TYPE_CPU_SOCKET;
    m.attr("SYS_SAGE_CHIP_TYPE_GPU") = SYS_SAGE_CHIP_TYPE_GPU;

    m.attr("SYS_SAGE_DATAPATH_NONE") = SYS_SAGE_DATAPATH_NONE;
    m.attr("SYS_SAGE_DATAPATH_OUTGOING") = SYS_SAGE_DATAPATH_OUTGOING;
    m.attr("SYS_SAGE_DATAPATH_INCOMING") = SYS_SAGE_DATAPATH_INCOMING;
    m.attr("SYS_SAGE_DATAPATH_BIDIRECTIONAL") = SYS_SAGE_DATAPATH_BIDIRECTIONAL;
    m.attr("SYS_SAGE_DATAPATH_ORIENTED") = SYS_SAGE_DATAPATH_ORIENTED;
    m.attr("SYS_SAGE_DATAPATH_TYPE_NONE") = SYS_SAGE_DATAPATH_TYPE_NONE;
    m.attr("SYS_SAGE_DATAPATH_TYPE_LOGICAL") = SYS_SAGE_DATAPATH_TYPE_LOGICAL;
    m.attr("SYS_SAGE_DATAPATH_TYPE_PHYSICAL") = SYS_SAGE_DATAPATH_TYPE_PHYSICAL;
    m.attr("SYS_SAGE_DATAPATH_TYPE_L3CAT") = SYS_SAGE_DATAPATH_TYPE_L3CAT;
    m.attr("SYS_SAGE_DATAPATH_TYPE_MIG") = SYS_SAGE_DATAPATH_TYPE_MIG;
    m.attr("SYS_SAGE_DATAPATH_TYPE_DATATRANSFER") = SYS_SAGE_DATAPATH_TYPE_DATATRANSFER;
    m.attr("SYS_SAGE_DATAPATH_TYPE_C2C") = SYS_SAGE_DATAPATH_TYPE_C2C;

    //bind component class
    py::class_<Component>(m, "Component", py::dynamic_attr(),"Generic Component")
       
        .def(py::init<int, string, int>(), py::arg("id") = 0, py::arg("name") = "unknown", py::arg("componentType") = 1)
        .def(py::init<Component*, int, string, int>(), py::arg("parent"), py::arg("id") = 0, py::arg("name") = "unknown", py::arg("componentType") = 1)
        .def("syncAttrib",[](Component& self) {
            //TODO: Better use vector instead of dict 
            py::object pyself = py::cast(&self);
            py::dict dict = syncAttributes((self.attrib), pyself.attr("__dict__"));
            for(auto const& [key, value] : dict){
                py::setattr(pyself, key, value);
            }
        })
        .def("testAttrib",[](Component& self) {
            self.attrib["test_int"] = new int(1);
            int* test = (int*) self.attrib["test_int"];
            printf("%d\n", *test);
        })
        // TODO: implement get and set attributes according to ipad sketches 
        //.def("__getattr__", [](Component& self, const std::string& name) {})
        // .def("__setattr__", [](Component& self, const std::string& name, py::object value) {})
        // .def("get_attrib", [](Component& self) {} 
        // .def("set_attrib", [](Component& self, const py::dict& dict) {})
        .def("InsertChild", &Component::InsertChild, py::arg("child"))
        //rename functions
        .def("InsertBetweenParentAndChild", &Component::InsertBetweenParentAndChild, "Insert a component between parent and child")
        .def("InsertBetweenParentAndChildren", &Component::InsertBetweenParentAndChildren, "Insert a component between parent and children")
        .def("RemoveChild", &Component::RemoveChild, "Remove a child component")
        //.def("SetParent", &Component::SetParent, "Set the parent of the component")
        .def_property("parent", &Component::GetParent, &Component::SetParent, "The parent of the component")
        .def("PrintSubtree", (void (Component::*)()) &Component::PrintSubtree, "Print the subtree of the component")
        .def("PrintSubtree", (void (Component::*)(int)) &Component::PrintSubtree, "Print the subtree of the component with a maximum depth of <level>")
        .def("PrintAllDataPathsInSubtree", &Component::PrintAllDataPathsInSubtree, "Print the datapath subtree of the component")
        //.def("GetName", &Component::GetName, "The name of the component")
        //.def("SetName", &Component::SetName, "Set the name of the component")
        .def_property("name", &Component::GetName, &Component::SetName, "The name of the component")
        .def("GetId", &Component::GetId, "The id of the component")
        .def_property_readonly("id", &Component::GetId, "The id of the component")
        //.def("GetComponentType", &Component::GetComponentType, "The type of the component")
        .def_property_readonly("type", &Component::GetComponentType, "The type of the component")
        .def("GetComponentTypeStr", &Component::GetComponentTypeStr, "The type of the component as string")
        .def("GetChildren", &Component::GetChildren, "The children of the component")
        //.def("GetParent", &Component::GetParent, "The parent of the component")
        .def("GetChild", &Component::GetChild, "Like get_child_by_id()")
        .def("GetChildById", &Component::GetChildById, "Get the first child component by id")
        .def("GetChildByType", &Component::GetChildByType, "Get the first child component by type")
        .def("GetAllChildrenByType", (void (Component::*)(vector<Component*> *, int)) &Component::GetAllChildrenByType, "Get all child components by type")
        .def("GetAllChildrenByType", (vector<Component*> (Component::*)(int))(&Component::GetAllChildrenByType), "Get all child components by type")
        .def("GetSubcomponentById", &Component::GetSubcomponentById, "Get the first sub component by id")
        .def("GetAllSubcomponentsByType", (void (Component::*)(vector<Component*> *, int))(&Component::GetAllSubcomponentsByType), "Get the first sub component by type")
        .def("GetAllSubcomponentsByType", (vector<Component*> (Component::*)(int))(&Component::GetAllSubcomponentsByType), "Get all sub components by type")
        .def("CountAllSubcomponents", &Component::CountAllSubcomponents, "Count all sub components")
        .def("CountAllSubcomponentsByType", &Component::CountAllSubcomponentsByType, "Count sub components by type")
        .def("CountChildrenByType", &Component::CountAllChildrenByType, "Count children by type")
        .def("GetAncestorByType", &Component::GetAncestorByType, "Get the first ancestor component by type")
        .def("GetSubtreeDepth", &Component::GetSubtreeDepth, "Get the depth of the subtree")
        .def("GetNthAncestor", &Component::GetNthAncestor, "Get the nth ancestors of the component")
        .def("GetNthDescendents", (void (Component::*)(vector<Component*> *, int))&Component::GetNthDescendents,"Get all the nth descendents of the component")
        .def("GetNthDescendents", (vector<Component*> (Component::*)(int))&Component::GetNthDescendents,"Get all the nth descendents of the component")
        .def("GetSubcomponentsByType", (void (Component::*)(vector<Component*> *, int))&Component::GetSubcomponentsByType,"Get all the sub components of the component by type")
        .def("GetSubcomponentsByType", (vector<Component*> (Component::*)(int))&Component::GetSubcomponentsByType,"Get all the sub components of the component by type")
        .def("GetComponentsInSubtree", (void (Component::*)(vector<Component*> *))&Component::GetComponentsInSubtree,"Get all the components in the subtree of the component")
        .def("GetComponentsInSubtree", (vector<Component*> (Component::*)())&Component::GetComponentsInSubtree,"Get all the components in the subtree of the component")
        .def("GetDataPaths", &Component::GetDataPaths,"Get all the data paths associated with the component")
        .def("AddDataPath", &Component::AddDataPath,"Add a data path to the component")
        .def("GetDataPathByType", &Component::GetDataPathByType,"Get the first data path associated with the component by type")
        .def("GetAllDataPathsByType", (void (Component::*)(vector<DataPath*> *, int, int)) &Component::GetAllDataPathsByType,"Get all the data paths associated with the component by type")
        .def("GetAllDataPathsByType", (vector<DataPath*> (Component::*)(int, int)) &Component::GetAllDataPathsByType,"Get all the data paths associated with the component by type")
        .def("CheckComponentTreeConsistency", &Component::CheckComponentTreeConsistency,"Check if the component tree is consistent")
        .def("GetTopologySize", (int (Component::*)(unsigned*, unsigned*)) (&Component::GetTopologySize),"Get the size of the topology")
        .def("GetDepth", &Component::GetDepth,"Get the depth of the component")
        .def("DeleteDataPath", &Component::DeleteDataPath,"Delete a data path from the component")
        .def("DeleteAllDataPaths", &Component::DeleteAllDataPaths,"Delete all the data paths from the component")
        .def("DeleteSubtree", &Component::DeleteSubtree,"Delete the subtree of the component")
        .def("Nullcheck", [](Component& self){ return (&self == nullptr);})
        //.def("Delete", &Component::Delete,"Delete the component")
        .def("Delete", [](Component& self, bool deleteSubtree) { 
            py::object obj = py::cast(&self);
            obj.release();
            if (deleteSubtree){
                obj.attr("DeleteSubtree")();
            }
            self.Delete(false); }, "Delete the component")
        .def_readwrite("attrib", &Component::attrib);
    py::class_<Topology, Component>(m, "Topology")
        .def(py::init<>());
    py::class_<Node, Component>(m, "Node")
        .def(py::init<int, string>(), py::arg("id") = 0, py::arg("name")= "Node")
        .def(py::init<Component*, int, string>(), py::arg("parent"), py::arg("id") = 0, py::arg("name") = "Node")
        .def("RefreshCpuCoreFrequency", &Node::RefreshCpuCoreFrequency, py::arg("keep_history")=false,"Refresh the cpu core frequency");
    py::class_<Memory, Component>(m, "Memory")
        .def(py::init<long long, bool>(), py::arg("size") = -1, py::arg("isVolatile") = false)
        .def(py::init<Component*,int, string, long long, bool>(), py::arg("parent"), py::arg("id") = 0, py::arg("name") = "Memory", py::arg("size")=-1, py::arg("isVolatile")=false)
        .def_property("size", &Memory::GetSize, &Memory::SetSize, "The size of the memory")
        .def_property("isVolatile", &Memory::GetIsVolatile, &Memory::SetIsVolatile, "Whether the memory is volatile or not");
    py::class_<Storage, Component>(m, "Storage")
        .def(py::init<long long>(), py::arg("size")=-1)
        .def(py::init<Component*,long long>(), py::arg("parent"), py::arg("size")= -1)
        .def_property("size", &Storage::GetSize, &Storage::SetSize, "The size of the storage");
    py::class_<Chip, Component>(m, "Chip")
        .def(py::init<int,string,int,string,string>(), py::arg("id") = 0, py::arg("name") = "Chip", py::arg("chipType")= 1, py::arg("vendor") = "", py::arg("model") = "", py::return_value_policy::reference)
        .def(py::init<Component*,int,string,int,string,string>(), py::arg("parent"),py::arg("id") = 0, py::arg("name") = "Chip", py::arg("chipType") = 1, py::arg("vendor") = "", py::arg("model") = "")
        .def_property("vendor", &Chip::GetVendor, &Chip::SetVendor, "The vendor of the chip")
        .def_property("model", &Chip::GetModel, &Chip::SetModel, "The model of the chip")
        .def_property("chipType", &Chip::GetChipType, &Chip::SetChipType, "The type of the chip");
    py::class_<Cache, Component>(m, "Cache")
        .def(py::init<int,int,long long, int, int>(), py::arg("id") = 0, py::arg("level") = 0, py::arg("size") = -1, py::arg("associativity") = -1, py::arg("lineSize") = -1)
        .def(py::init<Component*,int,int,long long, int, int>(), py::arg("parent"), py::arg("id") = 0, py::arg("level") = 0, py::arg("size") = -1, py::arg("associativity") = -1, py::arg("lineSize") = -1)
        .def(py::init<Component*, int, string, long long, int, int>(), py::arg("parent"), py::arg("id"), py::arg("cache_type"), py::arg("size") = -1, py::arg("associativity") = -1, py::arg("lineSize") = -1)
        .def_property("cacheLevel", &Cache::GetCacheLevel, &Cache::SetCacheLevel, "The level of the cache")
        .def_property("cacheName", &Cache::GetCacheName, &Cache::SetCacheName, "The name of the cache")
        .def_property("cacheSize", &Cache::GetCacheSize, &Cache::SetCacheSize, "The size of the cache")
        .def_property("cacheAssociativity", &Cache::GetCacheAssociativityWays, &Cache::SetCacheAssociativityWays, "The associativity of the cache")
        .def_property("cacheLineSize", &Cache::GetCacheLineSize, &Cache::SetCacheLineSize, "The line size of the cache");
    py::class_<Subdivision, Component>(m, "Subdivision")
        .def(py::init<int,string, int>(), py::arg("id") = 0, py::arg("name") = "Subdivision", py::arg("componentType") = 16)
        .def(py::init<Component*,int,string, int>(), py::arg("parent"), py::arg("id") = 0, py::arg("name") = "Subdivision", py::arg("componentType") = 16)
        .def_property("subdivisionType", &Subdivision::GetSubdivisionType, &Subdivision::SetSubdivisionType, "The type of the subdivision");
    py::class_<Numa, Subdivision>(m, "Numa")
        .def(py::init<int, long long>(), py::arg("id") = 0, py::arg("size") = -1)
        .def(py::init<Component*, int, long long>(), py::arg("parent"), py::arg("id") = 0, py::arg("size") = -1)
        .def_property("size", &Numa::GetSize, &Numa::SetSize, "Size of the NUMA region");
    py::class_<Core, Component>(m, "Core")
        .def(py::init<int,string>(),py::arg("id") = 0, py::arg("name") = "Core")
        .def(py::init<Component*,int,string>(),py::arg("parent"),py::arg("id") = 0 ,py::arg("name") = "Core")
        .def("RefreshFreq", &Core::RefreshFreq,py::arg("keep_history") = false,"Refresh the frequency of the component")
        .def_property("freq", &Core::GetFreq, &Core::SetFreq, "Frequency of this core");
        
    py::class_<Thread, Component>(m,"Thread")
        .def(py::init<int,string>(),py::arg("id") = 0,py::arg("name") = "Thread")
        .def(py::init<Component*,int,string>(),py::arg("parent"),py::arg("id") = 0,py::arg("name") = "Thread")
        .def("RefreshFreq", &Thread::RefreshFreq,py::arg("keep_history") = false,"Refresh the frequency of the component")
        .def_property_readonly("freq", &Thread::GetFreq, "Get Frequency of this thread");

    py::class_<DataPath>(m,"DataPath")
        .def(py::init<Component*, Component*, int, int>(), py::arg("source"), py::arg("target"), py::arg("oriented"), py::arg("type") = 32)
        .def(py::init<Component*, Component*, int, double, double>(), py::arg("source"), py::arg("target"), py::arg("oriented"), py::arg("bw"), py::arg("latency"))
        .def(py::init<Component*, Component*, int, int, double, double>(), py::arg("source"), py::arg("target"), py::arg("oriented"), py::arg("type"), py::arg("bw"), py::arg("latency"))
        .def_property("bandwidth", &DataPath::GetBandwidth, &DataPath::SetBandwidth, "The bandwidth of the data path")
        .def_property("latency", &DataPath::GetLatency, &DataPath::SetLatency, "The latency of the data path")
        .def_property_readonly("type", &DataPath::GetDataPathType, "The type of the data path")
        .def_property_readonly("oriented", &DataPath::GetOrientation, "The orientation of the data path")
        .def_property("source", &DataPath::GetSource, &DataPath::UpdateSource, "The source of the data path")
        .def_property("target", &DataPath::GetTarget, &DataPath::UpdateTarget, "The target of the data path");

    m.def("parseMt4gTopo", (int (*) (Node*,string,int, string)) &parseMt4gTopo, "parseMt4gTopo", py::arg("parent"), py::arg("dataSourcePath"), py::arg("gpuID"), py::arg("delim") = ";");
    m.def("parseMt4gTopo", (int (*) (Component*,string,int, string)) &parseMt4gTopo, "parseMt4gTopo", py::arg("parent"), py::arg("dataSourcePath"), py::arg("gpuID"), py::arg("delim") = ";");
    m.def("parseMt4gTopo", (int (*) (Chip*,string, string)) &parseMt4gTopo, "parseMt4gTopo", py::arg("parent"), py::arg("dataSourcePath"),  py::arg("delim") = ";");

    m.def("parseHwlocOutput", &parseHwlocOutput, "parseHwlocOutput", py::arg("root"), py::arg("xmlPath"));

    m.def("parseCccbenchOutput", &parseCccbenchOutput, "parseCccbenchOutput", py::arg("root"), py::arg("cccPath"));

    m.def("parseCapsNumaBenchmark", &parseCapsNumaBenchmark,  py::arg("root"), py::arg("benchmarkPath"), py::arg("delim") = ";");
}


// #endif