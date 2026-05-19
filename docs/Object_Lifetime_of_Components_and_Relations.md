# Object Lifetime of Components and Relations

This document contains general information about the object creation and
specifically the destruction of components and relations within the _sys-sage_
library. This should be taken into account for correct memory management and to
avoid undefined behavior.

## Ownership

Components do not take ownership of other components in the Component Tree.
This holds especially for parent and child components. Therefore, the
destructor of the `Component` class will only **unlink this component from the
its parent and its children** (if they are present) and additionally clean up any
claimed resources. These resources include the relation objects of type
`Relation`. Hence, deleting a component will trigger the **deletion of all
relations the component was part of**. If this is not desired, then the component
should first be unlinked expliciteley from the relation through the
`Relation::RemoveComponent` method. Moreover, if the entire subtree spanned by
a component should be deleted, then the `Component::DeleteSubtree` method
should be used. Note that this recursively calls `operator delete` on the
children and on the component itself. It should therefore **only be called if all
components in the subtree are heap-allocated**.

Contrary to this, the destructor of the `Relation` class **does not delete any
components** and instead **unlinks itself** from them in addition to cleaning up
claimed resources. It has no influence on other relations.

## Construction and Destruction Order

_sys-sage_ does not enforce a strict requirement on whether objects are created
on the stack or on the heap. However, the construction order of stack variables
can matter and is handled similar to the QObject class of the Qt library.

Since destructors of components only unlink themselves from the Component Tree,
their order of construction and destruction does not matter at all. The
following examples work fine:

```cpp
#include <sys-sage.hpp>

int main()
{
    {
        // first create the parent on the stack and then the child

        sys_sage::Component parent;
        sys_sage::Component child (&parent);
    }

    {
        // first create the child on the stack and then the parent

        sys_sage::Component child;
        sys_sage::Component parent;
        parent.InsertChild(&child);
    }

    {
        // first create the parent on the heap and then the child

        auto parent = new sys_sage::Component;
        auto child = new sys_sage::Component(parent);

        // `delete` can be called in any order
        delete parent;
        delete child;
    }

    {
        // aggregate deletion of the Component Tree

        auto child = new sys_sage::Component;
        auto parent = new sys_sage::Component;
        parent->InsertChild(child);

        parent->DeleteSubtree();
        // `parent` and `child` are deleted and should not be used further
    }

    {
        // first create the parent on the heap and the child on the stack

        auto parent = new sys_sage::Component;
        sys_sage::Component child (parent);

        delete parent; // does not falsly clean up the child
        // child is cleaned up at scope exit
    }

    return 0;
}
```

Relations are generally cleaned up internally by deleting the corresponding
components in the Component Tree as mentioned above. Nevertheless, a bit more
care has to be taken here. Consider the following:

```cpp
#include <sys-sage.hpp>

int main()
{
    // initialize an empty relation
    sys_sage::Relation rel ({});

    sys_sage::Component comp1;
    sys_sage::Component comp2;

    rel.AddComponent(&comp1);
    rel.AddComponent(&comp2);

    return 0;
}
```

Since the C++ language standard specifies that local variables are cleaned up
in reverse order of their construction, the destructor of `comp2` will falsly
try to delete the stack-allocated relation by calling `operator delete` on
`rel`. This results in undefined behavior. To mitigate this, the construction
order should be changed such that all stack-allocated components are created
before the creation of the stack-allocated relation. Alternatively, one can
heap-allocate the relation instead:

```cpp
#include <sys-sage.hpp>

int main()
{

    {
        sys_sage::Component comp1;
        sys_sage::Component comp2;

        sys_sage::Relation rel({ &comp1, &comp2 });
    }

    {
        auto rel = new sys_sage::Relation ({});

        sys_sage::Component comp1;
        sys_sage::Component comp2;

        rel->AddComponent(&comp1);
        rel->AddComponent(&comp2);

        // we don't have to explicitely delete the relation
    }

    return 0;
}
```
