#ifndef CHIP_HPP
#define CHIP_HPP

#include "Component.hpp"

namespace sys_sage {

    /**
    Class Chip - represents a building block of a node. It may be a CPU socket, a GPU, a NIC or any other chip.
    \n This class is a child of Component class, therefore inherits its attributes and methods.
    */
    class Chip : public Component {
    public:
        /**
        Chip constructor (no automatic insertion in the Component Tree). Sets:
        @param _id = id, default 0
        @param _name = name, default "Chip"
        @param _type = chip type, default sys_sage::ChipType::None. Defines which chip we are describing. The options are: sys_sage::ChipType::None (default/generic), sys_sage::ChipType::Cpu, sys_sage::ChipType::CpuSocket, sys_sage::ChipType::Gpu.
        @param componentType=>SYS_SAGE_COMPONENT_CHIP
        @param _vendor = name of the vendor, default ""
        @param _model = model name, default ""
        */
        Chip(int _id = 0, std::string _name = "Chip", ChipType::type _type = ChipType::None, std::string _vendor = "", std::string _model = "");
        /**
        Chip constructor with insertion into the Component Tree as the parent 's child (as long as parent is an existing Component). Sets:
        @param parent = the parent 
        @param _id = id, default 0
        @param _name = name, default "Chip"
        @param _type = chip type, default sys_sage::ChipType::None. Defines which chip we are describing. The options are: sys_sage::ChipType::None (default/generic), sys_sage::ChipType::Cpu, sys_sage::ChipType::CpuSocket, sys_sage::ChipType::Gpu.
        @param componentType=>SYS_SAGE_COMPONENT_CHIP
        @param _vendor = name of the vendor, default ""
        @param _model = model name, default ""
        */
        Chip(Component * parent, int _id = 0, std::string _name = "Chip", ChipType::type _type = ChipType::None, std::string _vendor = "", std::string _model = "");
        /**
        * @private
        * Use Delete() or DeleteSubtree() for deleting and deallocating the components. 
        */
        ~Chip() override = default;
        /**
        Sets the vendor of the chip.
        @param _vendor - The name of the vendor to set.
        */
        void SetVendor(std::string _vendor);
        /**
        Gets the vendor of the chip.
        @return The name of the vendor.
        @see vendor
        */
        const std::string& GetVendor() const;
        
        /**
        Sets the model of the chip.
        @param _model - The model name to set.
        */
        void SetModel(std::string _model);
        
        /**
        Gets the model of the chip.
        @return The model name.
        @see model
        */
        const std::string& GetModel() const;
        
        /**
        Sets the type of the chip.
        @param chipType - The chip type to set.
        */
        void SetChipType(int chipType);
        
        /**
        Gets the type of the chip.
        @return The chip type.
        @see type
        */
        int GetChipType() const;
        
        /**
        @private
        !!Should normally not be used!! Helper function of XML dump generation.
        @see exportToXml(Component* root, string path = "", std::function<int(string,void*,string*)> custom_search_attrib_key_fcn = NULL);
        */
        xmlNodePtr _CreateXmlSubtree() override;
    private:
        std::string vendor; /**< Vendor of the chip */
        std::string model; /**< Model of the chip */
        ChipType::type type; /**< Type of the chip, e.g., CPU, GPU */
    #ifdef NVIDIA_MIG
    public:
        /**
        Updates the MIG settings for the chip.
        @param uuid - The UUID of the chip, default is an empty string.
        @return Status of the update operation.
        */
        int UpdateMIGSettings(std::string uuid = "");

        /**
        Gets the number of SMs for the MIG.
        @param uuid - The UUID of the chip, default is an empty string.
        @return The number of SMs.
        */
        int GetMIGNumSMs(std::string uuid = "") const;

        /**
        Gets the number of cores for the MIG.
        @param uuid - The UUID of the chip, default is an empty string.
        @return The number of cores.
        */
        int GetMIGNumCores(std::string uuid = "") const;
    #endif
    };
}

#endif //CHIP_HPP