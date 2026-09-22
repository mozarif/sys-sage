# Tutorial 06: Using _sys-sage_'s Type Registry System

The [documentation](../../docs/JSON_Serialization_and_Deserialization.md) about the type registry to support serialization and deserialization of arbitrary attributes mentions multiple case scenarios that involve their own set of API features and functionality.
This tutorial will only consider one scenario for brevity, that is the implicit registration of user-defined types that are type-templated, and showcase how to actually use some of the API features that were only mentioned briefly in the documentation.
Rather than repeating the explanations already provided in the documentation, we will focus on incrementally writing the code and simply show how you could use the type registry system and the meta data surrounding it.

## Enabling Serialization and Deserialization of a User-defined Type

Let us consider the following class defined in the header `Bar.hpp`

```cpp
// Bar.hpp

namespace Foo
{
    template <typename T1, typename T2>
    class Bar
    {
    public:
        Bar(T1 _x, T2 _y) : x (_x), y (_y) {}

        T1 x;
        T2 y;
    };
}
```

The first step would be implement some logic that expresses how to serialize the type `Foo::Bar<T1, T2>` to JSON and how to deserialize it back from JSON.
For that we make use of the [nlohmann-json](https://json.nlohmann.me/) library.
Since the above type is non-default-constructible (see [here](https://json.nlohmann.me/api/adl_serializer/from_json/#examples)), we need to update `Bar.hpp` with the following:

```cpp
namespace nlohmann
{
    template <typename T1, typename T2>
    struct adl_serializer<Foo::Bar<T1, T2>>
    {
        static void to_json(nlohmann::json &obj, const Foo::Bar<T1, T2> &bar)
        {
            obj["x"] = bar.x;
            obj["y"] = bar.y;
        }

        static Foo::Bar<T1, T2> from_json(const nlohmann::json &obj)
        {
            return Foo::Bar<T1, T2>(obj["x"].get<T1>(), obj["y"].get<T2>());
        }
    };
}
```

The `to_json` function in our specialized `nlohmann::adl_serializer<Foo::Bar<T1, T2>>` struct will handle the serialization while the `from_json` function manages the deserialization.
These functions will internally be called whenever we do something like this

```cpp
Foo::Bar<int, float> bar (1, 0.1);

nlohmann::json barJSON = bar; // uses to_json
Foo::Bar<int, float> newBar = barJSON.get<Foo::Bar<int, float>>(); // uses from_json
```

## Type Registration

We want to implicitly register `Foo::Bar<T1, T2>` for any instance of `T1` and `T2`.
Therefore, we make use of the `SYS_SAGE_REGISTER_TEMPLATED_TYPE_TRAIT` macro inside of `Bar.hpp`:

```cpp
SYS_SAGE_REGISTER_TEMPLATED_TYPE_TRAIT(Foo::Bar)
```

With that our `Bar.hpp` header file will look like this:

```cpp
// Bar.hpp

#include <nlohmann/json.hpp>
#include <sys-sage.hpp>

// type definition
namespace Foo
{
    template <typename T1, typename T2>
    class Bar
    {
    public:
        Bar(T1 _x, T2 _y) : x (_x), y (_y) {}

        T1 x;
        T2 y;
    };
}

// serialization & deserialization callbacks
namespace nlohmann
{
    template <typename T1, typename T2>
    struct adl_serializer<Foo::Bar<T1, T2>>
    {
        static void to_json(nlohmann::json &obj, const Foo::Bar<T1, T2> &bar)
        {
            obj["x"] = bar.x;
            obj["y"] = bar.y;
        }

        static Foo::Bar<T1, T2> from_json(const nlohmann::json &obj)
        {
            return Foo::Bar<T1, T2>(obj["x"].get<T1>(), obj["y"].get<T2>());
        }
    };
}

// registration
SYS_SAGE_REGISTER_TEMPLATED_TYPE_TRAIT(Foo::Bar)
```

## Using the Type as an Attribute

Inside of `main.cpp`, we can for instance create a Component that contains an attribute of type `Foo::Bar<T1, T2>`:

```cpp
// main.cpp

#include "Bar.hpp"
#include <iostream>

int main()
{
    sys_sage::Component *comp = new sys_sage::Component;

    comp->SetAttribute("bar", Foo::Bar<std::string, std::tuple<long, double>>(
        "hello world",
        { 100, 3.1415 }
    ));

    // serialize the component into a JSON object and print it
    nlohmann::json obj = comp;
    std::cout << obj.dump(4) << '\n';

    std::cout << "\n-----------------------\n\n";

    // deserialize the component into a new Component object and print it's JSON representation
    sys_sage::Component *newComp = obj.get<sys_sage::Component *>();
    std::cout << nlohmann::json(newComp).dump(4) << '\n';

    delete newComp;
    delete comp;

    return 0;
}
```

The output would be

```bash
{
    "address": 94809525488240,
    "attributes": {
        "bar": {
            "_sys_sage_type": "Foo::Bar<std::string, std::tuple<long, double>>",
            "_sys_sage_value": {
                "x": "hello world",
                "y": [
                    100,
                    3.1415
                ]
            }
        }
    },
    "id": 0,
    "type": "GenericComponent"
}

-----------------------

{
    "address": 94809525492832,
    "attributes": {
        "bar": {
            "_sys_sage_type": "Foo::Bar<std::string, std::tuple<long, double>>",
            "_sys_sage_value": {
                "x": "hello world",
                "y": [
                    100,
                    3.1415
                ]
            }
        }
    },
    "id": 0,
    "type": "GenericComponent"
}
```

Notice that the "attribute" field of both JSON objects are identical and that they denote the type and the corresponding value of the attribute.

## Meta Programming

As mentioned in the documentation, `SYS_SAGE_REGISTER_TEMPLATED_TYPE_TRAIT` generates some meta information that is used for the registration.
We can access this information (see [here](../../docs/JSON_Serialization_and_Deserialization.md#meta-information)):

```cpp
// main.cpp

#include "Bar.hpp"
#include <iostream>

int main()
{
    using FooBarStrTup = Foo::Bar<std::string, std::tuple<long, double>>;

    std::cout << (sys_sage::TypeTrait<FooBarStrTup>::serializable ? "Yes" : "No") << '\n'; // prints "Yes"

    std::cout << (sys_sage::TypeTrait<FooBarStrTup>::deserializable ? "Yes" : "No") << '\n'; // prints "Yes"

    std::cout << sys_sage::TypeTrait<FooBarStrTup>::id << '\n'; // prints "Foo::Bar<std::string, std::tuple<long, double>>"

    std::cout << (sys_sage::TypeTrait<FooBarStrTup>::registered<> ? "Yes" : "No") << '\n'; // prints "Yes"

    std::cout << (sys_sage::IsBlacklistedFromSerialization<FooBarStrTup>::value ? "Yes" : "No") << '\n'; // prints "No"

    std::cout << (sys_sage::IsBlacklistedFromDeserialization<FooBarStrTup>::value ? "Yes" : "No") << '\n'; // prints "No"

    return 0;
}
```
