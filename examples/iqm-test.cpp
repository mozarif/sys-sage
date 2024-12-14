#include <iomanip>
#include "sys-sage.hpp"

int main()
{
    std::cout << std::setprecision(15);
    std::ifstream filePath("/home/diogenes/sys-sage/database/calibration_data_Q-Exa_20241013.json");
    IQMParser iqm(filePath);

    auto qc = iqm.createQcTopo();

    cout << "---------------- Printing the configuration of IQM Backend----------------" << endl;
    qc.PrintSubtree();

    // json data = json::parse(_filePath);

    // std::string name = data["backend_name"];

    // std::cout << "Name of the backend: " << name <<"\n";

    // std::vector<double> T1;
    // for (const auto& element : data["T1"]) 
    // {
    //     T1.push_back(std::stod(element.get<std::string>()));
    // }
    // // auto T1 = data["T1"].get<std::vector<double>>();
    // for (const auto& value : T1) 
    // {
    //     std::cout << value << "\n";
    // }

    return 0;
}
