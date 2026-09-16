# Tutorial 05: Adding 3rd Party Extensions and Processing Dynamic System Properties

This tutorial will briefly illustrate how you can further integrate 3rd party libraries into _sys-sage_ to manage dynamic system properties that are relevant to your use case.

## Mapping to the Internal Representation

At first, it is important to decide how the data provided by the 3rd party library should be stored/represented within _sys-sage_'s Internal Representation.
This can range from simply attaching a new attribute to a Component or defining new data structures and semantics.

For this example, let us consider a library that gives us information about the power zones and associated metrics of a node (e.g. Linux's DTPM framework).
Let's say that the cores are logically aggregated into distinct power zones, each carrying information about the aggregate energy consumption and corresponding min/max limits.
We want to capture this information in _sys-sage_ by wrapping the libraries API calls around some additional logic that integrates the provided data into _sys-sage_.

Since a power zone usually spans multiple components/cores, we would like to use a Relation to model this logical relationship.
However, _sys-sage_ does not provide a RelationType that is tailored out-of-the-box for this specific use case, as we need to store power-related metrics.
Luckily, we can take a "plain" Relation and customize it by defining a new RelationCategory and by making use of a Relation's attribute to store arbitrary data.
