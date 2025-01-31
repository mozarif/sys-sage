
#include "sys-sage.hpp"
#include <map>
#include <string>

int main(int argc, char *argv[]) {
  string xmlPath;
  if (argc < 2) {
    std::string path_prefix(argv[0]);
    std::size_t found = path_prefix.find_last_of("/\\");
    path_prefix = path_prefix.substr(0, found) + "/";
    xmlPath = path_prefix + "example_data/skylake_hwloc.xml";
  } else if (argc == 3) {
    xmlPath = argv[1];
  }

  // create root Topology and one node
  Topology *topo = new Topology();
  Node *n = new Node(topo, 1);

  cout << "-- Parsing Hwloc output from file " << xmlPath << endl;
  parseHwlocOutput(n, xmlPath);

  vector<Component *> c_orig = topo->GetComponentsInSubtree();

  exportToXml(topo, "output.xml");

  Topology *topo2 = (Topology *)importFromXml("output.xml");

  for (auto c : c_orig) {
    int type = c->GetComponentType();
    int id = c->GetId();
    Component *c2 = topo2->GetSubcomponentById(id, type);
    if (c2 == NULL) {
      cout << "Component with id " << id << " and type " << type
           << " not found in imported topology" << endl;

      topo->Delete(true);
      topo2->Delete(true);

      // Delete output.xml
      std::remove("output.xml");
      return 0;
    }
  }
  cout << "Original topology:" << endl;
  topo->PrintSubtree(2);
  cout << "\nImported topology:" << endl;
  topo2->PrintSubtree(2);
  cout << "\nAll components found!" << endl;

  topo->Delete(true);
  topo2->Delete(true);

  // Delete output.xml
  std::remove("output.xml");
  return 0;
}
