#include <boost/ut.hpp>

#include <cstdio>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlschemas.h>
#include <libxml/xpath.h>

#include "sys-sage.hpp"

#include <memory>
#include <set>
#include <string>
#include <vector>

// Test that we can import XML files
// TODO: Add more tests

using namespace boost::ut;

static suite<"import"> _ = [] {
  
  "sample"_test = [] {
    Topology *topo = (Topology *)importFromXml(SYS_SAGE_TEST_RESOURCE_DIR
                                               "/sys-sage_sample_output.xml");
    expect(topo != nullptr);
    expect(topo->GetId() == 0);
    Node* node = (Node*)topo->GetChildren()->at(0);
    expect(node != nullptr);
    expect(node->GetId()==1);
    //size of node children must be 2
    expect(node->GetChildren()->size()==2);
    Chip* chip = (Chip*)node->GetChildById(0);
    expect(chip != nullptr);
    expect(chip->GetModel() == "Intel(R) Xeon(R) Silver 4116 CPU @ 2.10GHz");
    expect (nullptr != (Thread *)chip->GetSubcomponentById(0, SYS_SAGE_COMPONENT_THREAD));

    Chip* chip2 = (Chip*)node->GetChildById(1);
    expect(chip2 != nullptr);
    Numa* numa = (Numa*)chip2->GetSubcomponentById(2, SYS_SAGE_COMPONENT_NUMA);
    expect(numa != nullptr);
    vector<DataPath*>* dp_out = numa->GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING);
    expect(dp_out->size() == 4);
    //find a all datapaths where target is other Numa
    std::set<int> found;
    for(DataPath* dp : *dp_out){
        Component* target = dp->GetTarget();
        if(target->GetComponentType() == SYS_SAGE_COMPONENT_NUMA){
            found.insert(target->GetId());
        }
    }
    expect(found == std::set<int>{0,1,2,3});


  };
};
// Compare two XML files
// TODO: Add more tests

// Compare Attributes