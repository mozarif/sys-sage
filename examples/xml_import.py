import sys_sage as syge
import os
import sys

def main():
    xmlPath = ""
    if len(sys.argv) < 2:
        path_prefix = os.path.dirname(os.path.abspath(sys.argv[0]))
        xmlPath = os.path.join(path_prefix, "example_data", "skylake_hwloc.xml")
    elif len(sys.argv) == 3:
        xmlPath = sys.argv[1]

    # create root Topology and one node
    topo = syge.Topology()
    n = syge.Node(topo, 1)

    print("-- Parsing Hwloc output from file", xmlPath)
    syge.parseHwlocOutput(n, xmlPath)

    c_orig = topo.GetComponentsInSubtree()

    syge.exportToXml(topo, "output.xml")

    topo2 = syge.importFromXml("output.xml")

    for c in c_orig:
        type = c.GetComponentType()
        id = c.GetId()
        c2 = topo2.GetSubcomponentById(id, type)
        if c2 is None:
            print(f"Component with id {id} and type {type} not found in imported topology")

            topo.Delete(True)
            topo2.Delete(True)

            # Delete output.xml
            os.remove("output.xml")
            return

    print("Original topology:")
    topo.PrintSubtree(2)
    print("\nImported topology:")
    topo2.PrintSubtree(2)
    print("\nAll components found!")

    topo.Delete(True)
    topo2.Delete(True)

    # Delete output.xml
    os.remove("output.xml")

if __name__ == "__main__":
    main()

