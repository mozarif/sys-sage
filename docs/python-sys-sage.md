# sys-sage Python version documentation

The sys-sage module is the Python version of the sys-sage library, which was already implemented for C++. This documentation focuses on the Python version of the sys-sage library, here called the sys-sage module. It does not contain a list of all functions, choosing instead to focus more on the key aspects and avoiding redundancy.

## Installation

To install the ```sys_sage``` module from source, ensure that Python and Pybind11 are already installed on your system. You can then use CMake for the installation by setting the flag ```-DPYBIND=ON```.

## Module Import

After installation the module can now be imported:
```python
import sys_sage
```

## Object initialistaion

To create Component and Datapath objects, you can utilize the provided initialization functions. For instance, a Component object can be instantiated as follows:

```Python
c = sys_sage.Component(parent, 0, "Generic Component")
```
The sys-sage library exposes all known constructors for ```Component``` and its subclasses as Python initialization functions, complete with their default values. In the example above, if the component type is not explicitly provided, it will automatically be assigned ```None``` during initialization.

Throughout this documentation, c will consistently refer to a Component object, and d will refer to a Datapath object in all given examples.

## Methods

Calling class methods in Python is very similar to the C++ version:

```Python
c.InsertChild(child)
```
However, some methods require a workaround. For instance, directly passing a list as an argument to be populated will not work:

```Python
c.GetAllChildrenByType(children_list, type) # This will not work as expected
```
Instead, for methods that take a list of Component or Datapath objects as arguments (which are currently not working as direct in-place modifications), an alternative approach is provided. You should append the results to an existing list, as shown below:

```Python
children_list += c.GetAllChildrenByType(type)
```
In general, for all methods that expect a list of Component or Datapath objects as arguments to be modified in-place, you should use the alternative method of appending the returned list to your existing list.

Finally, certain attributes, such as id and name, are not accessible via standard getters and setters. The specific methods for reading and writing these attributes are detailed in the "Attributes" section of this documentation.

## Module functions

All of the parsers, that provided by sys-sage library are also included in the Python module:

```python
sys_sage.parseCapsNumaBenchmark(root,"benchmark.csv",";")
```

## Attributes

### Makros / Module Attributes

In the C++ version of the sys-sage library, macros like SYS_SAGE_COMPONENT_NONE are used to represent specific integer values. The Python module provides an analogous way to access these values for use with functions:

```Python
c.CountAllSubcomponentsByType(sys_sage.COMPONENT_NONE)
```

### Properties
As previously mentioned, some attributes aren't accessed using traditional getters or setters. Instead, ```sys_sage``` offers a more Pythonic approach to access them:

```Python
name = c.name
c.name = "name"
```

### Dynamic Attributes

Similar to the core sys-sage library, users can define their own custom attributes.

For the Component class, dynamic attributes are accessed using the [] operator:

```Python
c["dynamic"] = "attribute"
c["dynamic"] # Returns "attribute"
c[0]         # Returns "attribute" (assuming 0 is the index assigned to "dynamic")
```
For Datapath objects, dynamic attributes are treated like basic attributes, allowing direct access:

```Python
d.dynamic = "attribute"
d.dynamic # Returns "attribute"
```

## XML I/O

XML import and export is enabled, however there are some difference to the original library functions.

### XML Export
When exporting data in sys-sage, users can define custom functions to parse component attributes. Two distinct functions can be specified: one for simple attributes and another for complex attributes.

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

Similarly, for complex attributes, the XML node as a string is forwarded. However, unlike the simple attribute function, the custom function for complex attributes also gains access to the respective Component object (c). This allows for more profound modifications to the component's attributes. This function is expected to return only a success status (e.g., 1 for success, 0 for failure).

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
