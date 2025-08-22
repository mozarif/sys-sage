# sys-sage Python API documentation

The _sys-sage_ library provides bindings for the Python programming language through the _sys-sage_ module. This documenation offers a brief introduction.

## Installation

First make sure that `python` (>= 3.10) and `pybind11` are already installed on your system. You can then pass the `-DPY_SYS_SAGE=ON` flag to CMake when compiling the
library to also build the module. The following command can be executed at the root of the repo

```
cmake -B build -DCMAKE_BUILD_TYPE=Release -DPY_SYS_SAGE=ON .
```

## Module Import

After installation the module can now be imported:

```Python
import sys_sage
```

## Object initialistaion

To create Component and Relation objects, you can utilize the provided initialization functions. For instance, a Component object can be instantiated as follows:

```python
c = sys_sage.Component(parent, 0, "Generic Component")
```
The _sys-sage_ library exposes all known constructors for `Component` and its subclasses as Python initialization functions, complete with their default values. In the example above, if the component type is not explicitly provided, it will automatically be assigned ```None``` during initialization.

Throughout this documentation, `c` will consistently refer to a Component object.

## Methods

Calling class methods in Python is very similar to the C++ version:

```Python
c.InsertChild(child)
```
However, certain attributes, such as id and name, are not accessible via standard getters and setters. The specific methods for reading and writing these attributes are detailed in the "Attributes" section of this documentation.

## Module functions

All the default parsers of the _sys-sage_ library are also included in the Python module:

```python
sys_sage.parseCapsNumaBenchmark(root,"benchmark.csv",";")
```

## Attributes

### Module Attributes

The Python module provides a one-to-one correspondance to the known _sys-sage_ constants.

|                C++                 |                Python              |
| ---------------------------------- | ---------------------------------- |
| ComponentType::None                | COMPONENT_NONE                     |
| ComponentType::Thread              | COMPONENT_THREAD                   |
| ComponentType::Core                | COMPONENT_CORE                     |
| ComponentType::Cache               | COMPONENT_CACHE                    |
| ComponentType::Subdivision         | COMPONENT_SUBDIVISION              |
| ComponentType::Numa                | COMPONENT_NUMA                     |
| ComponentType::Chip                | COMPONENT_CHIP                     |
| ComponentType::Memory              | COMPONENT_MEMORY                   |
| ComponentType::Storage             | COMPONENT_STORAGE                  |
| ComponentType::Node                | COMPONENT_NODE                     |
| ComponentType::QuantumBackend      | COMPONENT_QUANTUMBACKEND           |
| ComponentType::AtomSite            | COMPONENT_ATOMSITE                 |
| ComponentType::Qubit               | COMPONENT_QUBIT                    |
| ComponentType::Topology            | COMPONENT_TOPOLOGY                 |
| SubdivisionType::None              | SUBDIVISION_TYPE_NONE              |
| SubdivisionType::GpuSM             | SUBDIVISION_TYPE_GPU_SM            |
| ChipType::None                     | CHIP_TYPE_NONE                     |
| ChipType::Cpu                      | CHIP_TYPE_CPU                      |
| ChipType::CpuSocket                | CHIP_TYPE_CPU_SOCKET               |
| ChipType::Gpu                      | CHIP_TYPE_GPU                      |
| RelationType::Any                  | RELATION_TYPE_ANY                  |
| RelationType::Relation             | RELATION_TYPE_RELATION             |
| RelationType::DataPath             | RELATION_TYPE_DATAPATH             |
| RelationType::QuantumGate          | RELATION_TYPE_QUANTUMGATE          |
| RelationType::CouplingMap          | RELATION_TYPE_COUPLINGMAP          |
| DataPathType::Any                  | DATAPATH_TYPE_ANY                  |
| DataPathType::None                 | DATAPATH_TYPE_NONE                 |
| DataPathType::Logical              | DATAPATH_TYPE_LOGICAL              |
| DataPathType::Physical             | DATAPATH_TYPE_PHYSICAL             |
| DataPathType::Datatransfer         | DATAPATH_TYPE_DATATRANSFER         |
| DataPathType::L3CAT                | DATAPATH_TYPE_L3CAT                |
| DataPathType::MIG                  | DATAPATH_TYPE_MIG                  |
| DataPathType::C2C                  | DATAPATH_TYPE_C2C                  |
| DataPathDirection::Any             | DATAPATH_DIRECTION_ANY             |
| DataPathDirection::Outgoing        | DATAPATH_DIRECTION_OUTGOING        |
| DataPathDirection::Incoming        | DATAPATH_DIRECTION_INCOMING        |
| DataPathOrientation::Oriented      | DATAPATH_ORIENTATION_ORIENTED      |
| DataPathOrientation::Bidirectional | DATAPATH_ORIENTATION_BIDIRECTIONAL |
| QuantumGateType::Unknown           | QUANTUMGATE_TYPE_UNKNOWN           |
| QuantumGateType::Id                | QUANTUMGATE_TYPE_ID                |
| QuantumGateType::X                 | QUANTUMGATE_TYPE_X                 |
| QuantumGateType::Rz                | QUANTUMGATE_TYPE_RZ                |
| QuantumGateType::Cnot              | QUANTUMGATE_TYPE_CNOT              |
| QuantumGateType::Sx                | QUANTUMGATE_TYPE_SX                |
| QuantumGateType::Toffoli           | QUANTUMGATE_TYPE_TOFFOLI           |

