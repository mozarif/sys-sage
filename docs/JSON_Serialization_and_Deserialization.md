# JSON Serialization & Deserialization

_sys-sage_ provides the means to dump the entire component tree and its associated relations to JSON.
In addition, the component tree can be loaded back from JSON.
This way, a snapshot of the dynamic state of the hardware topology can be saved, inspected and reconstructed.
The relevant functions in the library's API are:

```cpp
sys_sage::DumpJson
sys_sage::LoadJson
```

An example is shown below

```cpp
#include <sys-sage.hpp>
#include <iostream>
#include <string>

namespace ss = sys_sage;

int main()
{
    ss::Component *l2cache0 = new ss::Cache(0, 2);
    ss::Component *l1cache0 = new ss::Cache(l2cache0, 0, 1);
    ss::Component *l1cache1 = new ss::Cache(l2cache0, 1, 1);
    ss::Component *core0 = new ss::Core(l1cache0, 0);
    ss::Component *core1 = new ss::Core(l1cache1, 1);

    l2cache0->SetAttribute<std::string>("write policy", "write-back/write-allocate");

    ss::Relation *datapathCore0L2Cache0 = new ss::DataPath(core0, l2cache0, ss::DataPathOrientation::Bidirectional, ss::DataPathCategory::Datatransfer, 199e9, 2.5e-9);
    ss::Relation *datapathCore1L2Cache0 = new ss::DataPath(core1, l2cache0, ss::DataPathOrientation::Bidirectional, ss::DataPathCategory::Datatransfer, 200e9, 2.4e-9);
    ss::Relation *datapathCore0L1Cache0 = new ss::DataPath(core0, l1cache0, ss::DataPathOrientation::Bidirectional, ss::DataPathCategory::Datatransfer, 401e9, 0.4e-9);
    ss::Relation *datapathCore1L1Cache1 = new ss::DataPath(core1, l1cache1, ss::DataPathOrientation::Bidirectional, ss::DataPathCategory::Datatransfer, 399e9, 0.6e-9);

    nlohmann::json obj;
    ss::DumpJson(l2cache0, obj);
    std::cout << obj.dump(4) << '\n';

    ss::Component::DeleteSubtree(l2cache0);

    return 0;
}
```

with corresponding output

```
{
    "componentTree": {
        "address": 94005482936752,
        "attributes": {
            "write policy": {
                "_sys_sage_type": "std::string",
                "_sys_sage_value": "write-back/write-allocate"
            }
        },
        "cacheType": "2",
        "children": [
            {
                "address": 94005482936960,
                "cacheType": "1",
                "children": [
                    {
                        "address": 94005482937440,
                        "id": 0,
                        "type": "Core"
                    }
                ],
                "id": 0,
                "type": "Cache"
            },
            {
                "address": 94005482937200,
                "cacheType": "1",
                "children": [
                    {
                        "address": 94005482937600,
                        "id": 1,
                        "type": "Core"
                    }
                ],
                "id": 1,
                "type": "Cache"
            }
        ],
        "id": 0,
        "type": "Cache"
    },
    "relationGraph": [
        {
            "bandwidth": 200000000000.0,
            "category": 0,
            "components": [
                94005482937600,
                94005482936752
            ],
            "dataPathCategory": 3,
            "id": 0,
            "latency": 2.4e-09,
            "ordered": false,
            "type": "DataPath"
        },
        {
            "bandwidth": 399000000000.0,
            "category": 0,
            "components": [
                94005482937600,
                94005482937200
            ],
            "dataPathCategory": 3,
            "id": 0,
            "latency": 6e-10,
            "ordered": false,
            "type": "DataPath"
        },
        {
            "bandwidth": 199000000000.0,
            "category": 0,
            "components": [
                94005482937440,
                94005482936752
            ],
            "dataPathCategory": 3,
            "id": 0,
            "latency": 2.5e-09,
            "ordered": false,
            "type": "DataPath"
        },
        {
            "bandwidth": 401000000000.0,
            "category": 0,
            "components": [
                94005482937440,
                94005482936960
            ],
            "dataPathCategory": 3,
            "id": 0,
            "latency": 4e-10,
            "ordered": false,
            "type": "DataPath"
        }
    ]
}
```

## Attributes

Since a component's attribute can be of arbitrary type, _sys-sage_ maintains a type registry system to automatically dump and load arbitrary attributes.
In order for _sys-sage_ to know how to process an attribute, its type needs to be registered and serialization/deserialization callbacks need to be provided.

### Serialization & Deserialization Callbacks

_sys-sage_ relies on the callbacks used natively by the `nlohmann-json` library, i.e. a type must provide a `to_json` function for serialization and a `from_json` function for deserialization.
The documentation for arbitrary type conversion can be found [here](https://json.nlohmann.me/features/arbitrary_types/).
The following illustrates an example:

```cpp
struct Foo {
    int x;
    int y;
};

void to_json(nlohmann::json &obj, const Foo &foo)
{
    obj["x"] = foo.x;
    obj["y"] = foo.y;
}

void from_json(const nlohmann::json &obj, Foo &foo)
{
    obj.at("x").get_to(foo.x);
    obj.at("y").get_to(foo.y);
}
```

### Type Registration

Types can be registered in two ways: automatic or manual.

In case of the former, types are registered **on demand** without the user's involvement.
Meaning that if a component contains an attribute of type `Attribute<T>`, then `T` will be registered automatically.
Note that if an attribute is never used, its type will never be registered.
Let's assume that the file `topo.json` contains the following content:

```cpp
{
    "componentTree": {
        "address": 94705651546592,
        "attributes": {
            "foo": {
                "_sys_sage_type": "Foo",
                "_sys_sage_value": {
                    "x": 0,
                    "y": 1
                }
            }
        },
        "id": 0,
        "type": "GenericComponent"
    }
}
```

We can load the component and print the number of attributes stored by the component with 

```
int main()
{
    ss::Component *comp = ss::LoadJson("topo.json");
    std::cout << comp->GetAttributesSize() << '\n';

    return 0;
}
```

The output will be `0`, because the attribute `"foo"` is never used and hence its type is not registered automatically.
