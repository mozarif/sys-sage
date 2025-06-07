# XML – Functionality 
## XML - Export

```

int exportToXml(Component *root, string path = "", std::function<int(string, void *, string *)> search_custom_attrib_key_fcn = NULL, std::function<int(string, void *, xmlNodePtr)> search_custom_complex_attrib_key_fcn = NULL); 
```
 


This function starts with the root component and exports the entire topology to an XML file, including the Datapaths, which are appended at the end of the file.

For attribute parsing, the function uses default parsing mechanisms if no custom functions are provided. The default parsing includes the following simple attributes:

| Key                                   | Value-Type |
|---------------------------------------|------------|
| CATcos                                | uint64_t   |
| CATL3mask                             | uint64_t   |
| mig_size                              | long long  |
| Number_of_streaming_multiprocessors   | int        |
| Number_of_cores_in_GPU                | int        |
| Number_of_cores_per_SM                | int        |
| Bus_Width_bit                         | int        |
| Clock_Frequency                       | double     |
| latency                               | float      |
| latency_min                           | float      |
| latency_max                           | float      |

For complex attributes the default function can parse the freq_history and GPU_Clock_Rate. 

The freq_history might look like this in the resulting xml-file: 

 ```
<Attribute name="freq_history"> 

    <freq_history timestamp="1736603045075772798" frequency="3048.269000" unit="MHz"/> 

    <freq_history timestamp="1736603045177185666" frequency="800.000000" unit="MHz"/> 

    <freq_history timestamp="1736603045278773648" frequency="800.030000" unit="MHz"/> 

    <freq_history timestamp="1736603045380334162" frequency="800.001000" unit="MHz"/> 

    <freq_history timestamp="1736603045482207012" frequency="800.033000" unit="MHz"/> 

    <freq_history timestamp="1736603045584019169" frequency="800.009000" unit="MHz"/> 

    <freq_history timestamp="1736603045685676789" frequency="800.005000" unit="MHz"/> 

    <freq_history timestamp="1736603045786759038" frequency="1600.935000" unit="MHz"/> 

    <freq_history timestamp="1736603045889138928" frequency="800.012000" unit="MHz"/> 

    <freq_history timestamp="1736603045990249699" frequency="2601.876000" unit="MHz"/> 

</Attribute> 
```

And for the GPU_Clock_Rate the xml-node will look like this 

```

<Attribute name="GPU_Clock_Rate"> 

    <GPU_Clock_Rate frequency="3048.269000" unit="MHz"/> 

</Attribute> 
```

The Attribute-Nodes are all inserted as child-nodes under the corresponding Component-Node in the xml-file. 

### Custom Functions 

The XML Export function supports two optional custom functions, allowing users to customize the export process for both simple attributes and complex attributes.

Custom Function for Simple Attributes:\
If a custom function for simple attributes is provided, the export function forwards the key-value pair from the ```attrib``` map of the respective component to the custom function. The custom function is expected to return the processed value as a string, which will be written into the export as the provided ret_value_string (the last parameter of the custom function).

Such a function might look like this:

```C++
int search_simple(std::string k, void *value, std::string *ret_value_str) {
  //check the key
  if (!k.compare("benchmark")) {
    //process value to a string and store it
    *ret_value_str = *(std::string *)value;
    return 1;
  }
  return 0;
};
```


Custom Function for Complex Attributes:\
Similarly, for complex attributes, the export function forwards the key-value pair from the ```attrib``` map to the custom function. In addition, the current XML node is passed as a third parameter. Unlike the simple attributes, the custom function is responsible for directly writing the corresponding XML node(s) to the provided node.

One example of this kind of custom funcion is shown below:
```C++
int search_complex(std::string k, void *value, xmlNodePtr n) {
  //check key
  if (!k.compare("complex")) {
    int *val = (int *)value;
    //create new xmlNode with "Attribute" as name
    xmlNodePtr attr_node = xmlNewNode(NULL, (const unsigned char *)"Attribute");
    //Assign the key given as argument
    xmlNewProp(attr_node, (const unsigned char *)"key",
               (const unsigned char *)k.c_str());
    //Process the value to a string or use more xmlnodes if needed
    xmlNewProp(attr_node, (const unsigned char *)"value",
               (const unsigned char *)std::to_string(*val).c_str());
    xmlAddChild(n, attr_node);
    return 1;
  }
  return 0;
};
```

The export tries to parse the attributes in this order:
1. custom_simple
2. default_simple
3. custom_complex
4. default_complex

It will try each function in this order untill one function returns 1.

## XML - Import
 
 ```
 Component* importFromXml(string path, std::function<void*(xmlNodePtr)> search_custom_attrib_key_fcn = NULL, std::function<int(xmlNodePtr, Component*)> search_custom_complex_attrib_key_fcn = NULL);
```

 The Import of xml-files works similiar to the export but in the opposite direction. It reads the xml-file with the same structure as the xml-export-files and returns the topology node. All the Datapaths and attributes are also stored after import.

The Attributes are stored in the attrib map of the components. Per default all the attribute types that can be parsed in the export can also be parsed in import.

### Custom Functions

Simple Attributes:\
For simple attributes, the corresponding attribute node from the XML file is forwarded to the custom function. The custom function is expected to produce a ```void*``` that can then be stored in the ```attrib``` map of the component.

Here is an example of how to define such a function:
```C++
void *imp_search_simple(xmlNodePtr n) {
  //check existence of key-value-pair
  if (xmlHasProp(n, (const xmlChar *)"name") &&
      xmlHasProp(n, (const xmlChar *)"value")) {
    //retrieve key
    const unsigned char *v = xmlGetProp(n, (const unsigned char *)"name");
    std::string key(reinterpret_cast<char const *>(v));
    //retrieve value
    v = xmlGetProp(n, (const unsigned char *)"value");
    std::string value(reinterpret_cast<char const *>(v));
    //return value (key is already known by caller function)
    return new std::string(value);
  } else {
    return NULL;
  }
}
```

Complex Attributes:\
For complex attributes, the attribute node is forwarded alongside the current component. This allows the custom function to access the component’s ```attrib``` map and store the parsed attributes directly.

Defining a custom parsing function might look like this
```C++
int imp_search_complex(xmlNodePtr n, Component *c) {
  //check existence of key-value-pair
  if (xmlHasProp(n, (const xmlChar *)"key") &&
      xmlHasProp(n, (const xmlChar *)"value")) {
    //retrieve key
    const unsigned char *v = xmlGetProp(n, (const unsigned char *)"key");
    std::string key(reinterpret_cast<char const *>(v));
    //retrieve value
    v = xmlGetProp(n, (const unsigned char *)"value");
    int* value = new int(std::stoi(reinterpret_cast<char const *>(v)));
    //store value in component's attrib map
    c->attrib[key] = (void*) value;
  } else {
    return 0;
  }
}
```

While traversing XML nodes, it may be necessary to skip text nodes to avoid processing unintended data. For example:
```
if (xml_node->type == XML_TEXT_NODE)
    continue;
```
The custom function for complex attributes should return 1 on success and 0 on failure.
