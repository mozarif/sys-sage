#ifndef QUBIT_HPP
#define QUBIT_HPP

#include "Component.hpp"

namespace sys_sage {

    
    class Qubit : public Component {
    public:
        /**
        Qubit constructor (no automatic insertion in the Component Tree). Sets:
        @param _id = id, default 0
        @param _name = name, default "Qubit"
        @param componentType=>SYS_SAGE_COMPONENT_QUBIT
        */
        Qubit(int _id = 0, std::string _name = "Qubit");
        /**
        Qubit constructor with insertion into the Component Tree as the parent 's child (as long as parent is an existing Component). Sets:
        @param parent = the parent 
        @param _id = id, default 0
        @param _name = name, default "Qubit0"
        @param componentType=>SYS_SAGE_COMPONENT_QUBIT
        */
        Qubit(Component * parent, int _id = 0, std::string _name = "Qubit");

        // struct NeighbouringQubit
        // {   
        //     NeighbouringQubit (int qubit_index, double fidelity)
        //     : _qubit_index (qubit_index), _fidelity (fidelity){}
            
        //     int _qubit_index;
        //     double _fidelity; // Two qubit fidelity
        // };

        // /**
        // * @brief Sets the coupling mapping for the qubit.
        // * 
        // * @param coupling_mapping A vector of integers representing the coupling mapping.
        // * @param size_coupling_mapping The size of the coupling mapping.
        // */
        // void SetCouplingMapping( const std::vector <NeighbouringQubit> &coupling_mapping, const int &size_coupling_mapping);

        /**
        * @brief Sets the properties of the qubit.
        * 
        * @param t1 The T1 relaxation time.
        * @param t2 The T2 dephasing time.
        * @param readout_fidelty The readout fidelity.
        * @param q1_fidelity 1 qubit fidelity
        * @param readout_length The readout length.
        */
        void SetProperties(double t1, double t2, double readout_fidelty, double q1_fidelity = 0, double readout_length = 0);

        // /**
        // * @brief Gets the coupling mapping of the qubit.
        // * 
        // * @return A constant reference to a vector of integers representing the coupling mapping.
        // */
        // const std::vector <NeighbouringQubit> &GetCouplingMapping() const;

        /**
        * @brief Gets the T1 relaxation time of the qubit.
        * 
        * @return The T1 relaxation time.
        */
        double GetT1() const;
        /**
        * @brief Gets the T2 dephasing time of the qubit.
        * 
        * @return The T2 dephasing time.
        */
        double GetT2() const;

        /**
        * @brief Gets the readout fidelity of the qubit.
        * 
        * @return The readout fidelity 
        */
        double GetReadoutFidelity() const;

        /**
        * @brief Gets the 1Q fidelity of the qubit.
        * 
        * @return 1Q fidelity 
        */
        double Get1QFidelity() const;

        /**
        * @brief Gets the readout length of the qubit.
        * 
        * @return The readout length.
        */
        double GetReadoutLength() const;

        /**
        * @brief Gets the frequency of the qubit.
        * 
        * @return The frequency of the qubit.
        */
        double GetFrequency() const;

        /**
        * @brief Refreshes the properties of the qubit.
        */
        void RefreshProperties();

        /**
        @private 
        !!Should normally not be used!! Helper function of XML dump generation.
        @see exportToXml(Component* root, string path = "", std::function<int(string,void*,string*)> custom_search_attrib_key_fcn = NULL);
        */
        xmlNodePtr _CreateXmlSubtree() override;

        ~Qubit() override = default;

    private:
        // std::vector <NeighbouringQubit> _coupling_mapping;
        // int _size_coupling_mapping;
        double q1_fidelity;
        double t1;
        double t2;
        double readout_fidelity;
        double readout_length;
        double fequency;
        // double qubit_weight;
        std::string calibration_time;
    };

}
#endif //QUBIT_HPP