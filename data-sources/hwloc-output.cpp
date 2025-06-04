#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <hwloc.h>


/*! \file */
/// @private
std::string _get_hwloc_topology_xml_string();

/**
Binary (entrypoint) for generating hwloc topology XML output (to current directory). Functionality is almost identical to "lstopo --of XML" (with an option to store to a file).
\n usage: ./hwloc-output [output_filename]
@param filename of the output file. If no output_filename is selected, print to stdout (also use "-" as output_filename to print to stdout)
*/
int main(int argc, const char* argv[])
{
    bool to_stdout = false;
    std::string filename;
    if (argc < 2 || std::string(argv[1]) == "-" ) {
        to_stdout = true;
    }
    else {
       filename = argv[1];
    }

    std::string xml_output = _get_hwloc_topology_xml_string();

    if (!xml_output.empty()) {
        if(to_stdout)
        {
            std::cout << xml_output;
        }
        else
        {
            std::ofstream outfile;
            outfile.open(filename);
            outfile << xml_output;
            outfile.close();
            //cout << "Hwloc XML output exported to " << filename << endl;
        }
    }
    else {
        std::cerr << "Failed to generate hwloc topology XML output" << std::endl;
        return 1;
    }

    return 0;
}

std::string _get_hwloc_topology_xml_string() {
    int err;
    unsigned long flags = 0; // don't show anything special
    hwloc_topology_t topology;

    err = hwloc_topology_init(&topology);
    if (err) {
        std::cerr << "hwloc: Failed to initialize" << std::endl;
        return "";
    }
    err = hwloc_topology_set_flags(topology, flags);
    if (err) {
        std::cerr << "hwloc: Failed to set flags" << std::endl;
        hwloc_topology_destroy(topology);
        return "";
    }
    err = hwloc_topology_load(topology);
    if (err) {
        std::cerr << "hwloc: Failed to load topology" << std::endl;
        hwloc_topology_destroy(topology);
        return "";
    }
    char * xmlbuffer;
    int buflen;
    err = hwloc_topology_export_xmlbuffer(topology, &xmlbuffer, &buflen, flags);
    if (err) {
        std::cerr << "hwloc: Failed to export to a temporary buffer" << std::endl;
        hwloc_free_xmlbuffer(topology, xmlbuffer);
        hwloc_topology_destroy(topology);
        return "";
    }
    std::string xml_output(xmlbuffer, buflen);

    hwloc_free_xmlbuffer(topology, xmlbuffer);
    hwloc_topology_destroy(topology);

    return xml_output;
}
