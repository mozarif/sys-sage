#include <iostream>
#include <filesystem>

#include "sys-sage.hpp"


int main()
{
    auto vec = FOMAC_available_devices();
    int count = 0;
    std::cout << "Total number of devices: " << vec.size() << "\n";
    for (auto dev : vec)
    {
        std::cout << "Printing for device id: " << ++count << "\n";
        FOMAC_print_coupling_mappings(dev);
    }

    
    return 0;
}