An example would be:

```Python
c.CountAllSubcomponentsByType(sys_sage.COMPONENT_NONE)
```

### Properties
As previously mentioned, some attributes aren't accessed using traditional getters or setters. Instead, `sys-sage` offers a more pythonic approach to access them:

```Python
name = c.name
c.name = "name"
```

### Dynamic Attributes

Similar to the core _sys-sage_ library, users can define their own custom attributes.
The usage resembles that of dictionaries.

```Python
c["foo"] = "test"
c["foo"]            # returns "test"
c["bar"] = 1        # values can be arbitrary
c[0]                # returns 1, since key-value pairs are ordered in lexicographical order of the keys
```
The same holds for relations.

## XML I/O

XML import and export is enabled, however there are some difference to the original library functions.

### XML Export
When exporting data in _sys-sage_, users can define custom functions to parse component attributes. Two distinct functions can be specified: one for simple attributes and another for complex attributes.

For simple attributes, the custom function must return either a string or None. The processing of the key-value pair is entirely at the user's discretion. Here's an example:

```Python
def search_custom_simple(key: str, value) -> str:
    if "example" in key:
        return str(value)
    return None
```
For complex attributes, the function is expected to return a string representation of an XML node. The encapsulating root element (e.g., <root></root>) should be omitted. For instance:

```Python
def search_custom_complex(key: str, value) -> str:
    # This function returns an XML fragment representing the complex attribute
    return f"<Attribute key=\"{key}\" value=\"{value}\"/>"
```
After defining your custom user functions, the export process can be initiated by calling the sys_sage.export function, passing the root component, the desired export path, and your custom functions:

```Python
sys_sage.export(root, path, search_custom_simple, search_custom_complex)
```

### XML Import
The XML import functionality mirrors the export function, but with key differences that are best illustrated through examples.

For simple attributes, all custom attributes from the XML file are passed to the user-provided custom parsing function as XML nodes (strings). Therefore, the custom function must employ string operations to extract the desired value:

```Python
def import_search_simple(x):
    # x will be an XML string like '<Attribute key="benchmark" value="some_value"/>'
    if "benchmark" in x:
        y = x.split("value=\"")[-1]
        return y.rstrip("\"/>")
    return None
```

Similarly, for complex attributes, the XML node as a string is forwarded. However, unlike the simple attribute function, the custom function for complex attributes also gains access to the respective Component object (`c`). This allows for more profound modifications to the component's attributes. This function is expected to return only a success status (e.g., 1 for success, 0 for failure).

```Python
def import_search_complex(x, c):
    # x will be an XML string like '<Attribute key="complex" value="123"/>'
    # c is the Component object to which this attribute belongs
    if "complex" in x:
        y = x.split("value=\"")[-1]
        val = y.rstrip("\"/>")
        c["complex"] = int(val)  # Modify the component's attribute directly
        return 1  # Indicate success
    return 0  # Indicate failure
```
