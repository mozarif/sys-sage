#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <hwloc.h>


using namespace std;


/*! \file */
/// @private
string get_hwloc_topology_xml_string();

/**
Binary (entrypoint) for generating hwloc topology XML output (to current directory). Functionality is almost identical to "lstopo --of XML" (with an option to store to a file).
\n usage: ./hwloc-output [output_filename]
@param filename of the output file. If no output_filename is selected, print to stdout (also use "-" as output_filename to print to stdout)
*/
int main(int argc, char* argv[])
{
    bool to_stdout = false;
    string filename;
    if (argc < 2 || strcmp(argv[1], "-") == 0 ) {
        to_stdout = true;
    }
    else {
       filename = argv[1];
    }

    string xml_output = get_hwloc_topology_xml_string();

    if (!xml_output.empty()) {
        if(to_stdout)
        {
            std::cout << xml_output;
        }
        else
        {
            ofstream outfile;
            outfile.open(filename);
            outfile << xml_output;
            outfile.close();
            //cout << "Hwloc XML output exported to " << filename << endl;
        }
    }
    else {
        cerr << "Failed to generate hwloc topology XML output" << endl;
        return 1;
    }

    return 0;
}

string get_hwloc_topology_xml_string() {
    int err;
    unsigned long flags = 0; // don't show anything special
    hwloc_topology_t topology;

    err = hwloc_topology_init(&topology);
    if (err) {
        cerr << "hwloc: Failed to initialize" << endl;
        return "";
    }
    err = hwloc_topology_set_flags(topology, flags);
    if (err) {
        cerr << "hwloc: Failed to set flags" << endl;
        hwloc_topology_destroy(topology);
        return "";
    }
    err = hwloc_topology_load(topology);
    if (err) {
        cerr << "hwloc: Failed to load topology" << endl;
        hwloc_topology_destroy(topology);
        return "";
    }
    char * xmlbuffer;
    int buflen;
    err = hwloc_topology_export_xmlbuffer(topology, &xmlbuffer, &buflen, flags);
    if (err) {
        cerr << "hwloc: Failed to export to a temporary buffer" << endl;
        hwloc_free_xmlbuffer(topology, xmlbuffer);
        hwloc_topology_destroy(topology);
        return "";
    }
    std::string xml_output(xmlbuffer, buflen);

    hwloc_free_xmlbuffer(topology, xmlbuffer);
    hwloc_topology_destroy(topology);

    return xml_output;
}
