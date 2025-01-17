
#include <string>
#include "sys-sage.hpp"

int main(int argc, char *argv[]) {
    
    //first argument is the xml file to import
    string xmlPath = argv[1];

    //import the topology from the xml file
    Topology* topo = (Topology*) importFromXml(xmlPath);

    //print the subtree of the topology object
    topo->PrintSubtree(2);

    //print all datapaths in the topology object
    topo->PrintAllDataPathsInSubtree();

    //export the topology to an xml file named "output.xml"
    exportToXml(topo, "output.xml");
}
    
